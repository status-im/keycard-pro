#include "iso7816/t1.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define T1_FAIL 0
#define T1_RETRY 1
#define T1_OK 2

static void T1_Wait_BGT() {
  // TODO: implement properly
  HAL_Delay(2);
}

static inline uint8_t T1_LRC(uint8_t* header, uint8_t* data, uint32_t len) {
  uint8_t lrc = header[0] ^ header[1] ^ header[2];

  for (int i = 0; i < len; i++) {
    lrc ^= data[i];
  }

  return lrc;
}

static uint8_t T1_Transmit_R(SmartCard* sc, uint8_t err) {
  uint8_t block[4];
  block[0] = T1_NAD;
  block[1] = T1_R_BLOCK | err | T1_R_SEQ(sc->recv_seq);
  block[2] = 0;
  block[3] = T1_LRC(block, NULL, 0);
  
  return SmartCard_Transmit_Sync(sc, block, 4);
}

static uint8_t T1_Transmit_S(SmartCard* sc, uint8_t type, uint8_t has_data, uint8_t data) {
  uint8_t block[5];
  block[0] = T1_NAD;
  block[1] = T1_S_BLOCK | type;
  block[2] = has_data;
  block[3] = data;
  block[3+has_data] = T1_LRC(block, &block[3], has_data);
  
  return SmartCard_Transmit_Sync(sc, block, 4+has_data);
}

static uint8_t T1_Handle_I(SmartCard* sc, uint8_t pcb, uint8_t* more) {
  uint8_t errcode;

  if ((pcb & T1_I_SEQ_MSK) != T1_I_SEQ(sc->recv_seq)) {
    errcode = T1_R_OTHER;
  } else {
    errcode = T1_R_OK;
    sc->recv_seq = sc->recv_seq ^ 1;  
  }

  *more = ((pcb & T1_I_MORE) != 0) || (errcode != T1_R_OK);

  if (*more) {
    if (!T1_Transmit_R(sc, errcode)) {
      return T1_FAIL;
    }
  }

  return T1_OK;
}

static uint8_t T1_Handle_S(SmartCard* sc, uint8_t pcb, uint8_t data, uint8_t* more) {
  uint8_t is_resp = (pcb & T1_S_RES) == T1_S_RES;

  uint8_t res = T1_S_RES;
  uint8_t len;
  uint8_t err = T1_OK;
  *more = !is_resp;

  if ((pcb & T1_S_WTX) == T1_S_WTX) {
    sc->t1_bwt_factor = data;
    len = 1;
    res |= T1_S_WTX;
  } else if ((pcb & T1_S_ABORT) == T1_S_ABORT) {
    len = 0;
    res |= T1_S_ABORT;
    err = T1_FAIL;
    *more = 0;
  } else if ((pcb & T1_S_IFS) == T1_S_IFS) {
    if (!is_resp) {
      sc->atr.t1_ifsc = data;
    }
    len = 1;
    res |= T1_S_IFS;
  } else {
    len = 0;
    res |= T1_S_RESYNCH;
    sc->recv_seq = 0;
    sc->send_seq = 0;
  }

  if (!is_resp) {
    if (!T1_Transmit_S(sc, res, len, data)) {
      *more = 0;
      return T1_FAIL;
    }
  }
 
  return err;
}

static uint8_t T1_Handle_R(SmartCard* sc, uint8_t pcb) {
  if (((pcb & T1_R_ERR_MSK) != T1_R_OK) || ((pcb & T1_R_SEQ_MSK) != (sc->send_seq ^ 1))) {
    return T1_RETRY;
  }

  return T1_OK; 
}

uint8_t T1_Handle_Resp(SmartCard* sc, APDU* apdu) {
  uint8_t header[3];

  HAL_SMARTCARDEx_TimeOut_Config(sc->dev, (sc->t1_bwt * sc->t1_bwt_factor));
  sc->t1_bwt_factor = 1;
  if (!SmartCard_Receive_Sync(sc, header, 3)) {
    return T1_FAIL;
  }

  uint8_t blen = header[2];

  HAL_SMARTCARDEx_BlockLength_Config(sc->dev, blen);
  HAL_SMARTCARDEx_TimeOut_Config(sc->dev, sc->t1_cwt);

  uint8_t *data;
  uint8_t s;

  if ((header[1] & T1_R_BLOCK) == 0) {
    data = &apdu->data[apdu->lr];
    apdu->lr += blen;
  } else if ((header[1] & T1_S_BLOCK) == T1_S_BLOCK) {
    data = &s;
  } else {
    data = NULL;
    blen = 0;
  }

  if (blen) {
    if (!SmartCard_Receive_Sync(sc, data, blen)) {
      return T1_FAIL;
    }    
  }

  uint8_t lrc;
  if (!SmartCard_Receive_Sync(sc, &lrc, 1)) {
    return T1_FAIL;
  }

  T1_Wait_BGT();

  if (T1_LRC(header, data, blen) != lrc) {
    if (!T1_Transmit_R(sc, T1_R_PARITY)) {
      return T1_FAIL;
    }

    return T1_Handle_Resp(sc, apdu);
  }

  uint8_t more = 0;
  uint8_t err;

  if ((header[1] & T1_R_BLOCK) == 0) {
    err = T1_Handle_I(sc, header[1], &more);
  } else if ((header[1] & T1_S_BLOCK) == T1_S_BLOCK) {
    err = T1_Handle_S(sc, header[1], s, &more);    
    BSP_LED_On(LED4);
  } else {
    err = T1_Handle_R(sc, header[1]);
    if (header[1] == 0x81) {
      BSP_LED_Off(LED2);
    }
  }

  return more ? T1_Handle_Resp(sc, apdu) : err;
}

uint8_t T1_Transmit(SmartCard* sc, APDU* apdu) {
  uint8_t* data = apdu->data;
  uint8_t to_send = APDU_LEN(apdu);
  uint8_t resend = 0;

  //TODO: implement resync
  while(to_send > 0 && (resend < 2)) {
    uint8_t header[3];
    uint8_t blen = MIN(to_send, sc->atr.t1_ifsc);

    header[0] = T1_NAD;
    header[1] = T1_I_BLOCK | T1_I_SEQ(sc->send_seq) | ((to_send > sc->atr.t1_ifsc) ? T1_I_MORE : T1_I_LAST);
    header[2] = blen;

    uint8_t lrc = T1_LRC(header, data, blen);

    if (!SmartCard_Transmit_Sync(sc, header, 3)) {
      return 0;
    }
    
    if (!SmartCard_Transmit_Sync(sc, data, blen)) {
      return 0;
    }

    if (!SmartCard_Transmit_Sync(sc, &lrc, 1)) {
      return 0;
    }

    uint8_t res = T1_Handle_Resp(sc, apdu);

    switch(res) {
      case T1_FAIL:
        return 0;
      case T1_RETRY:
        resend++;
        break;
      case T1_OK:
        sc->send_seq = sc->send_seq ^ 1;
        to_send -= blen;
        data += blen;
        resend = 0;
        break;
    }
  }

  return to_send == 0;
}

uint8_t T1_Negotiate_IFSD(SmartCard* sc) {
  if (!T1_Transmit_S(sc, T1_S_IFS, 1, T1_IFSD)) {
    return 0;
  }

  if (T1_Handle_Resp(sc, NULL) == T1_OK) {
    return 1;
  }

  if (!T1_Transmit_S(sc, T1_S_RESYNCH, 0, 0)) {
    return 0;
  }

  if (T1_Handle_Resp(sc, NULL) != T1_OK) {
    return 0;
  }

  BSP_LED_Toggle(LED1);
  HAL_Delay(50);
  return T1_Negotiate_IFSD(sc);
}