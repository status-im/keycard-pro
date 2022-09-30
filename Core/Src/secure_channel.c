#include "secure_channel.h"
#include "wolfssl/wolfcrypt/cmac.h"
#include "wolfssl/wolfcrypt/ecc.h"
#include "wolfssl/wolfcrypt/sha512.h"
#include "error.h"

uint16_t SecureChannel_Mutual_Authenticate(SecureChannel* sc, WC_RNG* rng, SmartCard* card, APDU* apdu) {
  uint8_t data[32];
  wc_RNG_GenerateBlock(rng, data, 32);
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

uint16_t SecureChannel_Open(SecureChannel* sc, WC_RNG* rng, SmartCard* card, APDU* apdu, Pairing* pairing, uint8_t* sc_pub) {
  ecc_key priv;
  wc_ecc_init(&priv);
  wc_ecc_make_key_ex(rng, 32, &priv, ECC_SECP256K1);

  APDU_RESET(apdu);
  APDU_CLA(apdu) = 0x80;
  APDU_INS(apdu) = 0x10;
  APDU_P1(apdu) = pairing->idx;
  APDU_P2(apdu) = 0;
  word32 len = 65;
  wc_ecc_export_x963_ex(&priv, APDU_DATA(apdu), &len, 0);
  APDU_SET_LC(apdu, len);
  APDU_SET_LE(apdu, 48);
  
  if (!SmartCard_Send_APDU(card, apdu)) {
    return ERR_TXRX;
  }

  APDU_ASSERT_OK(apdu);

  ecc_key pub;
  wc_ecc_init(&pub);
  if (wc_ecc_import_x963_ex(sc_pub, 65, &pub, ECC_SECP256K1) != 0) {
    return ERR_CRYPTO;
  }

  uint8_t secret[32];
  len = 32;

  if (wc_ecc_shared_secret(&priv, &pub, secret, &len) != 0) {
    return ERR_CRYPTO;
  }

  Sha512 sha512;
  wc_InitSha512(&sha512);
  wc_Sha512Update(&sha512, APDU_RESP(apdu), SHA256_DIGEST_SIZE);
  wc_Sha512Update(&sha512, pairing->key, SHA256_DIGEST_SIZE);
  wc_Sha512Update(&sha512, secret, SHA256_DIGEST_SIZE);
  wc_Sha512Final(&sha512, sc->encKey);
  
  memcpy(sc->iv, &APDU_RESP(apdu)[32], AES_IV_SIZE);
  sc->open = 1;

  return SecureChannel_Mutual_Authenticate(sc, rng, card, apdu);
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