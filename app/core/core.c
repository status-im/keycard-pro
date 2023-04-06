#include "app_tasks.h"
#include "core.h"
#include "log/log.h"
#include "ui/ui_internal.h"

core_ctx_t g_core;

void core_usb_cancel() {
  //TODO: implement
}

void core_usb_run() {

}

void core_qr_run() {
  ui_qrscan(&g_core.qr_request);

  if (core_wait_event(0) != CORE_EVT_UI_OK) {
    return;
  }

  LOG(LOG_CBOR, g_core.qr_request._eth_sign_request_sign_data.value, g_core.qr_request._eth_sign_request_sign_data.len);
}

void core_action_run(i18n_str_id_t menu) {
  switch(menu) {
  case MENU_QRCODE:
    core_qr_run();
    break;
  default:
    //unhandled commands
    break;
  }
}

core_evt_t core_wait_event(uint8_t accept_usb) {
  uint32_t events;

  while(1) {
    BaseType_t res = pdFAIL;
    res = xTaskNotifyWaitIndexed(CORE_EVENT_IDX, 0, UINT32_MAX, &events, portMAX_DELAY);

    if (res != pdPASS) {
      continue;
    }

    if (events & CORE_USB_EVT) {
      if (accept_usb) {
        return CORE_EVT_USB_CMD;
      } else {
        core_usb_cancel();
      }
    }

    if (events & CORE_UI_EVT) {
      return g_ui_cmd.result == ERR_OK ? CORE_EVT_UI_OK : CORE_EVT_UI_CANCELLED;
    }
  }
}

