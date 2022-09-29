#ifndef __KEYCARD_H
#define __KEYCARD_H

#include "main.h"
#include "iso7816/smartcard.h"

#define KEYCARD_ERR_OK 0
#define KEYCARD_ERR_TXRX 1
#define KEYCARD_ERR_CRYPTO 2

void Keycard_Init();
void Keycard_Run(SmartCard* sc);

#endif