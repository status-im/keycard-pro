#ifndef __KEYS__
#define __KEYS__

#include <stdint.h>
#include "error.h"

typedef enum {
  DEV_AUTH_PRIV_KEY,
  DEV_AUTH_SERVER_KEY,
  FW_VERIFICATION_KEY,
  DB_VERIFICATION_KEY
} key_type_t;

app_err_t key_read(key_type_t type, const uint8_t** out);

#endif