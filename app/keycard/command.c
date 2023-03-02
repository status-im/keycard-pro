#include <string.h>

#include "command.h"
#include "common.h"

uint8_t Command_Lock(CommandChannel ch, Command* cmd) {
  if (cmd->channel == ch) {
    return 1;
  }

  uint32_t prim = __get_PRIMASK();

  __disable_irq();
  if (cmd->status == COMMAND_IDLE) {
    cmd->status = COMMAND_INBOUND;
    cmd->channel = ch;
  }

  if (!prim) {
    __enable_irq();
  }

  return cmd->channel == ch;
}

uint8_t Command_Init_Recv(Command* cmd, uint16_t len) {
  if (len >= APDU_BUF_LEN) {
    return 0;
  }

  cmd->apdu.lr = 0;
  cmd->to_rxtx = len;
  cmd->segment_count = 0;

  return 1;
}

void Command_Init_Send(Command* cmd) {
  cmd->to_rxtx = 0;
  cmd->segment_count = 0;
  cmd->status = COMMAND_OUTBOUND;
}

void Command_Receive(Command* cmd, uint8_t* data, uint8_t len) {
  len = OMV_MIN(len, cmd->to_rxtx);

  memcpy(&cmd->apdu.data[cmd->apdu.lr], data, len);
  cmd->apdu.lr += len;
  cmd->to_rxtx -= len;

  if (cmd->to_rxtx == 0) {
    cmd->status = COMMAND_COMPLETE;
  }
}

uint8_t Command_Send(Command* cmd, uint8_t* buf, uint8_t len) {
  len = OMV_MIN(len, cmd->apdu.lr);
  uint8_t* data = APDU_RESP(&cmd->apdu);
  memcpy(buf, &data[cmd->to_rxtx], len);
  return len;
}

void Command_Send_ACK(Command* cmd, uint8_t len) {
  cmd->apdu.lr -= len;
  cmd->to_rxtx += len;
  cmd->segment_count++;

  if (cmd->apdu.lr == 0) {
    cmd->status = COMMAND_IDLE;
  }  
}
