#ifndef _UR_H_
#define _UR_H_

#include <stdint.h>
#include "error.h"

#define UR_MAX_PART_COUNT 32
#define UR_PART_DESC_COUNT (UR_MAX_PART_COUNT + 5)

typedef enum {
  CRYPTO_KEYPATH = 0,
  ETH_SIGN_REQUEST = 1,
  CRYPTO_HDKEY = 2,
  ETH_SIGNATURE = 3
} ur_type_t;

typedef struct {
  ur_type_t type;
  uint32_t crc;
  uint32_t part_desc[UR_PART_DESC_COUNT];
  uint8_t part_mask;
  double sampler_probs[UR_MAX_PART_COUNT];
  int sampler_aliases[UR_MAX_PART_COUNT];
  size_t data_max_len;
  size_t data_len;
  uint8_t* data;
} ur_t;

app_err_t ur_process_part(ur_t* ur, const uint8_t* in, size_t in_len);
app_err_t ur_encode(ur_t* ur, char* out, size_t max_len);

#endif
