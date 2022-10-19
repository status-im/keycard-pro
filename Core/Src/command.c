#include <string.h>

#include "command.h"

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

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
  cmd->to_recv = len;

  return 1;
}

void Command_Receive(Command* cmd, uint8_t* data, uint8_t len) {
  len = MIN(len, cmd->to_recv);

  memcpy(&cmd->apdu.data[cmd->apdu.lr], data, len);
  cmd->apdu.lr += len;
  cmd->to_recv -= len;

  if (cmd->to_recv == 0) {
    cmd->status = COMMAND_COMPLETE;
  }
}
