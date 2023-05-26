#ifndef __COMMAND_H
#define __COMMAND_H

#include <stdint.h>
#include "iso7816/smartcard.h"
#include "error.h"

#define INS_GET_ETH_ADDR 0x02
#define INS_SIGN_ETH_TX 0x04
#define INS_GET_APP_CONF 0x06
#define INS_SIGN_ETH_MSG 0x08
#define INS_SIGN_ETH_MSG 0x08
#define INS_PROVIDE_ERC20 0x0A
#define INS_SIGN_EIP_712 0x0C
#define INS_SIGN_ETH_MSG 0x08
#define INS_PROVIDE_NFT 0x14

typedef enum {
  COMMAND_IDLE = 0x00,
  COMMAND_INBOUND = 0x01,
  COMMAND_COMPLETE = 0x02,
  COMMAND_OUTBOUND = 0x03
} command_status_t;

typedef struct {
  command_status_t status;
  uint16_t to_rxtx;
  uint16_t segment_count;
  apdu_t apdu;
} command_t;

app_err_t command_init_recv(command_t* cmd, uint16_t len);
void command_init_send(command_t* cmd);
void command_receive(command_t* cmd, uint8_t* data, uint8_t len);
uint8_t command_send(command_t* cmd, uint8_t* buf, uint8_t len);
void command_send_ack(command_t* cmd, uint8_t len);

#endif
