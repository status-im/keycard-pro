#include "atr.h"

#define ATR_TIMEOUT 110
#define ATR_SIZE 2
#define ATR_DIRECT_CONVENTION 0x3b

#define ATR_READ_BYTE(v, mask, td, def) ((mask & td) ? v : def)

uint8_t ATR_Read_Bytes(SmartCard* sc, uint8_t* buf, uint8_t td, uint8_t *ck) {
  for (int i = 0; i < 4; i++) {
    if (td & 0x10) {
      if (!SmartCard_Receive_Sync(sc, &buf[i], 1)) {
        return 0;
      }

      *ck ^= buf[i];
    } else {
      buf[i] = 0;
    }

    td >>= 1;
  }

  return 1;
}

void ATR_Parse_Bytes(SmartCard* sc, uint8_t* buf, uint8_t td, uint8_t i) {
  if (i == 1) {
    sc->atr.fi_di = ATR_READ_BYTE(buf[0], 0x10, td, ATR_DEFAULT_FIDI);
    sc->atr.n = ATR_READ_BYTE(buf[2], 0x40, td, ATR_DEFAULT_N);
  } else if (i == 2) {
    sc->atr.default_protocol = td & 0xf;
    sc->atr.ta2 = ATR_READ_BYTE(buf[0], 0x10, td, ATR_INVALID_TA2);
    sc->atr.t0_wi = ATR_READ_BYTE(buf[2], 0x40, td, ATR_DEFAULT_T0_WI);
  } else {
    switch (td & 0xf) {
      case 1:
        sc->atr.protocols |= ATR_PROTOCOLS_T1;
        sc->atr.t1_ifsc = ATR_READ_BYTE(buf[0], 0x10, td, ATR_DEFAULT_T1_IFSC);
        uint8_t tbt1 = ATR_READ_BYTE(buf[1], 0x20, td, ((ATR_DEFAULT_T1_BWI << 4) | ATR_DEFAULT_T1_CWI));
        sc->atr.t1_cwi = tbt1 & 0xf;
        sc->atr.t1_bwi = (tbt1 & 0xf0) >> 4;
        sc->atr.t1_rc = ATR_READ_BYTE(buf[2], 0x40, td, ATR_DEFAULT_T1_RC);
        break;
      case 15:
        sc->atr.t15_present = 1;
        uint8_t ta15 = ATR_READ_BYTE(buf[0], 0x10, td, (ATR_DEFAULT_CLOCKSTOP | ATR_DEFAULT_CLASSES));
        sc->atr.classes = ta15 & 0x7;
        sc->atr.clockstop = ta15 & 0xc0;
        break;
    }
  }
}

void ATR_Reset(SmartCard* sc) {
  sc->atr.valid = 0;
  sc->atr.t15_present = 0;
  sc->atr.default_protocol = SC_T0;
  sc->atr.protocols = ATR_PROTOCOLS_T0;

  sc->atr.fi_di = ATR_DEFAULT_FIDI;
  sc->atr.n = ATR_DEFAULT_N;
  sc->atr.ta2 = ATR_INVALID_TA2;
  sc->atr.t0_wi = ATR_DEFAULT_T0_WI;
  sc->atr.t1_ifsc = ATR_DEFAULT_T1_IFSC;
  sc->atr.t1_cwi = ATR_DEFAULT_T1_CWI;
  sc->atr.t1_bwi = ATR_DEFAULT_T1_BWI;
  sc->atr.clockstop = ATR_DEFAULT_CLOCKSTOP;
  sc->atr.classes = ATR_DEFAULT_CLASSES;
  sc->atr.hist_len = 0;
}

uint8_t ATR_Read(SmartCard* sc) {
  ATR_Reset(sc);

  uint8_t buf[4];

  __HAL_SMARTCARD_FLUSH_DRREGISTER(sc->dev);
  HAL_SMARTCARDEx_TimeOut_Config(sc->dev, ATR_TIMEOUT);
  if (!SmartCard_Receive_Sync(sc, buf, 2)) {
    return 0;
  }
  
  HAL_SMARTCARDEx_TimeOut_Config(sc->dev, sc->dev->Init.TimeOutValue);

  if (buf[0] != ATR_DIRECT_CONVENTION) {
    return 0;
  }

  uint8_t td = buf[1];
  uint8_t ck = td;

  sc->atr.hist_len = td & 0x0f;

  int i = 1;

  while(td) {
    if (!ATR_Read_Bytes(sc, buf, td, &ck)) {
      return 0;
    }

    ATR_Parse_Bytes(sc, buf, td, i++);
    td = buf[3];
  }

  if (!SmartCard_Receive_Sync(sc, sc->atr.hist, sc->atr.hist_len)) {
    return 0;
  }
  
  for (int i = 0; i < sc->atr.hist_len; i++) {
    ck ^= sc->atr.hist[i];
  }

  if ((sc->atr.protocols & ATR_PROTOCOLS_T1) || sc->atr.t15_present) {
    if (!SmartCard_Receive_Sync(sc, buf, 1)) {
      return 0;
    }
    
    sc->atr.valid = (buf[0] ^ ck) == 0;
  } else {
    sc->atr.valid = 1;
  }

  return sc->atr.valid;
}