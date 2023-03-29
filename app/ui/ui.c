#include "ui.h"
#include "error.h"



void ui_card_inserted() {
 /* BSP_LCD_ClearStringLine(4);
  BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Card IN");*/
}

void ui_card_removed() {
 /* BSP_LED_Off(LED4);
  BSP_LED_Off(LED3);
  BSP_LCD_ClearStringLine(4);
  BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Card OUT");*/
}

void ui_card_transport_error() {
  //BSP_LED_On(LED3);
}

void ui_card_accepted() {
  //BSP_LED_On(LED4);
}

void ui_clear() {
  //BSP_LCD_ClearStringLine(3);
  //BSP_LCD_ClearStringLine(4);
  //BSP_LCD_ClearStringLine(5);
  //BSP_LCD_ClearStringLine(6);
  //BSP_LCD_ClearStringLine(7);
}

void ui_fatal() {
 // BSP_LCD_ClearStringLine(4);
 // BSP_LCD_DisplayStringAtLine(4, (uint8_t*)"Error!");
}

void ui_keycard_wrong_card() {
  //BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Not a Keycard!");
}

void ui_keycard_not_initialized() {
 // BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Not initialized!");
}

void ui_keycard_init_failed() {
 // BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Initialization failed!");
}

void ui_keycard_no_keys() {
 // BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Card has no keys!");
}

void ui_keycarrd_ready() {
 // BSP_LCD_DisplayStringAtLine(3, (uint8_t*) "Ready for signing!");
}

void ui_keyard_paired() {
 // BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Pairing succesful!");
}

void ui_keycard_already_paired() {
 // BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Already paired!");
}

void ui_keycard_pairing_failed() {
 // BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Pairing failed");
}

void ui_keycard_flash_failed() {
 // BSP_LCD_DisplayStringAtLine(4, (uint8_t*) "Write error!");
}

void ui_keycard_secure_channel_failed() {
 // BSP_LCD_DisplayStringAtLine(5, (uint8_t*) "Failure opening SC");
}

void ui_keycard_secure_channel_ok() {
 // BSP_LCD_DisplayStringAtLine(5, (uint8_t*) "SecureChannel opened");
}

void ui_keycard_wrong_pin() {
//  BSP_LCD_DisplayStringAtLine(6, (uint8_t*) "Wrong PIN");
}

void ui_keycard_pin_ok() {
 // BSP_LCD_DisplayStringAtLine(6, (uint8_t*) "Authenticated");
}

void ui_keycard_puk_ok() {
 // BSP_LCD_DisplayStringAtLine(6, (uint8_t*) "PIN changed");
}

void ui_keycard_wrong_puk() {
 // BSP_LCD_DisplayStringAtLine(6, (uint8_t*) "Wrong PUK");
}

uint8_t ui_prompt_try_puk() {
  return 1;
}

uint8_t ui_read_pin(uint8_t* out, int8_t retries) {
  out[0] = '1';
  out[1] = '2';
  out[2] = '3';
  out[3] = '4';
  out[4] = '5';
  out[5] = '6';

  return 1;
}

uint8_t ui_read_puk(uint8_t* out, int8_t retries) {
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

uint8_t ui_read_pairing(uint8_t* pairing, uint32_t *len) {
  return 0;
}

void ui_seed_loaded() {
  //BSP_LCD_DisplayStringAtLine(6, (uint8_t*) "Seed loaded");
}

uint8_t ui_backup_mnemonic(const char* mnemonic) {
 // BSP_LCD_DisplayStringAtLine(7, (uint8_t*) mnemonic);
  return 1;
}

uint16_t ui_read_mnemonic(uint16_t* indexes, uint32_t* len) {
  *len = 12;
  return ERR_DATA;
}

uint8_t ui_confirm_eth_address(const char* address) {
  return 1;
}
