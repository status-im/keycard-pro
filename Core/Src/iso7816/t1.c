#include "iso7816/t1.h"

uint8_t T1_Negotiate_Params(SmartCard* sc) {
  uint8_t sblock[] = {0x00, 0xc1, 0x01, 0xfe, (0x00 ^ 0xc0 ^ 0x01 ^ 0xfe)};
  uint8_t sblock_resp[4];

  if (HAL_SMARTCARD_Transmit(sc->dev, sblock, 5, 20) != HAL_OK) {
    return 0;
  }

  __HAL_SMARTCARD_FLUSH_DRREGISTER(sc->dev);

  if (HAL_SMARTCARD_Receive_IT(sc->dev, sblock_resp, 4) != HAL_OK) {
    return 0;
  }

  while(HAL_SMARTCARD_GetState(sc->dev) != HAL_SMARTCARD_STATE_READY) {}

  for (int i = 0; i < 4; i++) {
    sblock_resp[i] += 1;
  }

  return 1;
}