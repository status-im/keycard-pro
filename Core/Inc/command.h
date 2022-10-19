#ifndef __COMMAND_H
#define __COMMAND_H

#include <stdint.h>
#include "smartcard.h"

typedef enum {
  USBHID,
  NFC
} CommandChannel;

typedef enum {
  COMMAND_IDLE = 0x00,
  COMMAND_INBOUND = 0x01,
  COMMAND_COMPLETE = 0x02,
  COMMAND_OUTBOUND = 0x03
} CommandStatus;

typedef struct {
  CommandChannel channel;
  CommandStatus status;
  uint16_t to_recv;
  APDU apdu;
} Command;

uint8_t Command_Lock(CommandChannel ch, Command* cmd);
uint8_t Command_Init_Recv(Command* cmd, uint16_t len);
void Command_Receive(Command* cmd, uint8_t* data, uint8_t len);


#endif