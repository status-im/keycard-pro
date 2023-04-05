#include "FreeRTOS.h"
#include "task.h"
#include "log/log.h"
#include "ui/ui.h"
#include "ur/eip4527_types.h"
#include "log/log.h"
#include "app_tasks.h"

static struct eth_sign_request sign_request;

void core_task_entry(void* pvParameters) {
  while(1) {
    //ui_qrscan(&sign_request);
    ui_menu(&menu_mainmenu);

    uint32_t events;

    BaseType_t res = pdFAIL;

    do {
      res = xTaskNotifyWait(pdFALSE, UINT32_MAX, &events, portMAX_DELAY);
    } while(res != pdPASS);

    if (events & CORE_USB_EVT) {
      //TODO: handle
    }

    if (events & CORE_UI_EVT) {
      LOG(LOG_CBOR, sign_request._eth_sign_request_sign_data.value, sign_request._eth_sign_request_sign_data.len);
    }
  }
}
