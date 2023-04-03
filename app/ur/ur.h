#ifndef _UR_H_
#define _UR_H_

#include <stdint.h>
#include "error.h"

#define UR_MAX_DATA_LEN 512

typedef enum {
  CRYPTO_KEYPATH = 0,
  ETH_SIGN_REQUEST = 1,
  CRYPTO_HDKEY = 2,
  ETH_SIGNATURE = 3
} ur_type_t;

typedef struct {
  ur_type_t type;
  uint8_t is_multipart;
  uint8_t is_complete;
  size_t data_len;
  uint8_t data[UR_MAX_DATA_LEN];
} ur_t;

app_err_t ur_process_part(ur_t* ur, const uint8_t* in, size_t in_len);

#endif
