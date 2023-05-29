#ifndef __KEYCARD_H
#define __KEYCARD_H

#include "secure_channel.h"
#include "command.h"
#include "iso7816/smartcard.h"

#define KEYCARD_PIN_LEN 6
#define KEYCARD_PUK_LEN 12

typedef struct {
  smartcard_t sc;
  secure_channel_t ch;
  apdu_t apdu;
} keycard_t;

void keycard_init(keycard_t* kc);
void keycard_activate(keycard_t* kc);
void keycard_in(keycard_t* kc);
void keycard_out(keycard_t* kc);

app_err_t keycard_read_signature(uint8_t* data, uint8_t* digest, uint8_t* out_sig);

#endif
