#ifndef __PAIRING_H
#define __PAIRING_H

#include <stdint.h>
#include "wolfssl/wolfcrypt/sha256.h"
#include "application_info.h"

typedef struct {
  uint8_t instance_uid[APP_INFO_INSTANCE_UID_LEN];
  uint8_t idx;
  uint8_t key[WC_SHA256_DIGEST_SIZE];
} Pairing;

uint8_t Pairing_Read(Pairing* out);
uint8_t Pairing_Write(Pairing* in);

#endif