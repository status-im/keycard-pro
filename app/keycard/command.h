#ifndef __COMMAND_H
#define __COMMAND_H

#include <stdint.h>
#include "iso7816/smartcard.h"

typedef enum {
  COMMAND_IDLE = 0x00,
  COMMAND_INBOUND = 0x01,
  COMMAND_COMPLETE = 0x02,
  COMMAND_OUTBOUND = 0x03
} CommandStatus;

typedef struct {
  CommandStatus status;
  uint16_t to_rxtx;
  uint16_t segment_count;
  APDU apdu;
} Command;

app_err_t Command_Init_Recv(Command* cmd, uint16_t len);
void Command_Init_Send(Command* cmd);
void Command_Receive(Command* cmd, uint8_t* data, uint8_t len);
uint8_t Command_Send(Command* cmd, uint8_t* buf, uint8_t len);
void Command_Send_ACK(Command* cmd, uint8_t len);

#endif
