#ifndef __KEYCARD_H
#define __KEYCARD_H

#include "main.h"
#include "secure_channel.h"
#include "iso7816/smartcard.h"

typedef struct {
  SmartCard sc;
  SecureChannel ch;
  APDU apdu;
} Keycard;

void Keycard_Init(Keycard* kc, SMARTCARD_HandleTypeDef* dev, TIM_HandleTypeDef* usec_timer);
void Keycard_Run(Keycard* kc);
void Keycard_In(Keycard* kc);
void Keycard_Out(Keycard* kc);

#endif