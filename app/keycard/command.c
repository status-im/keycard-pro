#include <string.h>

#include "command.h"
#include "common.h"

app_err_t command_init_recv(command_t* cmd, uint16_t len) {
  if (len >= APDU_BUF_LEN) {
    return ERR_DATA;
  }

  cmd->apdu.lr = 0;
  cmd->to_rxtx = len;
  cmd->segment_count = 0;

  return ERR_OK;
}

void command_init_send(command_t* cmd) {
  cmd->to_rxtx = 0;
  cmd->segment_count = 0;
  cmd->status = COMMAND_OUTBOUND;
}

void command_receive(command_t* cmd, uint8_t* data, uint8_t len) {
  len = APP_MIN(len, cmd->to_rxtx);

  memcpy(&cmd->apdu.data[cmd->apdu.lr], data, len);
  cmd->apdu.lr += len;
  cmd->to_rxtx -= len;

  if (cmd->to_rxtx == 0) {
    cmd->status = COMMAND_COMPLETE;
  }
}

uint8_t command_send(command_t* cmd, uint8_t* buf, uint8_t len) {
  len = APP_MIN(len, cmd->apdu.lr);
  uint8_t* data = APDU_RESP(&cmd->apdu);
  memcpy(buf, &data[cmd->to_rxtx], len);
  return len;
}

void command_send_ack(command_t* cmd, uint8_t len) {
  cmd->apdu.lr -= len;
  cmd->to_rxtx += len;
  cmd->segment_count++;

  if (cmd->apdu.lr == 0) {
    cmd->status = COMMAND_IDLE;
  }  
}
