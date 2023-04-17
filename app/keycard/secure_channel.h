#ifndef __SECURE_CHANNEL
#define __SECURE_CHANNEL

#include "pairing.h"
#include "iso7816/smartcard.h"
#include "crypto/aes.h"
#include "error.h"

#define SC_PAD AES_IV_SIZE

#define SC_BUF(__NAME__, __LEN__) uint8_t __NAME__[__LEN__+SC_PAD] __attribute__((aligned(4)))

typedef struct __attribute__((packed, aligned(4))) {
  uint8_t encKey[AES_256_KEY_SIZE];
  uint8_t macKey[AES_256_KEY_SIZE];
  uint8_t iv[AES_IV_SIZE];
  uint8_t open;
} SecureChannel;

app_err_t SecureChannel_Open(SecureChannel* sc, SmartCard* card, APDU* apdu, Pairing* pairing, uint8_t* sc_pub);
app_err_t SecureChannel_Init(SmartCard* card, APDU* apdu, uint8_t* sc_pub, uint8_t* data, uint32_t len);
app_err_t SecureChannel_Protect_APDU(SecureChannel *sc, APDU* apdu, uint8_t* data, uint32_t len);
app_err_t SecureChannel_Decrypt_APDU(SecureChannel *sc, APDU* apdu);
app_err_t SecureChannel_Send_APDU(SmartCard* card, SecureChannel *sc, APDU* apdu, uint8_t* data, uint32_t len);
void SecureChannel_Close(SecureChannel* sc);

#endif
