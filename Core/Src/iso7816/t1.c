#include "iso7816/t1.h"

uint8_t T1_Negotiate_Params(SmartCard* sc) {
  uint8_t sblock[] = {0x00, 0xc1, 0x01, 0xfe, (0x00 ^ 0xc0 ^ 0x01 ^ 0xfe)};
  uint8_t sblock_resp[5];
    
  if (HAL_SMARTCARD_Transmit(sc->dev, sblock, 5, SC_TRANSMIT_TO) != HAL_OK) {
    return 0;
  }

  if (!SmartCard_Receive_Sync(sc, sblock_resp, 5)) {
    return 0;
  }

  BSP_LED_Off(LED1);

  //this will be removed soon, it is just here to place a breakpoint
  for (int i = 0; i < 4; i++) {
    sblock_resp[i] += 1;
  }

  return 1;
}