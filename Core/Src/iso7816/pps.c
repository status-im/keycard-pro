#include "iso7816/pps.h"
#include "iso7816/atr.h"

#define PPS_TIMEOUT 20

const static uint32_t F_Table[] = {372, 372, 558, 744, 1116, 1488, 1860, 0, 0, 512, 768, 1024, 1536, 2048, 0, 0};
const static uint32_t D_Table[] = {0, 1, 2, 4, 8, 16, 32, 0, 12, 20, 0, 0, 0, 0, 0, 0};

#if(USART_CLOCK == 64000000)
const static uint32_t F_freq_Table[] = {4000000, 4533333, 5333333, 8000000, 10666666, 16000000, 16000000, 0, 0, 4533333, 6400000, 8000000, 10666666, 16000000, 0, 0};
#endif

uint8_t PPS_Negotiate(SmartCard* sc) {
  if (sc->atr.fi_di != ATR_DEFAULT_FIDI) {
    uint8_t pps[4] = {0xff, 0x10, 0x00, 0x00};
    uint8_t ppsr[4];
    pps[1] |= sc->atr.default_protocol;
    pps[2] = sc->atr.fi_di;
    pps[3] = pps[0] ^ pps[1] ^ pps[2];

    if (HAL_SMARTCARD_Transmit(sc->dev, pps, 4, SC_TRANSMIT_TO) != HAL_OK) {
      return 0;
    }

    if (!SmartCard_Receive_Sync(sc, ppsr, 4)) {
      return 0;
    }

    if(!((pps[0] == ppsr[0]) && (pps[1] == ppsr[1]) && (pps[2] == ppsr[2]) && (pps[3] == ppsr[3]))) {
        return 0;
    }
  }

  uint8_t fi = (sc->atr.fi_di >> 4);
  uint8_t di = (sc->atr.fi_di & 0x0f);
  
  uint32_t F = F_Table[fi];
  uint32_t D = D_Table[di];
  uint32_t freq = F_freq_Table[fi];

  sc->dev->Init.BaudRate = (freq * D) / F;
  sc->dev->Init.Prescaler = (USART_CLOCK / freq / 2);

  if (sc->atr.default_protocol == SC_T1) {
    sc->dev->Init.NACKEnable = SMARTCARD_NACK_DISABLE;
  }

  sc->dev->Init.TimeOutValue = 9600;
  sc->dev->Init.GuardTime = (sc->atr.n == 0 || sc->atr.n == 255) ? 0 : ((F/D) * sc->atr.n);

  if (HAL_SMARTCARD_Init(sc->dev) != HAL_OK) {
      return 0;
  }

  return 1;
}