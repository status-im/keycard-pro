#include "keycard.h"

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

void Keycard_Run(SmartCard* sc) {
        APDU apdu;

  switch (sc->state) {
    case SC_NOT_PRESENT:
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
      APDU_RESET(&apdu);
      APDU_CLA(&apdu) = 0;
      APDU_INS(&apdu) = 0xa4;
      APDU_P1(&apdu) = 4;
      APDU_P2(&apdu) = 0;
      APDU_SET_LE(&apdu, 0);
      if (!SmartCard_Send_APDU(sc, &apdu)) {
        SmartCard_Deactivate(sc);
        return;
      }
      if (APDU_SW(&apdu) == 0x9000) {
        BSP_LCD_ClearStringLine(4);
        BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Success!");
      }
      sc->state = SC_NOT_PRESENT;
      break; // process commands
  }
}
