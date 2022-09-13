#ifndef __KEYCARD_H
#define __KEYCARD_H

#include "main.h"
#include "iso7816/smartcard.h"

void Keycard_Init();
void Keycard_Run(SmartCard* sc);

#endif