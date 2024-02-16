#ifndef TEST_APP
#include "FreeRTOS.h"
#include "task.h"

#include "app_tasks.h"
#include "core/settings.h"
#include "pwr.h"
#include "qrcode/qrout.h"
#include "qrcode/qrscan.h"
#include "screen/screen.h"
#include "ui/dialog.h"
#include "ui/input.h"
#include "ui/menu.h"
#include "ui/settings_ui.h"
#include "ui/ui_internal.h"

struct ui_cmd g_ui_cmd;
struct ui_ctx g_ui_ctx;

void ui_task_entry(void* pvParameters) {
  if (screen_init() != HAL_SUCCESS) {
    vTaskSuspend(NULL);
  }

  hal_pwm_set_dutycycle(PWM_BACKLIGHT, g_settings.lcd_brightness);
  screen_fill_area(&screen_fullarea, SCREEN_COLOR_BLACK);

  g_ui_cmd.received = 0;

  hal_inactivity_timer_set(g_settings.shutdown_timeout);

  while(1) {
    g_ui_ctx.battery = pwr_battery_level();

    if (!g_ui_cmd.received && ((ui_wait_event(portMAX_DELAY) & UI_CMD_EVT) == 0)) {
      continue;
    }

    hal_inactivity_timer_reset();

    g_ui_cmd.received = 0;

    switch(g_ui_cmd.type) {
    case UI_CMD_INFO:
      g_ui_cmd.result = dialog_info();
      break;
    case UI_CMD_MENU:
      g_ui_cmd.result = menu_run();
      break;
    case UI_CMD_DISPLAY_TXN:
      g_ui_cmd.result = dialog_confirm_tx();
      break;
    case UI_CMD_DISPLAY_MSG:
      g_ui_cmd.result = dialog_confirm_msg();
      break;
    case UI_CMD_DISPLAY_QR:
      g_ui_cmd.result = qrout_run();
      break;
    case UI_CMD_QRSCAN:
      g_ui_cmd.result = qrscan_scan();
      break;
    case UI_CMD_INPUT_PIN:
      g_ui_cmd.result = input_pin();
      break;
    case UI_CMD_INPUT_PUK:
      g_ui_cmd.result = input_puk();
      break;
    case UI_CMD_INPUT_STRING:
      g_ui_cmd.result = input_string();
      break;
    case UI_CMD_INPUT_MNEMO:
      g_ui_cmd.result = input_mnemonic();
      break;
    case UI_CMD_BACKUP_MNEMO:
      g_ui_cmd.result = input_backup_mnemonic();
      break;
    case UI_CMD_DEV_AUTH:
      g_ui_cmd.result = dialog_dev_auth();
      break;
    case UI_CMD_LCD_BRIGHTNESS:
      g_ui_cmd.result = settings_ui_lcd_brightness();
      break;
    case UI_CMD_PROGRESS:
      g_ui_cmd.result = settings_ui_update_progress();
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
