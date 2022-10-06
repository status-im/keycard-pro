#ifndef __UI_H
#define __UI_H

#include "main.h"

void UI_Init();
void UI_Card_Inserted();
void UI_Card_Removed();
void UI_Card_Transport_Error();
void UI_Card_Accepted();
void UI_Clear();
void UI_Fatal();
void UI_Keycard_Wrong_Card();
void UI_Keycard_Not_Initalized();
void UI_Keycard_Init_Failed();
void UI_Keycard_No_Keys();
void UI_Keycard_Ready();
void UI_Keycard_Paired();
void UI_Keycard_Already_Paired();
void UI_Keycard_Wrong_Pairing();
void UI_Keycard_Flash_Error();
void UI_Keycard_Secure_Channel_Failed();
void UI_Keycard_Secure_Channel_OK();
void UI_Keycard_Wrong_PIN();
void UI_Keycard_PIN_OK();
void UI_Keycard_PUK_OK();
void UI_Keycard_Wrong_PUK();

uint8_t UI_Prompt_Try_PUK();
uint8_t UI_Read_PIN(uint8_t* out, int8_t retries);
uint8_t UI_Read_PUK(uint8_t* out, int8_t retries);
uint8_t UI_Read_Pairing(uint8_t* pairing, uint32_t *len);

#endif