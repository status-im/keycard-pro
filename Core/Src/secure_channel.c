#include <string.h>

#include "secure_channel.h"
#include "crypto/ecdsa.h"
#include "crypto/sha2.h"
#include "crypto/rand.h"
#include "crypto/secp256k1.h"
#include "error.h"

#define SECP256K1_KEYLEN 32
#define SECP256K1_PUBLEN 65

uint16_t SecureChannel_Mutual_Authenticate(SecureChannel* sc, SmartCard* card, APDU* apdu) {
  uint8_t data[32];
  random_buffer(data, 32);
  APDU_RESET(apdu);
  APDU_CLA(apdu) = 0x80;
  APDU_INS(apdu) = 0x11;
  APDU_P1(apdu) = 0;
  APDU_P2(apdu) = 0;
  
  if (SecureChannel_Protect_APDU(sc, apdu, data, 32) != ERR_OK) {
    return ERR_CRYPTO;
  }

  if (!SmartCard_Send_APDU(card, apdu)) {
    return ERR_TXRX;
  }

  if (SecureChannel_Decrypt_APDU(sc, apdu) != ERR_OK) {
    return ERR_CRYPTO;
  }

  APDU_ASSERT_OK(apdu);

  return ERR_OK;
}

uint16_t SecureChannel_Open(SecureChannel* sc, SmartCard* card, APDU* apdu, Pairing* pairing, uint8_t* sc_pub) {
  uint8_t priv[SECP256K1_KEYLEN];
  random_buffer(priv, SECP256K1_KEYLEN);
  APDU_RESET(apdu);
  APDU_CLA(apdu) = 0x80;
  APDU_INS(apdu) = 0x10;
  APDU_P1(apdu) = pairing->idx;
  APDU_P2(apdu) = 0;
  if (ecdsa_get_public_key65(&secp256k1, priv, APDU_DATA(apdu)) != 0) {
    return ERR_CRYPTO;
  }
  APDU_SET_LC(apdu, SECP256K1_PUBLEN);
  APDU_SET_LE(apdu, 48);
  
  if (!SmartCard_Send_APDU(card, apdu)) {
    return ERR_TXRX;
  }

  APDU_ASSERT_OK(apdu);

  uint8_t secret[SECP256K1_PUBLEN];

  if (ecdh_multiply(&secp256k1, priv, sc_pub, secret) != 0) {
    return ERR_CRYPTO;
  }

  SHA512_CTX sha512 = {0};
  sha512_Init(&sha512);
  sha512_Update(&sha512, APDU_RESP(apdu), SHA256_DIGEST_LENGTH);
  sha512_Update(&sha512, pairing->key, SHA256_DIGEST_LENGTH);
  sha512_Update(&sha512, &secret[1], SHA256_DIGEST_LENGTH);
  sha512_Final(&sha512, sc->encKey);
  
  memcpy(sc->iv, &APDU_RESP(apdu)[32], AES_IV_SIZE);
  sc->open = 1;

  return SecureChannel_Mutual_Authenticate(sc, card, apdu);
}

uint16_t SecureChannel_Protect_APDU(SecureChannel *sc, APDU* apdu, uint8_t* data, uint32_t len) {
  return 0;
}

uint16_t SecureChannel_Decrypt_APDU(SecureChannel *sc, APDU* apdu) {
  return 0;
}

void SecureChannel_Close(SecureChannel* sc) {
  sc->open = 0;
}