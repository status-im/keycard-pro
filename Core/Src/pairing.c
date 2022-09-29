#include "pairing.h"

#define PAIRING_STORAGE_START_ADDR 0x0807f800
#define PAIRING_STORAGE_END_ADDR 0x0807ffff
#define PAIRING_WORD_SIZE 7

uint8_t Pairing_Read(Pairing* out) {
  uint64_t* read = (uint64_t*)PAIRING_STORAGE_START_ADDR;
  out->idx = 0xff;

  uint64_t dw;
  while((((dw = read[0]) & 0xff00) != 0xff00) && (read < (uint64_t*)PAIRING_STORAGE_END_ADDR)) {
    if (dw == 0) {
      read += PAIRING_WORD_SIZE;
      continue;
    }

    out->idx = (dw & 0xff);
    read++;

    //TODO: fix
    for (int i = 0; i < APP_INFO_INSTANCE_UID_LEN; i++) {
      if (read[i] != out->instance_uid[i]) {
        out->idx = 0xff;
        break;
      }
    }

    if (out->idx == 0xff) {
      read += (PAIRING_WORD_SIZE - 1);
      continue;
    }

    read += 2;

    memcpy(out->key, read, WC_SHA256_DIGEST_SIZE);
  }

  return out->idx != 0xff;
}

uint8_t Pairing_Write(Pairing* in) {
  uint8_t* write = (uint8_t*)PAIRING_STORAGE_START_ADDR;
  return 0;
}