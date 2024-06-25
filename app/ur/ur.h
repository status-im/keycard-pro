#ifndef _UR_H_
#define _UR_H_

#include <stdint.h>
#include "error.h"

#define UR_MAX_PART_COUNT 32
#define UR_PART_DESC_COUNT (UR_MAX_PART_COUNT + 16)

typedef enum {
  ETH_SIGNATURE = 0,
  CRYPTO_OUTPUT = 1,
  CRYPTO_KEYPATH = 2,
  ETH_SIGN_REQUEST = 3,
  BYTES = 6,
  FS_DATA = 8,
  DEV_AUTH = 9,
  CRYPTO_MULTI_ACCOUNTS = 10,
  FW_UPDATE = 11,
  CRYPTO_PSBT = 14,
  CRYPTO_HDKEY = 15,
  UR_ANY_TX = 255
} ur_type_t;

typedef struct {
  ur_type_t type;
  uint32_t crc;
  uint32_t part_desc[UR_PART_DESC_COUNT];
  uint32_t part_mask;
  double sampler_probs[UR_MAX_PART_COUNT];
  int sampler_aliases[UR_MAX_PART_COUNT];
  size_t data_max_len;
  size_t data_len;
  uint8_t* data;
  uint8_t percent_done;
} ur_t;

app_err_t ur_process_part(ur_t* ur, const uint8_t* in, size_t in_len);
app_err_t ur_encode(ur_t* ur, char* out, size_t max_len);

#endif
