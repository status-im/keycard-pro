#ifndef __SMARTCARD_H
#define __SMARTCARD_H

#include "atr_types.h"

#define SW_OK 0x9000

#define APDU_CLA(__APDU__) ((__APDU__)->data[0])
#define APDU_INS(__APDU__) ((__APDU__)->data[1])
#define APDU_P1(__APDU__) ((__APDU__)->data[2])
#define APDU_P2(__APDU__) ((__APDU__)->data[3])
#define APDU_LC(__APDU__) ((__APDU__)->has_lc ? (__APDU__)->data[4] : -1)
#define APDU_DATA(__APDU__)(&((__APDU__)->data[5]))
#define APDU_RESP(__APDU__)(&((__APDU__)->data[1]))
#define APDU_LE(__APDU__) ((__APDU__)->has_le ? (__APDU__)->data[(__APDU__)->has_lc ? 5 : 4] : -1)
#define APDU_SET_LC(__APDU__, __val__) {(__APDU__)->data[4] = __val__; (__APDU__)->has_lc = 1;}
#define APDU_SET_LE(__APDU__, __val__) {(__APDU__)->data[5 + APDU_LC(__APDU__)] = __val__; (__APDU__)->has_le = 1;}
#define APDU_RESET(__APDU__) {(__APDU__)->has_lc = 0; (__APDU__)->has_le = 0; (__APDU__)->lr = 0;}
#define APDU_LEN(__APDU__) (4 + (__APDU__)->has_le + ((__APDU__)->has_lc ? (1 + (__APDU__)->data[4]) : 0))
#define APDU_SW1(__APDU__) ((__APDU__)->data[(__APDU__)->lr - 1])
#define APDU_SW2(__APDU__) ((__APDU__)->data[(__APDU__)->lr])
#define APDU_SW(__APDU__) ((APDU_SW1(__APDU__) << 8) | APDU_SW2(__APDU__))
#define APDU_BUF_LEN 255 + 6

#define APDU_ASSERT_SW(__APDU__, __SW__) if (APDU_SW(__APDU__) != __SW__) return APDU_SW(__APDU__);
#define APDU_ASSERT_OK(__APDU__) APDU_ASSERT_SW(__APDU__, SW_OK)

typedef enum {
  SC_NOT_PRESENT,
  SC_OFF,
  SC_DEACTIVATED,
  SC_READY
} SmartCardState;

typedef enum {
  SC_T0,
  SC_T1
} SmartCardProtocol;

typedef struct {
  SmartCardState state;
  ATR atr;
  uint8_t send_seq;
  uint8_t recv_seq;
  uint32_t t1_bwt;
  uint32_t t1_cwt;
  uint8_t t1_bwt_factor;
  uint32_t etu_10ns;
} SmartCard;

typedef struct __attribute__((packed,aligned(4))) {
  uint8_t has_lc;
  uint8_t has_le;
  uint8_t lr;
  uint8_t data[APDU_BUF_LEN];
} APDU;

#define SC_TRANSMIT_TO 25

void SmartCard_Delay(SmartCard* sc, uint32_t etu);
void SmartCard_Init(SmartCard* sc);
void SmartCard_Activate(SmartCard* sc);
void SmartCard_Deactivate(SmartCard* sc);
void SmartCard_In(SmartCard* sc);
void SmartCard_Out(SmartCard* sc);
uint8_t SmartCard_Transmit(SmartCard* sc, uint8_t* buf, uint32_t len);
uint8_t SmartCard_Transmit_Sync(SmartCard* sc, uint8_t* buf, uint32_t len);
uint8_t SmartCard_Receive(SmartCard* sc, uint8_t* buf, uint32_t len);
uint8_t SmartCard_Receive_Sync(SmartCard* sc, uint8_t* buf, uint32_t len);
uint8_t SmartCard_Send_APDU(SmartCard* sc, APDU* apdu);

#endif
