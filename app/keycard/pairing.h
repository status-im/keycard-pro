#ifndef __PAIRING_H
#define __PAIRING_H

#include <stdint.h>
#include "error.h"
#include "crypto/sha2.h"
#include "application_info.h"

typedef struct __attribute__ ((aligned (4))) {
  uint8_t instance_uid[APP_INFO_INSTANCE_UID_LEN];
  uint8_t key[SHA256_DIGEST_LENGTH];
  uint8_t idx;
} pairing_t;

app_err_t pairing_read(pairing_t* out);
app_err_t pairing_write(pairing_t* in);
app_err_t pairing_erase(pairing_t* in);

#endif
