#ifndef __AES_H__
#define __AES_H__

#include "main.h"
#include <stdint.h>

#define AES_256_KEY_SIZE 32
#define AES_IV_SIZE 16

void aes_init(CRYP_HandleTypeDef* aes);

uint8_t aes_encrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* data, uint32_t len, uint8_t* out);
uint8_t aes_decrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* data, uint32_t len, uint8_t* out);
uint8_t aes_cmac(const uint8_t* key, const uint8_t* data, uint32_t len, uint8_t* out);

#define aes_import_param(__OUT__, __IN__, __LEN__) rev32_all((uint32_t*)__OUT__, (uint32_t*)__IN__, __LEN__)

#endif