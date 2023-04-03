#include "FreeRTOS.h"
#include "task.h"
#include "log/log.h"
#include "qrcode/qrscan.h"
#include "screen/screen.h"
#include "ui/ui_internal.h"
#include "app_tasks.h"

struct ui_cmd g_ui_cmd;

void ui_task_entry(void* pvParameters) {
  LOG_MSG("Starting UI task");

  if (screen_init() != HAL_OK) {
    LOG_MSG("Failed to init screen");
  }

  screen_fill_area(&screen_fullarea, SCREEN_COLOR_BLACK);

  while(1) {
    if (!ulTaskNotifyTakeIndexed(UI_NOTIFICATION_IDX, pdTRUE, portMAX_DELAY)) {
      continue;
    }

    switch(g_ui_cmd.type) {
    case UI_CMD_MENU:
      break;
    case UI_CMD_QRSCAN:
      qrscan_scan();
      break;
    default:
      break;
    }

    xTaskNotifyIndexed(APP_TASK(core), CORE_EVENT_IDX, CORE_UI_EVT, eSetBits);
  }
}
