#include "FreeRTOS.h"
#include "task.h"

#include "app_tasks.h"
#include "log/log.h"
#include "keypad/keypad.h"
#include "qrcode/qrscan.h"
#include "screen/screen.h"
#include "ui/dialog.h"
#include "ui/menu.h"
#include "ui/ui_internal.h"

struct ui_cmd g_ui_cmd;
keypad_key_t g_last_key;

void ui_task_entry(void* pvParameters) {
  LOG_MSG("Starting UI task");

  if (screen_init() != HAL_OK) {
    LOG_MSG("Failed to init screen");
  }

  g_ui_cmd.received = 0;

  while(1) {
    if (!g_ui_cmd.received && ((ui_wait_event(portMAX_DELAY) & UI_CMD_EVT) == 0)) {
      continue;
    }

    g_ui_cmd.received = 0;

    switch(g_ui_cmd.type) {
    case UI_CMD_MENU:
      g_ui_cmd.result = menu_run();
      break;
    case UI_CMD_QRSCAN:
      g_ui_cmd.result = qrscan_scan();
      break;
    case UI_CMD_DISPLAY_TXN:
      g_ui_cmd.result = display_tx();
      break;
    default:
      break;
    }

    xTaskNotifyIndexed(APP_TASK(core), CORE_EVENT_IDX, CORE_UI_EVT, eSetBits);
  }
}
