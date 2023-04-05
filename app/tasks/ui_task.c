#include "FreeRTOS.h"
#include "task.h"
#include "app_tasks.h"
#include "log/log.h"
#include "qrcode/qrscan.h"
#include "ui/menu.h"
#include "ui/ui_internal.h"

struct ui_cmd g_ui_cmd;

void ui_task_entry(void* pvParameters) {
  LOG_MSG("Starting UI task");

  if (screen_init() != HAL_OK) {
    LOG_MSG("Failed to init screen");
  }

  while(1) {
    if (!ulTaskNotifyTakeIndexed(UI_NOTIFICATION_IDX, pdTRUE, portMAX_DELAY)) {
      continue;
    }

    switch(g_ui_cmd.type) {
    case UI_CMD_MENU:
      menu_run(g_ui_cmd.params.menu.menu);
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
