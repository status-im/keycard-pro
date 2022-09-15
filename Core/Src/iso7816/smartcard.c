#include "iso7816/smartcard.h"
#include "iso7816/atr.h"
#include "iso7816/pps.h"

void SmartCard_Init(SmartCard* sc, SMARTCARD_HandleTypeDef* dev) {
  sc->dev = dev;
  if (HAL_GPIO_ReadPin(SC_NOFF_GPIO_Port, SC_NOFF_Pin)) {
    sc->state = SC_OFF;
  } else {
    sc->state = SC_NOT_PRESENT;
  }
}

void SmartCard_Activate(SmartCard* sc) {
  HAL_GPIO_WritePin(SC_NCMDVCC_GPIO_Port, SC_NCMDVCC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SC_RST_GPIO_Port, SC_RST_Pin, GPIO_PIN_RESET);
  
  if (!ATR_Read(sc)) {
    sc->state = SC_OFF;
    return;
  }

  BSP_LED_On(LED1);

  if (!PPS_Negotiate(sc)) {
    sc->state = SC_OFF;
    return;
  }

  HAL_Delay(400);
  BSP_LED_On(LED2);

  if (sc->atr.default_protocol == SC_T1) {
    if (!T1_Negotiate_Params(sc)) {
      sc->state = SC_OFF;
      return;
    }
  } else {
    //T0 not yet supported!
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
  sc->dev->Init.TimeOutEnable = SMARTCARD_TIMEOUT_DISABLE;

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

void HAL_SMARTCARD_ErrorCallback(SMARTCARD_HandleTypeDef *hsc) {
  if(HAL_SMARTCARD_GetError(hsc) & HAL_SMARTCARD_ERROR_FE) {
    __HAL_SMARTCARD_FLUSH_DRREGISTER(hsc);
  }

  if(HAL_SMARTCARD_GetError(hsc) & HAL_SMARTCARD_ERROR_PE) {
    __HAL_SMARTCARD_ENABLE_IT(hsc, SMARTCARD_IT_RXNE);
    __HAL_SMARTCARD_FLUSH_DRREGISTER(hsc);
  }

  if(HAL_SMARTCARD_GetError(hsc) & HAL_SMARTCARD_ERROR_NE) {
    __HAL_SMARTCARD_FLUSH_DRREGISTER(hsc);
  }

  if(HAL_SMARTCARD_GetError(hsc) & HAL_SMARTCARD_ERROR_ORE) {
    __HAL_SMARTCARD_FLUSH_DRREGISTER(hsc);
  }
}

void HAL_SMARTCARD_TxCpltCallback(SMARTCARD_HandleTypeDef *hsc) {

}

void HAL_SMARTCARD_RxCpltCallback(SMARTCARD_HandleTypeDef *hsc) {

}