#ifdef TEST_APP
#include "FreeRTOS.h"
#include "task.h"

#include "common.h"
#include "core/core.h"
#include "core/settings.h"
#include "keycard/keycard.h"
#include "mem.h"
#include "usb/usb.h"
#include "ur/ur.h"
#include "ur/auth_types.h"
#include "ur/auth_encode.h"

static app_err_t core_keypad_test() {
  g_ui_cmd.type = UI_CMD_INPUT_STRING;
  if (ui_signal_wait(0) != CORE_EVT_UI_OK) {
    return ERR_CANCEL;
  }

  return ERR_OK;
}

static app_err_t core_lcd_test() {
  hal_pwm_set_dutycycle(PWM_BACKLIGHT, 0);

  if (ui_info("Screen is at minimum brightness. Press is OK if readable, cancel otherwise", "Brightness test", 1) != CORE_EVT_UI_OK) {
    return ERR_CANCEL;
  }

  hal_pwm_set_dutycycle(PWM_BACKLIGHT, 50);
  if (ui_info("Screen is at half brightness. Press is OK if brighter than before, cancel otherwise", "Brightness test", 1) != CORE_EVT_UI_OK) {
    return ERR_CANCEL;
  }

  hal_pwm_set_dutycycle(PWM_BACKLIGHT, 100);
  if (ui_info("Screen is at full brightness. Press is OK if very bright, cancel otherwise", "Brightness test", 1) != CORE_EVT_UI_OK) {
    return ERR_CANCEL;
  }

  hal_pwm_set_dutycycle(PWM_BACKLIGHT, 75);

  g_ui_cmd.type = UI_CMD_LCD_BRIGHTNESS;
  if (ui_signal_wait(0) != CORE_EVT_UI_OK) {
    return ERR_CANCEL;
  }

  return ERR_OK;
}

static app_err_t core_camera_test() {
  struct dev_auth auth;

  if (ui_qrscan(DEV_AUTH, &auth) != CORE_EVT_UI_OK) {
    return ERR_CANCEL;
  }

  if (memcmp(auth._dev_auth_challenge._dev_auth_challenge.value, "0123456789abcdefABCDEF9876543210", 32)) {
    return ERR_CANCEL;
  }

  return ERR_OK;
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
