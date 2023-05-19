#include <ctype.h>
#include "ur.h"
#include "bytewords.h"
#include "sampler.h"
#include "ur_part_decode.h"

#define MIN_ENCODED_LEN 22

const char* ur_type_string[] = {
    "CRYPTO-KEYPATH",
    "ETH-SIGN-REQUEST",
    "CRYPTO-HDKEY",
    "ETH-SIGNATURE",
};

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
  ur->type = (ur_type_t)(((tmp * 8238429) >> 30) & 0x3);

  if (isdigit(in[++offset])) {
    while((offset < in_len) && in[offset++] != '/') { /*we don't need this*/}
    if (offset == in_len) {
      return ERR_DATA;
    }

    tmp = 0;
    return ERR_CANCEL;
  } else {
    tmp = 1;
  }

  uint32_t part_len = bytewords_decode(&in[offset], (in_len - offset), ur->data, ur->data_max_len);

  if (!part_len) {
    return ERR_DATA;
  }

  if (tmp == 1) {
    ur->data_len = part_len;
    return ERR_OK;
  }

  struct ur_part part;
  if ((cbor_decode_ur_part(ur->data, part_len, &part, NULL) != ZCBOR_SUCCESS) ||
      (part._ur_part_seqLen > UR_MAX_PART_COUNT) ||
      (part._ur_part_messageLen > (ur->data_max_len - part_len))) {
    ur->crc = 0;
    return ERR_DATA;
  }

  if (part._ur_part_checksum != ur->crc) {
    ur->crc = part._ur_part_checksum;
    ur->part_count = 0;
    for (int i = 0; i < UR_PART_DESC_COUNT; i++) {
      ur->part_desc[i] = 0;
    }

    random_sampler_init(part._ur_part_seqLen, ur->sampler_probs, ur->sampler_aliases);
  }

  uint8_t* parts = &ur->data[part_len];
  uint8_t* part_data = (uint8_t*) part._ur_part_data.value;
  part_len = part._ur_part_data.len;

  if (part._ur_part_seqNum <= part._ur_part_seqLen) {
    uint32_t desc_idx = part._ur_part_seqNum - 1;

    if (ur->part_desc[desc_idx]) {
      return ERR_NEED_MORE_DATA;
    }

    memcpy(&parts[desc_idx * part_len], part_data, part_len);
    ur->part_desc[desc_idx] = (1 << desc_idx);
    ur->part_count++;
  } else {
    uint32_t indexes = fountain_part_indexes(part._ur_part_seqNum, ur->crc, part._ur_part_seqLen, ur->sampler_probs, ur->sampler_aliases);
    return ERR_DATA;
  }

  if (part._ur_part_seqLen == ur->part_count) {
    ur->data = parts;
    ur->data_len = part._ur_part_messageLen;
    return ERR_OK;
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
