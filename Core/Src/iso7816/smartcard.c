#include "iso7816/smartcard.h"
#include "iso7816/atr.h"
#include "iso7816/pps.h"
#include "iso7816/t1.h"

void SmartCard_Init(SmartCard* sc, SMARTCARD_HandleTypeDef* dev) {
  sc->send_seq = 0;
  sc->recv_seq = 0;
  sc->dev = dev;

  if (HAL_GPIO_ReadPin(SC_NOFF_GPIO_Port, SC_NOFF_Pin)) {
    sc->state = SC_OFF;
  } else {
    sc->state = SC_NOT_PRESENT;
  }
}

void SmartCard_Activate(SmartCard* sc) {
  HAL_GPIO_WritePin(SC_NCMDVCC_GPIO_Port, SC_NCMDVCC_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SC_RST_GPIO_Port, SC_RST_Pin, GPIO_PIN_RESET);

  HAL_Delay(100);

  HAL_GPIO_WritePin(SC_NCMDVCC_GPIO_Port, SC_NCMDVCC_Pin, GPIO_PIN_RESET);

  HAL_Delay(100);

  HAL_GPIO_WritePin(SC_RST_GPIO_Port, SC_RST_Pin, GPIO_PIN_SET);

  if (!ATR_Read(sc)) {
    sc->state = SC_OFF;
    return;
  }

  BSP_LED_On(LED1);

  if (!PPS_Negotiate(sc)) {
    sc->state = SC_OFF;
    return;
  }

  BSP_LED_On(LED2);

  if (sc->atr.default_protocol == SC_T1) {
    // Test code to remove
    HAL_Delay(100);
    APDU _apdu;
    APDU* apdu = &_apdu;
    APDU_RESET(apdu);
    APDU_CLA(apdu) = 0;
    APDU_INS(apdu) = 0xa4;
    APDU_P1(apdu) = 4;
    APDU_P2(apdu) = 0;
    if (!T1_Transmit(sc, apdu)) {
      sc->state = SC_OFF;
      return;
    }
  } else {
    //T0 not implemented yet
    sc->state = SC_OFF;
    return;
  }

  BSP_LED_On(LED3);

  sc->state = SC_READY;
}

void SmartCard_Deactivate(SmartCard* sc) {
  BSP_LED_Off(LED1);
  BSP_LED_Off(LED2);
  BSP_LED_Off(LED3);
  BSP_LED_Off(LED4);

  HAL_GPIO_WritePin(SC_NCMDVCC_GPIO_Port, SC_NCMDVCC_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SC_RST_GPIO_Port, SC_RST_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SC_5V3V_GPIO_Port, SC_5V3V_Pin, GPIO_PIN_SET);
  
  sc->dev->Init.BaudRate = SC_DEFAULT_BAUD_RATE;
  sc->dev->Init.Prescaler = SC_DEFAULT_PSC;
  sc->dev->Init.GuardTime = 0;
  sc->dev->Init.NACKEnable = SMARTCARD_NACK_ENABLE;
  sc->send_seq = 0;
  sc->recv_seq = 0;

  HAL_SMARTCARD_DeInit(sc->dev);
  HAL_SMARTCARD_Init(sc->dev);
  sc->state = SC_OFF;
}

void SmartCard_In(SmartCard* sc) {
  sc->state = SC_OFF;
}

void SmartCard_Out(SmartCard* sc) {
  SmartCard_Deactivate(sc);
  sc->state = SC_NOT_PRESENT;
}

uint8_t SmartCard_Transmit_Sync(SmartCard* sc, uint8_t* buf, uint32_t len) {
  return HAL_SMARTCARD_Transmit(sc->dev, buf, len, SC_TRANSMIT_TO) == HAL_OK; 
}

uint8_t SmartCard_Receive(SmartCard* sc, uint8_t* buf, uint32_t len) {
  __HAL_SMARTCARD_ENABLE_IT(sc->dev, SMARTCARD_IT_RTO);

  if (HAL_SMARTCARD_Receive_IT(sc->dev, buf, len) != HAL_OK) {
    return 0;
  }

  return 1;
}

uint8_t SmartCard_Receive_Sync(SmartCard* sc, uint8_t* buf, uint32_t len) {
  if (!SmartCard_Receive(sc, buf, len)) {
    return 0;
  }

  while(HAL_SMARTCARD_GetState(sc->dev) != HAL_SMARTCARD_STATE_READY) {}

  if (HAL_SMARTCARD_GetError(sc->dev) != HAL_SMARTCARD_ERROR_NONE) {
    return 0;
  }

  return 1;
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
    BSP_LED_On(LED4);
    __HAL_SMARTCARD_FLUSH_DRREGISTER(hsc);
    __HAL_SMARTCARD_DISABLE_IT(hsc, SMARTCARD_IT_RTO);
  }

}

void HAL_SMARTCARD_TxCpltCallback(SMARTCARD_HandleTypeDef *hsc) {

}

void HAL_SMARTCARD_RxCpltCallback(SMARTCARD_HandleTypeDef *hsc) {
  __HAL_SMARTCARD_DISABLE_IT(hsc, SMARTCARD_IT_RTO);
}