#include <string.h>

#include "keycard.h"
#include "application_info.h"
#include "pairing.h"
#include "error.h"
#include "ui.h"
#include "crypto/rand.h"
#include "crypto/sha2.h"
#include "crypto/util.h"
#include "crypto/pbkdf2.h"

#define KEYCARD_PIN_LEN 6
#define KEYCARD_PUK_LEN 12

const uint8_t keycard_aid[] = {0xa0, 0x00, 0x00, 0x08, 0x04, 0x00, 0x01, 0x01, 0x01};
const uint8_t keycard_aid_len = 9;

const uint8_t keycard_default_psk[] = {0x67, 0x5d, 0xea, 0xbb, 0x0d, 0x7c, 0x72, 0x4b, 0x4a, 0x36, 0xca, 0xad, 0x0e, 0x28, 0x08, 0x26, 0x15, 0x9e, 0x89, 0x88, 0x6f, 0x70, 0x82, 0x53, 0x5d, 0x43, 0x1e, 0x92, 0x48, 0x48, 0xbc, 0xf1};

void Keycard_Init(Keycard* kc, SMARTCARD_HandleTypeDef* dev, TIM_HandleTypeDef* usec_timer) {
  SmartCard_Init(&kc->sc, dev, usec_timer);
  kc->ch.open = 0;
}

uint8_t Keycard_CMD_Select(Keycard* kc) {
  APDU_RESET(&kc->apdu);
  APDU_CLA(&kc->apdu) = 0;
  APDU_INS(&kc->apdu) = 0xa4;
  APDU_P1(&kc->apdu) = 4;
  APDU_P2(&kc->apdu) = 0;
  memcpy(APDU_DATA(&kc->apdu), keycard_aid, keycard_aid_len);
  APDU_SET_LC(&kc->apdu, keycard_aid_len);
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

uint16_t Keycard_Init_Card(Keycard* kc, uint8_t* sc_key) {
  uint8_t pin[6];
  uint8_t puk[12];
  if (!UI_Read_PIN(pin, -1)) {
    return ERR_CANCEL;
  }

  if (!UI_Read_PUK(puk, -1)) {
    return ERR_CANCEL;
  }

  if (Keycard_CMD_Init(kc, sc_key, pin, puk, (uint8_t*)keycard_default_psk) != ERR_OK) {
    UI_Keycard_Init_Failed();
    return ERR_CRYPTO;
  }

  return ERR_OK;
}

uint16_t Keycard_Pair(Keycard* kc, Pairing* pairing, uint8_t* instance_uid) {
  memcpy(pairing->instance_uid, instance_uid, APP_INFO_INSTANCE_UID_LEN);
  
  if (Pairing_Read(pairing)) {
    UI_Keycard_Already_Paired();
    return ERR_OK;
  }

  uint8_t* psk = (uint8_t*) keycard_default_psk;
  
  while(1) {
    if (Keycard_CMD_AutoPair(kc, psk, pairing) == ERR_OK) {
      if (!Pairing_Write(pairing)) {
        UI_Keycard_Flash_Error();
        return ERR_DATA;
      }

      UI_Keycard_Paired();
      return ERR_OK;
    }

    uint8_t password[16];
    uint8_t pairing[32];
    uint32_t len = 16;
    psk = pairing;

    UI_Keycard_Wrong_Pairing();

    if (!UI_Read_Pairing(pairing, &len)) {
      return ERR_CANCEL;
    }

    pbkdf2_hmac_sha256(password, len, (uint8_t*)"Keycard Pairing Password Salt", 30, 50000, pairing, 32);
  }
}

uint16_t Keycard_FactoryReset(Keycard* kc) {
  //TODO: implement global platform
  return ERR_CANCEL;
}

uint16_t Keycard_Unblock(Keycard* kc, uint8_t pukRetries) {
  uint8_t pin[KEYCARD_PIN_LEN];

  if (pukRetries) {
    if (!UI_Prompt_Try_PUK()) {
      pukRetries = 0;
    } else if (!UI_Read_PIN(pin, -1)) {
      return ERR_CANCEL;
    }
  }

  while(pukRetries) {
    uint8_t puk[KEYCARD_PUK_LEN];
    if (!UI_Read_PUK(puk, pukRetries)) {
      return ERR_CANCEL;
    }

    if (!Keycard_CMD_UnblockPIN(kc, pin, puk)) {
      return ERR_TXRX;
    }

    uint16_t sw = APDU_SW(&kc->apdu);

    if (sw == SW_OK) {
      UI_Keycard_PUK_OK();
      return ERR_OK;
    } else if ((sw & 0x63c0) == 0x63c0) {
      UI_Keycard_Wrong_PUK();
      pukRetries = (sw & 0xf);
    } else {
      return sw;
    }    
  }

  return Keycard_FactoryReset(kc);
}

uint16_t Keycard_Authenticate(Keycard* kc) {
  if (!Keycard_CMD_GetStatus(kc)) {
    return ERR_TXRX;
  }

  APDU_ASSERT_OK(&kc->apdu);
  ApplicationStatus pinStatus;
  ApplicationStatus_Parse(APDU_RESP(&kc->apdu), &pinStatus);

  while(pinStatus.pin_retries) {
    SC_BUF(pin, KEYCARD_PIN_LEN);
    if (!UI_Read_PIN(pin, pinStatus.pin_retries)) {
      return ERR_CANCEL;
    }

    if (!Keycard_CMD_VerifyPIN(kc, pin)) {
      return ERR_TXRX;
    }

    uint16_t sw = APDU_SW(&kc->apdu);

    if (sw == SW_OK) {
      UI_Keycard_PIN_OK();
      return ERR_OK;
    } else if ((sw & 0x63c0) == 0x63c0) {
      UI_Keycard_Wrong_PIN();
      pinStatus.pin_retries = (sw & 0xf);
    } else {
      return sw;
    }
  } 

  return Keycard_Unblock(kc, pinStatus.puk_retries);
}

uint16_t Keycard_Init_Keys(Keycard* kc) {
  return ERR_OK;
}

uint16_t Keycard_Setup(Keycard* kc) {
  if (!Keycard_CMD_Select(kc)) {
    return ERR_TXRX;
  }  

  if (APDU_SW(&kc->apdu) != SW_OK) {
    UI_Keycard_Wrong_Card();
    return APDU_SW(&kc->apdu);
  }

  ApplicationInfo info;
  if (!ApplicationInfo_Parse(APDU_RESP(&kc->apdu), &info)) {
    UI_Keycard_Wrong_Card();
    return ERR_DATA;
  }

  uint8_t initKeys;
  uint16_t err;

  switch (info.status) {
    case NOT_INITIALIZED:
      UI_Keycard_Not_Initalized();
      err = Keycard_Init_Card(kc, info.sc_key);
      if (err != ERR_OK) {
        return err;
      }
      return ERR_RETRY;
    case INIT_NO_KEYS:
      initKeys = 1;
      UI_Keycard_No_Keys();
      break;
    case INIT_WITH_KEYS:
      initKeys = 0;
      UI_Keycard_Ready();
      break;
    default:
      return ERR_DATA;
  }

  Pairing pairing;
  err = Keycard_Pair(kc, &pairing, info.instance_uid);
  if (err != ERR_OK) {
    return err;
  }

  if (SecureChannel_Open(&kc->ch, &kc->sc, &kc->apdu, &pairing, info.sc_key) != ERR_OK) {
    Pairing_Erase(&pairing);
    UI_Keycard_Secure_Channel_Failed();
    return ERR_RETRY;
  }

  UI_Keycard_Secure_Channel_OK();

  err = Keycard_Authenticate(kc);
  if (err != ERR_OK) {
    return err;
  }

  if (initKeys) {
    return Keycard_Init_Keys(kc);
  } else {
    return ERR_OK;
  }
}

void Keycard_Activate(Keycard* kc) {
  SmartCard_Activate(&kc->sc);
  
  UI_Card_Accepted();

  if (kc->sc.state != SC_READY) {
    UI_Card_Transport_Error();
    return;
  }

  UI_Clear();

  uint16_t res;
  do {
    res = Keycard_Setup(kc);
  } while(res == ERR_RETRY);

  if (res != ERR_OK) {
    UI_Card_Transport_Error();
    SmartCard_Deactivate(&kc->sc);
  }
}

void Keycard_Run(Keycard* kc) {
  switch (kc->sc.state) {
    case SC_NOT_PRESENT:
    case SC_DEACTIVATED:
      break; // sleep unil interrupt!
    case SC_OFF:
      Keycard_Activate(kc);
      break;
    case SC_READY:
      break; // process commands
  }
}

void Keycard_In(Keycard* kc) {
  UI_Card_Inserted();
  SmartCard_In(&kc->sc);
}

void Keycard_Out(Keycard* kc) {
  UI_Card_Removed();
  SmartCard_Out(&kc->sc);
  kc->ch.open = 0;
}
