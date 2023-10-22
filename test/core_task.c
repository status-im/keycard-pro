#ifdef TEST_APP
#include "FreeRTOS.h"
#include "task.h"

#include "common.h"
#include "core/core.h"
#include "core/settings.h"
#include "keycard/keycard.h"
#include "usb/usb.h"

static app_err_t core_keypad_test() {
  return ERR_CANCEL;
}

static app_err_t core_lcd_test() {
  return ERR_CANCEL;
}

static app_err_t core_camera_test() {
  return ERR_CANCEL;
}

static app_err_t core_card_test() {
  return ERR_CANCEL;
}

static app_err_t core_usb_test() {
  return ERR_CANCEL;
}

void core_task_entry(void* pvParameters) {
  g_settings.enable_usb = false;

  if (core_keypad_test() != ERR_OK) {
    vTaskSuspend(NULL);
  }

  if (core_lcd_test() != ERR_OK) {
    vTaskSuspend(NULL);
  }

  if (core_camera_test() != ERR_OK) {
    vTaskSuspend(NULL);
  }

  if (core_card_test() != ERR_OK) {
    vTaskSuspend(NULL);
  }

  g_core.ready = true;

  if (core_usb_test() != ERR_OK) {
    vTaskSuspend(NULL);
  }

  g_bootcmd = BOOTCMD_SWITCH_FW;
  pwr_reboot();
}
#endif
