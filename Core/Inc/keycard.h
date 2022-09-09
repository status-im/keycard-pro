#ifndef __KEYCARD_H
#define __KEYCARD_H

#include "main.h"
#include "smartcard.h"
#include "atr.h"
#include "t1_protocol_param.h"

int Keycard_Run(SMARTCARD_HandleTypeDef* SmartCardHandle);
void Keycard_Card_In();
void Keycard_Card_Out();

#endif