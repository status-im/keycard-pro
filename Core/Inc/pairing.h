#ifndef __PAIRING_H
#define __PAIRING_H

#include <stdint.h>
#include "main.h"
#include "crypto/sha2.h"
#include "application_info.h"

typedef struct __attribute__ ((aligned (4))) {
  uint8_t instance_uid[APP_INFO_INSTANCE_UID_LEN];
  uint8_t key[SHA256_DIGEST_LENGTH];
  uint8_t idx;
} Pairing;

uint8_t Pairing_Read(Pairing* out);
uint8_t Pairing_Write(Pairing* in);
uint8_t Pairing_Erase(Pairing* in);

#endif