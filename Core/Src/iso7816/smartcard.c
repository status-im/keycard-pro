#include "iso7816/smartcard.h"
#include "iso7816/atr.h"
#include "iso7816/pps.h"
#include "iso7816/t1.h"

#define SC_RESET_DELAY 400
#define SC_DEFAULT_ETU10NS 9300

static inline void SmartCard_State_Reset(SmartCard* sc) {
  sc->send_seq = 0;
  sc->recv_seq = 0;
  sc->etu_10ns = SC_DEFAULT_ETU10NS;
}

void SmartCard_Delay(SmartCard* sc, uint32_t etu) {
  uint32_t usec = (sc->etu_10ns * etu)/100;

  if (usec & 0xffff0000) {
    HAL_Delay(usec/1000);
  } else {
    sc->usec_timer->Instance->CNT = 0;
    while (sc->usec_timer->Instance->CNT < usec);
  }
}

void SmartCard_Init(SmartCard* sc, SMARTCARD_HandleTypeDef* dev, TIM_HandleTypeDef* usec_timer) {
  SmartCard_State_Reset(sc);

  sc->dev = dev;
  sc->usec_timer = usec_timer;

  if (HAL_GPIO_ReadPin(SC_NOFF_GPIO_Port, SC_NOFF_Pin)) {
    sc->state = SC_OFF;
  } else {
    sc->state = SC_NOT_PRESENT;
  }
}

void SmartCard_Activate(SmartCard* sc) {
  HAL_GPIO_WritePin(SC_NCMDVCC_GPIO_Port, SC_NCMDVCC_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SC_RST_GPIO_Port, SC_RST_Pin, GPIO_PIN_RESET);

  SmartCard_Delay(sc, SC_RESET_DELAY);
  HAL_GPIO_WritePin(SC_NCMDVCC_GPIO_Port, SC_NCMDVCC_Pin, GPIO_PIN_RESET);
  SmartCard_Delay(sc, SC_RESET_DELAY);
  HAL_GPIO_WritePin(SC_RST_GPIO_Port, SC_RST_Pin, GPIO_PIN_SET);

  if (!ATR_Read(sc)) {
    SmartCard_Deactivate(sc);
    return;
  }

  if (!PPS_Negotiate(sc)) {
    SmartCard_Deactivate(sc);
    return;
  }

  if (sc->atr.default_protocol == SC_T1) {
    if (!T1_Negotiate_IFSD(sc, 3)) {
      SmartCard_Deactivate(sc);
      return;
    }
  }

  sc->state = SC_READY;
}

void SmartCard_Deactivate(SmartCard* sc) {
  HAL_GPIO_WritePin(SC_NCMDVCC_GPIO_Port, SC_NCMDVCC_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SC_RST_GPIO_Port, SC_RST_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SC_5V3V_GPIO_Port, SC_5V3V_Pin, GPIO_PIN_SET);
  
  sc->dev->Init.StopBits = SMARTCARD_STOPBITS_1_5;
  sc->dev->Init.BaudRate = SC_DEFAULT_BAUD_RATE;
  sc->dev->Init.Prescaler = SC_DEFAULT_PSC;
  sc->dev->Init.GuardTime = 0;
  sc->dev->Init.NACKEnable = SMARTCARD_NACK_ENABLE;
  sc->dev->Init.AutoRetryCount = 3;

  SmartCard_State_Reset(sc);

  HAL_SMARTCARD_Init(sc->dev);
  sc->state = SC_DEACTIVATED;
}

void SmartCard_In(SmartCard* sc) {
  sc->state = SC_OFF;
}

void SmartCard_Out(SmartCard* sc) {
  SmartCard_Deactivate(sc);
  sc->state = SC_NOT_PRESENT;
}

static inline uint8_t SmartCard_Wait(SmartCard* sc) {
  while(HAL_SMARTCARD_GetState(sc->dev) != HAL_SMARTCARD_STATE_READY) {}

  if (HAL_SMARTCARD_GetError(sc->dev) != HAL_SMARTCARD_ERROR_NONE) {
    return 0;
  }

  return 1;
}

uint8_t SmartCard_Transmit(SmartCard* sc, uint8_t* buf, uint32_t len) {
  return HAL_SMARTCARD_Transmit_IT(sc->dev, buf, len) == HAL_OK;
}

uint8_t SmartCard_Transmit_Sync(SmartCard* sc, uint8_t* buf, uint32_t len) {
  if (!SmartCard_Transmit(sc, buf, len)) {
    return 0;
  }

  return SmartCard_Wait(sc);
}

uint8_t SmartCard_Receive(SmartCard* sc, uint8_t* buf, uint32_t len) {
  __HAL_SMARTCARD_ENABLE_IT(sc->dev, SMARTCARD_IT_RTO);
  return HAL_SMARTCARD_Receive_IT(sc->dev, buf, len) == HAL_OK;
}

uint8_t SmartCard_Receive_Sync(SmartCard* sc, uint8_t* buf, uint32_t len) {
  if (!SmartCard_Receive(sc, buf, len)) {
    return 0;
  }

  return SmartCard_Wait(sc);
}

uint8_t SmartCard_Send_APDU(SmartCard* sc, APDU* apdu) {
  if (sc->atr.default_protocol == SC_T1) {
    return T1_Transmit(sc, apdu);
  } else {
    // T0 not implemented
    return 0;
  }
}

void HAL_SMARTCARD_ErrorCallback(SMARTCARD_HandleTypeDef *hsc) {
  uint32_t error = HAL_SMARTCARD_GetError(hsc);

  if(error & HAL_SMARTCARD_ERROR_FE) {
    __HAL_SMARTCARD_FLUSH_DRREGISTER(hsc);
  }

  if(error & HAL_SMARTCARD_ERROR_PE) {
    __HAL_SMARTCARD_ENABLE_IT(hsc, SMARTCARD_IT_RXNE);
    __HAL_SMARTCARD_FLUSH_DRREGISTER(hsc);
  }

  if(error & HAL_SMARTCARD_ERROR_NE) {
    __HAL_SMARTCARD_FLUSH_DRREGISTER(hsc);
  }

  if(error & HAL_SMARTCARD_ERROR_ORE) {
    __HAL_SMARTCARD_FLUSH_DRREGISTER(hsc);
  }

  if(error & HAL_SMARTCARD_ERROR_RTO) {
    __HAL_SMARTCARD_FLUSH_DRREGISTER(hsc);
    __HAL_SMARTCARD_DISABLE_IT(hsc, SMARTCARD_IT_RTO);
  }

}

void HAL_SMARTCARD_TxCpltCallback(SMARTCARD_HandleTypeDef *hsc) {

}

void HAL_SMARTCARD_RxCpltCallback(SMARTCARD_HandleTypeDef *hsc) {
  __HAL_SMARTCARD_DISABLE_IT(hsc, SMARTCARD_IT_RTO);
}