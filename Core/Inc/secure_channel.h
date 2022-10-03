#ifndef __SECURE_CHANNEL
#define __SECURE_CHANNEL

#include "pairing.h"
#include "smartcard.h"

#define AES_256_KEY_SIZE 32
#define AES_IV_SIZE 16

typedef struct __attribute__((packed)) {
  uint8_t encKey[AES_256_KEY_SIZE];
  uint8_t macKey[AES_256_KEY_SIZE];
  uint8_t iv[AES_IV_SIZE];
  uint8_t open;
} SecureChannel;

uint16_t SecureChannel_Open(SecureChannel* sc, SmartCard* card, APDU* apdu, Pairing* pairing, uint8_t* sc_pub);
uint16_t SecureChannel_Protect_APDU(SecureChannel *sc, APDU* apdu, uint8_t* data, uint32_t len);
uint16_t SecureChannel_Decrypt_APDU(SecureChannel *sc, APDU* apdu);
void SecureChannel_Close(SecureChannel* sc);

#endif