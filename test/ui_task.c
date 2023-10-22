#ifdef TEST_APP
#include "FreeRTOS.h"
#include "task.h"

#include "app_tasks.h"
#include "qrcode/qrscan.h"
#include "screen/screen.h"
#include "ui/dialog.h"
#include "ui/settings_ui.h"
#include "ui/ui_internal.h"

struct ui_cmd g_ui_cmd;
struct ui_ctx g_ui_ctx;

void ui_task_entry(void* pvParameters) {
  if (screen_init() != HAL_SUCCESS) {
    vTaskSuspend(NULL);
  }

  hal_pwm_set_dutycycle(PWM_BACKLIGHT, 50);
  screen_fill_area(&screen_fullarea, SCREEN_COLOR_BLACK);

  g_ui_cmd.received = 0;

  while(1) {
    if (!g_ui_cmd.received && ((ui_wait_event(portMAX_DELAY) & UI_CMD_EVT) == 0)) {
      continue;
    }

    g_ui_cmd.received = 0;

    switch(g_ui_cmd.type) {
    case UI_CMD_INFO:
      g_ui_cmd.result = dialog_info();
      break;
    case UI_CMD_QRSCAN:
      g_ui_cmd.result = qrscan_scan();
      break;
    default:
      g_ui_cmd.result = ERR_CANCEL;
      break;
    }

    if (!g_ui_cmd.received) {
      xTaskNotifyIndexed(APP_TASK(core), CORE_EVENT_IDX, CORE_UI_EVT, eSetBits);
    }
  }
}
#endif
