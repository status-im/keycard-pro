#include <string.h>

#include "keycard.h"
#include "application_info.h"
#include "pairing.h"
#include "secure_channel.h"
#include "error.h"
#include "crypto/rand.h"
#include "crypto/sha2.h"
#include "crypto/util.h"

#define KEYCARD_PIN_LEN 6
#define KEYCARD_PUK_LEN 12

const uint8_t keycard_aid[] = {0xa0, 0x00, 0x00, 0x08, 0x04, 0x00, 0x01, 0x01, 0x01};
const uint8_t keycard_aid_len = 9;

const uint8_t keycard_default_psk[] = {0x67, 0x5d, 0xea, 0xbb, 0x0d, 0x7c, 0x72, 0x4b, 0x4a, 0x36, 0xca, 0xad, 0x0e, 0x28, 0x08, 0x26, 0x15, 0x9e, 0x89, 0x88, 0x6f, 0x70, 0x82, 0x53, 0x5d, 0x43, 0x1e, 0x92, 0x48, 0x48, 0xbc, 0xf1};

static int tested = 0;

void Keycard_Activate(SmartCard* sc) {
  SmartCard_Activate(sc);
}

void Keycard_Init() {
  BSP_LED_Off(LED1);
  BSP_LED_Off(LED2);
  BSP_LED_Off(LED3);
  BSP_LED_Off(LED4);

  BSP_LCD_Init();
  BSP_LCD_DisplayOn();
  BSP_LCD_Clear(LCD_COLOR_ST_BLUE_DARK);
  BSP_LCD_DisplayOn();
  BSP_LCD_SetBackColor(LCD_COLOR_ST_BLUE_DARK);
  BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
  BSP_LCD_SetFont(&Font24);
  BSP_LCD_DisplayStringAtLine(1, (uint8_t*) "Keycard Pro");  
  BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Waiting for card...");
}

uint8_t Keycard_CMD_Select(SmartCard* sc, APDU* apdu) {
  APDU_RESET(apdu);
  APDU_CLA(apdu) = 0;
  APDU_INS(apdu) = 0xa4;
  APDU_P1(apdu) = 4;
  APDU_P2(apdu) = 0;
  memcpy(APDU_DATA(apdu), keycard_aid, keycard_aid_len);
  APDU_SET_LC(apdu, keycard_aid_len);
  APDU_SET_LE(apdu, 0);

  return SmartCard_Send_APDU(sc, apdu);
}

uint8_t Keycard_CMD_Pair(SmartCard* sc, APDU* apdu, uint8_t step, uint8_t* data) {
  APDU_RESET(apdu);
  APDU_CLA(apdu) = 0x80;
  APDU_INS(apdu) = 0x12;
  APDU_P1(apdu) = step;
  APDU_P2(apdu) = 0;
  memcpy(APDU_DATA(apdu), data, 32);  
  APDU_SET_LC(apdu, 32);
  APDU_SET_LE(apdu, 0);  

  return SmartCard_Send_APDU(sc, apdu);
}

uint16_t Keycard_CMD_AutoPair(SmartCard* sc, APDU* apdu, const uint8_t* psk, Pairing* pairing) {
  uint8_t buf[SHA256_DIGEST_LENGTH];
  random_buffer(buf, SHA256_DIGEST_LENGTH);

  if (!Keycard_CMD_Pair(sc, apdu, 0, buf)) {
    return ERR_TXRX;
  }

  APDU_ASSERT_OK(apdu);

  uint8_t* card_cryptogram = APDU_RESP(apdu);
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

  if (!Keycard_CMD_Pair(sc, apdu, 1, buf)) {
    return ERR_TXRX;
  }

  APDU_ASSERT_OK(apdu);

  pairing->idx = APDU_RESP(apdu)[0];
  uint8_t *salt = APDU_RESP(apdu) + 1;

	sha256_Init(&sha256);
  sha256_Update(&sha256, psk, SHA256_DIGEST_LENGTH);
  sha256_Update(&sha256, salt, SHA256_DIGEST_LENGTH);
  sha256_Final(&sha256, pairing->key);

  return ERR_OK;
}

uint8_t Keycard_CMD_VerifyPIN(SmartCard* sc, SecureChannel* ch, APDU* apdu, uint8_t* pin) {
  APDU_RESET(apdu);
  APDU_CLA(apdu) = 0x80;
  APDU_INS(apdu) = 0x20;
  APDU_P1(apdu) = 0;
  APDU_P2(apdu) = 0;
  
  if (SecureChannel_Protect_APDU(ch, apdu, pin, KEYCARD_PIN_LEN) != ERR_OK) {
    return 0;
  }

  if (!SmartCard_Send_APDU(sc, apdu)) {
    return 0;
  }

  if (SecureChannel_Decrypt_APDU(ch, apdu) != ERR_OK) {
    return 0;
  }

  return 1;
}

uint16_t Keycard_CMD_Init(SmartCard* sc, APDU* apdu, uint8_t* sc_pub, uint8_t* pin, uint8_t* puk, uint8_t* psk) {
  SC_BUF(data, (KEYCARD_PIN_LEN + KEYCARD_PUK_LEN + SHA256_DIGEST_LENGTH));
  memcpy(data, pin, KEYCARD_PIN_LEN);
  memcpy(&data[KEYCARD_PIN_LEN], puk, KEYCARD_PUK_LEN);
  memcpy(&data[KEYCARD_PIN_LEN+KEYCARD_PUK_LEN], psk, SHA256_DIGEST_LENGTH);

  memset(pin, 0, KEYCARD_PIN_LEN);
  memset(puk, 0, KEYCARD_PUK_LEN);

  if (psk != keycard_default_psk) {
    memset(psk, 0, SHA256_DIGEST_LENGTH);
  }

  return SecureChannel_Init(sc, apdu, sc_pub, data, KEYCARD_PIN_LEN+KEYCARD_PUK_LEN+SHA256_DIGEST_LENGTH);
}

void Keycard_Test(SmartCard* sc) {
  APDU apdu;
  if (!Keycard_CMD_Select(sc, &apdu)) {
    SmartCard_Deactivate(sc);
    return;
  }

  BSP_LCD_ClearStringLine(3);
  BSP_LCD_ClearStringLine(4);
  BSP_LCD_ClearStringLine(5);
  BSP_LCD_ClearStringLine(6);
  BSP_LCD_ClearStringLine(7);

  if (APDU_SW(&apdu) == 0x9000) {
    ApplicationInfo info;
    if (!ApplicationInfo_Parse(APDU_RESP(&apdu), &info)) {
      return;
    }

    switch (info.status) {
      case NOT_INITIALIZED:
        BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Not initialized!");
        uint8_t pin[6] = {'1', '2', '3', '4', '5', '6'};
        uint8_t puk[12]  = {'1', '2', '3', '4', '5', '6', '1', '2', '3', '4', '5', '6'};
        if (Keycard_CMD_Init(sc, &apdu, info.sc_key, pin, puk, (uint8_t*)keycard_default_psk) != ERR_OK) {
          BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Initialization failed!");
          return;
        }
        Keycard_Test(sc);
        return;
      case INIT_NO_KEYS:
        BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Card has no keys!");
        break;
      case INIT_WITH_KEYS:
        BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Ready for signing!");
        break;
    }

    Pairing pairing;
    memcpy(pairing.instance_uid, info.instance_uid, APP_INFO_INSTANCE_UID_LEN);
    if (!Pairing_Read(&pairing)) {
      if (Keycard_CMD_AutoPair(sc, &apdu, keycard_default_psk, &pairing) == ERR_OK) {
        if (!Pairing_Write(&pairing)) {
          BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Write error!");
          return;
        } else {
          BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Pairing succesful!");
        }
      } else {
        BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Pairing failed");
        return;
      }
    } else {
      BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Already paired!");
    }

    SecureChannel ch;
    if (SecureChannel_Open(&ch, sc, &apdu, &pairing, info.sc_key) != ERR_OK) {
      Pairing_Erase(&pairing);
      BSP_LCD_DisplayStringAtLine(5, (uint8_t*) "Failure opening SC");
      return;
    }

    BSP_LCD_DisplayStringAtLine(5, (uint8_t*) "SecureChannel opened");

    SC_BUF(pin, KEYCARD_PIN_LEN);
    pin[0] = '1';
    pin[1] = '2';
    pin[2] = '3';
    pin[3] = '4';
    pin[4] = '5';
    pin[5] = '6';

    if (!Keycard_CMD_VerifyPIN(sc, &ch, &apdu, pin) || (APDU_SW(&apdu) != 0x9000)) {
      BSP_LCD_DisplayStringAtLine(6, (uint8_t*) "Wrong PIN");
      return;
    }

    BSP_LCD_DisplayStringAtLine(6, (uint8_t*) "Authenticated");

  } else {
    BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Not a Keycard!");
  }
}

void Keycard_Run(SmartCard* sc) {
  switch (sc->state) {
    case SC_NOT_PRESENT:
      tested = 0;
      BSP_LED_Off(LED4);
      BSP_LED_Off(LED3);
      break; // sleep unil interrupt!
    case SC_OFF:
      Keycard_Activate(sc);
      BSP_LED_On(LED4);
      break;
    case SC_DEACTIVATED:
      BSP_LED_Off(LED4);
      BSP_LED_On(LED3);
      break;
    case SC_READY:
      // Test code to remove
      if (!tested) {
        Keycard_Test(sc);
        tested = 1;
      }
      break; // process commands
  }
}
