#ifndef _ATR_TYPES_H
#define _ATR_TYPES_H

#include "main.h"

#define ATR_HIST_BYTE_MAX 15
#define ATR_INVALID_TA2 0xff

#define ATR_CLASS_A_5V 0x1
#define ATR_CLASS_B_3V 0x2
#define ATR_CLASS_C_1V8 0x4

#define ATR_PROTOCOLS_T0 0x1
#define ATR_PROTOCOLS_T1 0x2

typedef enum {
    ATR_T1_LRC,
    ATR_T1_CRC
} ATR_T1_RC;

typedef enum {
    ATR_CS_NOT_SUPPORTED = 0,
    ATR_CS_L = 0x40,
    ATR_CS_H = 0x80,
    ATR_CS_BOTH = 0xC0
} ATR_ClockStop;

typedef struct {
    uint8_t valid;
    uint8_t fi_di;
    uint8_t n;
    uint8_t t15_present;
    uint8_t ta2;
    uint8_t t0_wi;
    uint8_t t1_ifsc;
    uint8_t t1_bwi;
    uint8_t t1_cwi;
    uint8_t t1_rc;
    uint8_t hist_len;
    uint8_t classes;
    uint8_t clockstop;
    uint8_t protocols;
    uint8_t default_protocol;
    uint8_t hist[ATR_HIST_BYTE_MAX];
} ATR;

#endif