#include "ui.h"

void UI_Init() {
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

void UI_Card_Inserted() {
  BSP_LCD_ClearStringLine(4);
  BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Card IN");
}

void UI_Card_Removed() {
  BSP_LED_Off(LED4);
  BSP_LED_Off(LED3);
  BSP_LCD_ClearStringLine(4);
  BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Card OUT");
}

void UI_Card_Transport_Error() {
  BSP_LED_On(LED3);
}

void UI_Card_Accepted() {
  BSP_LED_On(LED4);
}

void UI_Clear() {
  BSP_LCD_ClearStringLine(3);
  BSP_LCD_ClearStringLine(4);
  BSP_LCD_ClearStringLine(5);
  BSP_LCD_ClearStringLine(6);
  BSP_LCD_ClearStringLine(7);
}

void UI_Fatal() {
  BSP_LCD_ClearStringLine(4);
  BSP_LCD_DisplayStringAtLine(4, (uint8_t*)"Error!");  
}

void UI_Keycard_Wrong_Card() {
  BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Not a Keycard!");
}

void UI_Keycard_Not_Initalized() {
  BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Not initialized!");
}

void UI_Keycard_Init_Failed() {
  BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Initialization failed!");
}

void UI_Keycard_No_Keys() {
  BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Card has no keys!");
}

void UI_Keycard_Ready() {
  BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Ready for signing!");
}

void UI_Keycard_Paired() {
  BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Pairing succesful!");
}

void UI_Keycard_Already_Paired() {
  BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Already paired!");
}

void UI_Keycard_Wrong_Pairing() {
  BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Pairing failed");
}

void UI_Keycard_Flash_Error() {
  BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Write error!");
}

void UI_Keycard_Secure_Channel_Failed() {
  BSP_LCD_DisplayStringAtLine(5, (uint8_t*) "Failure opening SC");
}

void UI_Keycard_Secure_Channel_OK() {
  BSP_LCD_DisplayStringAtLine(5, (uint8_t*) "SecureChannel opened");
}

void UI_Keycard_Wrong_PIN() {
  BSP_LCD_DisplayStringAtLine(6, (uint8_t*) "Wrong PIN");
}

void UI_Keycard_PIN_OK() {
  BSP_LCD_DisplayStringAtLine(6, (uint8_t*) "Authenticated");
}

uint8_t UI_Read_PIN(uint8_t* out) {
  out[0] = '1';
  out[1] = '2';
  out[2] = '3';
  out[3] = '4';
  out[4] = '5';
  out[5] = '6';

  return 1;
}

uint8_t UI_Read_PUK(uint8_t* out) {
  out[0] = '1';
  out[1] = '2';
  out[2] = '3';
  out[3] = '4';
  out[4] = '5';
  out[5] = '6';
  out[6] = '1';
  out[7] = '2';
  out[8] = '3';
  out[9] = '4';
  out[10] = '5';
  out[11] = '6';

  return 1;
}