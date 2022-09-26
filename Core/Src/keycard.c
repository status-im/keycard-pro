#include <string.h>

#include "keycard.h"
#include "application_info.h"

const uint8_t keycard_aid[] = {0xa0, 0x00, 0x00, 0x08, 0x04, 0x00, 0x01, 0x01, 0x01};
const uint8_t keycard_aid_len = 9;

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
  BSP_LCD_DisplayStringAtLine(2, (uint8_t*) "Keycard Pro");  
  BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Waiting for card...");
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

void Keycard_Test(SmartCard* sc) {
  APDU apdu;
  if (!Keycard_CMD_Select(sc, &apdu)) {
    SmartCard_Deactivate(sc);
    return;
  }

  BSP_LCD_ClearStringLine(4);

  if (APDU_SW(&apdu) == 0x9000) {
    ApplicationInfo info;
    if (!ApplicationInfo_Parse(APDU_RESP(&apdu), &info)) {
      return;
    }

    switch (info.status) {
      case NOT_INITIALIZED:
        BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Not initialized!");
        break;
      case INIT_NO_KEYS:
        BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Card has no keys!");
        break;
      case INIT_WITH_KEYS:
        BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Ready for signing!");
        break;
    }
  } else {
    BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Not a Keycard!");
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
