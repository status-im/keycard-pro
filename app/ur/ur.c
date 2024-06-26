#include <ctype.h>
#include "ur.h"
#include "bytewords.h"
#include "sampler.h"
#include "ur_part_decode.h"

#define MIN_ENCODED_LEN 22
#define MAX_CBOR_HEADER_LEN 32

#define UR_TYPE(t) (ur_type_t)(((t * 2532410) >> 28) & 0xf)

const char *const ur_type_string[] = {
    NULL,
    NULL,
    NULL,
    "BYTES",
    "FS-DATA",
    "DEV-AUTH",
    "FW-UPDATE",
    NULL,
    "CRYPTO-HDKEY",
    "ETH-SIGNATURE",
    "CRYPTO-KEYPATH",
    "ETH-SIGN-REQUEST",
    NULL,
    NULL,
    "CRYPTO-MULTI-ACCOUNTS",
    NULL
};

static app_err_t ur_process_simple(ur_t* ur, uint8_t* parts, uint8_t* part_data, size_t part_len, uint32_t desc_idx, struct ur_part* part) {
  if (ur->part_desc[desc_idx]) {
    return ERR_NEED_MORE_DATA;
  }

  memcpy(&parts[desc_idx * part_len], part_data, part_len);
  ur->part_desc[desc_idx] = (1 << desc_idx);
  ur->part_mask |= (1 << desc_idx);

  uint32_t part_count = __builtin_popcount(ur->part_mask);
  ur->percent_done = (part_count * 100) / part->ur_part_seqLen;

  if (part->ur_part_seqLen == part_count) {
    ur->data = parts;
    ur->data_len = part->ur_part_messageLen;
    return ERR_OK;
  }

  return ERR_NEED_MORE_DATA;
}

app_err_t ur_process_part(ur_t* ur, const uint8_t* in, size_t in_len) {
  if (in_len < 10) {
    return ERR_DATA;
  }

  if (!(toupper(in[0]) == 'U' && toupper(in[1]) == 'R' && in[2] == ':')) {
    return ERR_DATA;
  }

  size_t offset;
  uint32_t tmp = 0;

  for (offset = 3; offset < in_len; offset++) {
    if (in[offset] == '/') {
      break;
    }

    tmp += toupper(in[offset]);
  }

  if (offset == in_len) {
    return ERR_DATA;
  }

  // we assume we are dealing with a supported type and moving the
  // case where we are not to actual payload validation
  ur->type = UR_TYPE(tmp);

  if (isdigit(in[++offset])) {
    while((offset < in_len) && in[offset++] != '/') { /*we don't need this*/}
    if (offset == in_len) {
      return ERR_DATA;
    }

    tmp = 0;
  } else {
    tmp = 1;
  }

  uint32_t part_len = bytewords_decode(&in[offset], (in_len - offset), ur->data, ur->data_max_len);

  if (!part_len) {
    return ERR_DATA;
  }

  if (tmp == 1) {
    ur->crc = 0;
    ur->data_len = part_len;
    return ERR_OK;
  }

  struct ur_part part;
  if ((cbor_decode_ur_part(ur->data, part_len, &part, NULL) != ZCBOR_SUCCESS) ||
      (part.ur_part_seqLen > UR_MAX_PART_COUNT) ||
      (part.ur_part_messageLen > (ur->data_max_len - part_len))) {
    ur->crc = 0;
    return ERR_DATA;
  }

  if (part.ur_part_checksum != ur->crc) {
    ur->crc = part.ur_part_checksum;
    ur->part_mask = 0;
    for (int i = 0; i < UR_PART_DESC_COUNT; i++) {
      ur->part_desc[i] = 0;
    }

    random_sampler_init(part.ur_part_seqLen, ur->sampler_probs, ur->sampler_aliases);
  }

  part_len = part.ur_part_data.len;
  uint8_t* parts = &ur->data[part_len + MAX_CBOR_HEADER_LEN];
  uint8_t* part_data = (uint8_t*) part.ur_part_data.value;

  if (part.ur_part_seqNum <= part.ur_part_seqLen) {
    return ur_process_simple(ur, parts, part_data, part_len, part.ur_part_seqNum - 1, &part);
  }

  uint32_t indexes = fountain_part_indexes(part.ur_part_seqNum, ur->crc, part.ur_part_seqLen, ur->sampler_probs, ur->sampler_aliases);
  if ((indexes & (~ur->part_mask)) == 0) {
    return ERR_NEED_MORE_DATA;
  }

  int desc_idx = 0;
  int store_idx = -1;

  // reduce new part by existing
  while(desc_idx < UR_PART_DESC_COUNT) {
    if (__builtin_popcount(indexes) == 1) {
      int target_idx = __builtin_ctz(indexes);
      if (ur_process_simple(ur, parts, part_data, part_len, target_idx, &part) == ERR_OK) {
        return ERR_OK;
      } else {
        store_idx = target_idx;
        break;
      }
    }

    if (ur->part_desc[desc_idx] == 0) {
      if (desc_idx >= part.ur_part_seqLen) {
        store_idx = desc_idx;
      }
    } else if ((ur->part_desc[desc_idx] & indexes) == (ur->part_desc[desc_idx])) {
      indexes = indexes ^ ur->part_desc[desc_idx];
      if (indexes == 0) {
        return ERR_NEED_MORE_DATA;
      }

      uint8_t* xorpart = &parts[desc_idx * part_len];
      for (int i = 0; i < part_len; i++) {
        part_data[i] ^= xorpart[i];
      }
    }

    desc_idx++;
  }

  // all buffers are full, but we don't give up yet. If one of the buffered parts is more mixed
  // then the current part, we overwrite it since parts easier to reduce are better for us
  if (store_idx == -1) {
    int worst_count = __builtin_popcount(indexes);

    desc_idx = part.ur_part_seqLen;

    while(desc_idx < UR_PART_DESC_COUNT) {
      int count = __builtin_popcount(ur->part_desc[desc_idx]);

      if (count > worst_count) {
        store_idx = desc_idx;
        worst_count = count;
      }

      desc_idx++;
    }

    if (store_idx == -1) {
      return ERR_NEED_MORE_DATA;
    }
  }

  if (store_idx >= part.ur_part_seqLen) {
    memcpy(&parts[store_idx * part_len], part_data, part_len);
    ur->part_desc[store_idx] = indexes;
  }

  //reduce existing parts by new part
  desc_idx = part.ur_part_seqLen;

  while(desc_idx < UR_PART_DESC_COUNT) {
    if ((desc_idx != store_idx) && ((ur->part_desc[desc_idx] & indexes) == indexes)) {
      ur->part_desc[desc_idx] = indexes ^ ur->part_desc[desc_idx];

      if (ur->part_desc[desc_idx] == 0) {
        desc_idx++;
        continue;
      }

      uint8_t* target_part = &parts[desc_idx * part_len];
      for (int i = 0; i < part_len; i++) {
        target_part[i] ^= part_data[i];
      }

      if (__builtin_popcount(ur->part_desc[desc_idx]) == 1) {
        int target_idx = __builtin_ctz(ur->part_desc[desc_idx]);
        if (ur_process_simple(ur, parts, target_part, part_len, target_idx, &part) == ERR_OK) {
          return ERR_OK;
        }

        ur->part_desc[desc_idx] = 0;
      }
    }

    desc_idx++;
  }

  return ERR_NEED_MORE_DATA;
}

app_err_t ur_encode(ur_t* ur, char* out, size_t max_len) {
  if (max_len < MIN_ENCODED_LEN) {
    return ERR_DATA;
  }

  size_t off = 0;
  out[off++] = 'U';
  out[off++] = 'R';
  out[off++] = ':';

  const char* typestr = ur_type_string[ur->type];

  while(*typestr != '\0') {
    out[off++] = *(typestr++);
  }

  out[off++] = '/';

  size_t outlen = bytewords_encode(ur->data, ur->data_len, (uint8_t*)&out[off], (max_len-off-1));
  if (!outlen) {
    return ERR_DATA;
  }

  out[off+outlen] = '\0';
  return ERR_OK;
}
