#include "iso7816/smartcard.h"
#include "iso7816/atr.h"
#include "iso7816/pps.h"
#include "iso7816/t1.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hal.h"

#define SC_DEFAULT_ETU10NS 9300
#define SC_MAX_TIMEOUT_MS 30000

static inline void SmartCard_State_Reset(SmartCard* sc) {
  sc->send_seq = 0;
  sc->recv_seq = 0;
  sc->etu_10ns = SC_DEFAULT_ETU10NS;
}

void SmartCard_Delay(SmartCard* sc, uint32_t etu) {
  uint32_t usec = (sc->etu_10ns * etu)/100;

  if (usec > 4000) {
    vTaskDelay(pdMS_TO_TICKS(usec/1000));
  } else {
    hal_delay_us(usec);
  }
}


void SmartCard_Init(SmartCard* sc) {
  SmartCard_State_Reset(sc);
  sc->state = SC_OFF;
}

void SmartCard_Activate(SmartCard* sc) {
  hal_smartcard_start();

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
  SmartCard_State_Reset(sc);
  hal_smartcard_stop();
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
  BaseType_t res = pdFAIL;
  uint32_t err;
  res = xTaskNotifyWaitIndexed(SMARTCARD_TASK_NOTIFICATION_IDX, 0, UINT32_MAX, &err, pdMS_TO_TICKS(SC_MAX_TIMEOUT_MS));

  if (res != pdPASS) {
    //TODO: abort transfer
    return 0;
  }

  return err == HAL_SUCCESS;
}

uint8_t SmartCard_Transmit(SmartCard* sc, const uint8_t* buf, uint32_t len) {
  return hal_smartcard_send(buf, len) == HAL_SUCCESS;
}

uint8_t SmartCard_Transmit_Sync(SmartCard* sc, const uint8_t* buf, uint32_t len) {
  if (!SmartCard_Transmit(sc, buf, len)) {
    return 0;
  }

  return SmartCard_Wait(sc);
}

uint8_t SmartCard_Receive(SmartCard* sc, uint8_t* buf, uint32_t len) {
  return hal_smarcard_recv(buf, len) == HAL_SUCCESS;
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
