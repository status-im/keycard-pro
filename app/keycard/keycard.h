#ifndef __KEYCARD_H
#define __KEYCARD_H

#include "secure_channel.h"
#include "command.h"
#include "iso7816/smartcard.h"

#define KEYCARD_PIN_LEN 6
#define KEYCARD_PUK_LEN 12

typedef struct {
  SmartCard sc;
  SecureChannel ch;
  APDU apdu;
} Keycard;

void Keycard_Init(Keycard* kc);
void Keycard_Run(Keycard* kc, Command* cmd);
void Keycard_In(Keycard* kc);
void Keycard_Out(Keycard* kc);

app_err_t Keycard_ConvertSignature(uint8_t* data, uint8_t* digest, uint8_t* out_sig);

#endif
