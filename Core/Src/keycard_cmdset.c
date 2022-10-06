#include <string.h>

#include "keycard_cmdset.h"
#include "error.h"
#include "crypto/rand.h"
#include "crypto/sha2.h"
#include "crypto/util.h"

const extern uint8_t keycard_default_psk[];

uint8_t Keycard_CMD_Select(Keycard* kc, const uint8_t* aid, uint32_t len) {
  APDU_RESET(&kc->apdu);
  APDU_CLA(&kc->apdu) = 0;
  APDU_INS(&kc->apdu) = 0xa4;
  APDU_P1(&kc->apdu) = 4;
  APDU_P2(&kc->apdu) = 0;
  memcpy(APDU_DATA(&kc->apdu), aid, len);
  APDU_SET_LC(&kc->apdu, len);
  APDU_SET_LE(&kc->apdu, 0);

  return SmartCard_Send_APDU(&kc->sc, &kc->apdu);
}

uint8_t Keycard_CMD_Pair(Keycard* kc, uint8_t step, uint8_t* data) {
  APDU_RESET(&kc->apdu);
  APDU_CLA(&kc->apdu) = 0x80;
  APDU_INS(&kc->apdu) = 0x12;
  APDU_P1(&kc->apdu) = step;
  APDU_P2(&kc->apdu) = 0;
  memcpy(APDU_DATA(&kc->apdu), data, 32);  
  APDU_SET_LC(&kc->apdu, 32);
  APDU_SET_LE(&kc->apdu, 0);  

  return SmartCard_Send_APDU(&kc->sc, &kc->apdu);
}

uint16_t Keycard_CMD_AutoPair(Keycard* kc, const uint8_t* psk, Pairing* pairing) {
  uint8_t buf[SHA256_DIGEST_LENGTH];
  random_buffer(buf, SHA256_DIGEST_LENGTH);

  if (!Keycard_CMD_Pair(kc, 0, buf)) {
    return ERR_TXRX;
  }

  APDU_ASSERT_OK(&kc->apdu);

  uint8_t* card_cryptogram = APDU_RESP(&kc->apdu);
  uint8_t* card_challenge = &card_cryptogram[SHA256_DIGEST_LENGTH];

  SHA256_CTX sha256 = {0};
  sha256_Init(&sha256);
  sha256_Update(&sha256, psk, SHA256_DIGEST_LENGTH);
  sha256_Update(&sha256, buf, SHA256_DIGEST_LENGTH);
  sha256_Final(&sha256, buf);

  if (memcmp_ct(card_cryptogram, buf, SHA256_DIGEST_LENGTH) != 0) {
    return ERR_CRYPTO;
  }

  sha256_Init(&sha256);
  sha256_Update(&sha256, psk, SHA256_DIGEST_LENGTH);
  sha256_Update(&sha256, card_challenge, SHA256_DIGEST_LENGTH);
  sha256_Final(&sha256, buf);

  if (!Keycard_CMD_Pair(kc, 1, buf)) {
    return ERR_TXRX;
  }

  APDU_ASSERT_OK(&kc->apdu);

  pairing->idx = APDU_RESP(&kc->apdu)[0];
  uint8_t *salt = APDU_RESP(&kc->apdu) + 1;

	sha256_Init(&sha256);
  sha256_Update(&sha256, psk, SHA256_DIGEST_LENGTH);
  sha256_Update(&sha256, salt, SHA256_DIGEST_LENGTH);
  sha256_Final(&sha256, pairing->key);

  return ERR_OK;
}

uint8_t Keycard_CMD_VerifyPIN(Keycard* kc, uint8_t* pin) {
  APDU_RESET(&kc->apdu);
  APDU_CLA(&kc->apdu) = 0x80;
  APDU_INS(&kc->apdu) = 0x20;
  APDU_P1(&kc->apdu) = 0;
  APDU_P2(&kc->apdu) = 0;
  
  return SecureChannel_Send_APDU(&kc->sc, &kc->ch, &kc->apdu, pin, KEYCARD_PIN_LEN) == ERR_OK;
}

uint8_t Keycard_CMD_UnblockPIN(Keycard* kc, uint8_t* pin, uint8_t* puk) {
  APDU_RESET(&kc->apdu);
  APDU_CLA(&kc->apdu) = 0x80;
  APDU_INS(&kc->apdu) = 0x22;
  APDU_P1(&kc->apdu) = 0;
  APDU_P2(&kc->apdu) = 0;

  SC_BUF(data, (KEYCARD_PUK_LEN + KEYCARD_PIN_LEN));
  memcpy(data, puk, KEYCARD_PUK_LEN);
  memcpy(&data[KEYCARD_PUK_LEN], pin, KEYCARD_PIN_LEN);
  
  return SecureChannel_Send_APDU(&kc->sc, &kc->ch, &kc->apdu, data, (KEYCARD_PUK_LEN + KEYCARD_PIN_LEN)) == ERR_OK;
}

uint8_t Keycard_CMD_GetStatus(Keycard* kc) {
  APDU_RESET(&kc->apdu);
  APDU_CLA(&kc->apdu) = 0x80;
  APDU_INS(&kc->apdu) = 0xf2;
  APDU_P1(&kc->apdu) = 0;
  APDU_P2(&kc->apdu) = 0;

  SC_BUF(data, 0);
  
  return SecureChannel_Send_APDU(&kc->sc, &kc->ch, &kc->apdu, data, 0) == ERR_OK;
}

uint16_t Keycard_CMD_Init(Keycard* kc, uint8_t* sc_pub, uint8_t* pin, uint8_t* puk, uint8_t* psk) {
  SC_BUF(data, (KEYCARD_PIN_LEN + KEYCARD_PUK_LEN + SHA256_DIGEST_LENGTH));
  memcpy(data, pin, KEYCARD_PIN_LEN);
  memcpy(&data[KEYCARD_PIN_LEN], puk, KEYCARD_PUK_LEN);
  memcpy(&data[KEYCARD_PIN_LEN+KEYCARD_PUK_LEN], psk, SHA256_DIGEST_LENGTH);

  memset(pin, 0, KEYCARD_PIN_LEN);
  memset(puk, 0, KEYCARD_PUK_LEN);

  if (psk != keycard_default_psk) {
    memset(psk, 0, SHA256_DIGEST_LENGTH);
  }

  return SecureChannel_Init(&kc->sc, &kc->apdu, sc_pub, data, KEYCARD_PIN_LEN+KEYCARD_PUK_LEN+SHA256_DIGEST_LENGTH);
}