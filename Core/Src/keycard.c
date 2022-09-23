#include "keycard.h"

void Keycard_Activate(SmartCard* sc) {
  BSP_LED_Off(LED1);
  BSP_LED_Off(LED2);
  BSP_LED_Off(LED3);
  BSP_LED_Off(LED4);
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
  switch (sc->state) {
    case SC_NOT_PRESENT:
      break; // sleep unil interrupt!
    case SC_OFF:
      Keycard_Activate(sc);
      break;
    case SC_DEACTIVATED:
      break;
    case SC_READY:
      /*// Test code to remove
      HAL_Delay(100);
      APDU _apdu;
      APDU* apdu = &_apdu;
      APDU_RESET(apdu);
      APDU_CLA(apdu) = 0;
      APDU_INS(apdu) = 0xa4;
      APDU_P1(apdu) = 4;
      APDU_P2(apdu) = 0;
      APDU_SET_LE(apdu, 0);
      if (!T1_Transmit(sc, apdu)) {
        SmartCard_Deactivate(sc);
        return;
      }*/
      break; // process commands
  }
}
