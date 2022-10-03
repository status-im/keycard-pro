#include <string.h>
#include "aes.h"

#define AES_TIMEOUT 1000

const static uint8_t cmac_iv[AES_IV_SIZE] __attribute__((aligned(4))) = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static CRYP_HandleTypeDef* _aes;

void aes_init(CRYP_HandleTypeDef* aes) {
  _aes = aes;
}

uint8_t aes_encrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* data, uint32_t len, uint8_t* out) {
  _aes->Init.pKey = (uint32_t*) key;
  _aes->Init.pInitVect = (uint32_t*) iv;
  
  return HAL_CRYP_Encrypt(_aes, (uint32_t*)data, len, (uint32_t*)out, AES_TIMEOUT) == HAL_OK;
}

uint8_t aes_decrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* data, uint32_t len, uint8_t* out) {
  _aes->Init.pKey = (uint32_t*) key;
  _aes->Init.pInitVect = (uint32_t*) iv;

  return HAL_CRYP_Decrypt(_aes, (uint32_t*)data, len, (uint32_t*)out, AES_TIMEOUT) == HAL_OK;
}

//TODO: Must be reimplemented completely
static uint8_t cmac_tmp[256] __attribute__((aligned(4)));
uint8_t aes_cmac(const uint8_t* key, const uint8_t* data, uint32_t len, uint8_t* out) {
  if (len >= 256) {
    return 0;
  }

  if (!aes_encrypt(key, cmac_iv, data, len, cmac_tmp)) {
    return 0;
  }

  memcpy(out, &cmac_tmp[(len-AES_IV_SIZE)-1], AES_IV_SIZE);

  return 1;
}