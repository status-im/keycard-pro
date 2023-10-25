#ifdef TEST_APP
#include "FreeRTOS.h"
#include "task.h"

#include "common.h"
#include "core/core.h"
#include "core/settings.h"
#include "keycard/keycard.h"
#include "keycard/keycard_cmdset.h"
#include "mem.h"
#include "ui/ui_internal.h"
#include "usb/usb.h"
#include "ur/ur.h"
#include "ur/auth_types.h"
#include "ur/auth_encode.h"

#define TEST_AID_LEN 9
const uint8_t TEST_AID[] = {0xa0, 0x00, 0x00, 0x08, 0x04, 0x00, 0x01, 0x01, 0x01};

static app_err_t core_keypad_test() {
  g_ui_cmd.type = UI_CMD_INPUT_STRING;
  if (ui_signal_wait(0) != CORE_EVT_UI_OK) {
    return ERR_CANCEL;
  }

  return ERR_OK;
}

static app_err_t core_lcd_test() {
  hal_pwm_set_dutycycle(PWM_BACKLIGHT, 5);

  if (ui_info("Brightness test", "Screen is at minimum brightness. Press is OK if readable, cancel otherwise", 1) != CORE_EVT_UI_OK) {
    return ERR_CANCEL;
  }

  hal_pwm_set_dutycycle(PWM_BACKLIGHT, 50);
  if (ui_info("Brightness test", "Screen is at half brightness. Press is OK if brighter than before, cancel otherwise", 1) != CORE_EVT_UI_OK) {
    return ERR_CANCEL;
  }

  hal_pwm_set_dutycycle(PWM_BACKLIGHT, 100);
  if (ui_info("Brightness test", "Screen is at full brightness. Press is OK if very bright, cancel otherwise", 1) != CORE_EVT_UI_OK) {
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
    ui_info("Invalid QR", "The scanned QR code is invalid", 1);
    return ERR_CANCEL;
  }

  return ERR_OK;
}

static app_err_t core_card_test() {
  while (hal_gpio_get(GPIO_SMARTCARD_PRESENT) == GPIO_RESET) {
    ui_info("Insert card", "Insert card and press OK", 1);
  }

  keycard_init(&g_core.keycard);
  smartcard_activate(&g_core.keycard.sc);

  if (g_core.keycard.sc.state != SC_READY) {
    ui_info("Card error", "Couldn't start communication with the card", 1);
    return ERR_CANCEL;
  }

  if (keycard_cmd_select(&g_core.keycard, TEST_AID, TEST_AID_LEN) != ERR_OK) {
    ui_info("Card error", "Couldn't send SELECT APDU", 1);
    return ERR_CANCEL;
  }

  if (APDU_SW(&g_core.keycard.apdu) != SW_OK) {
    ui_keycard_wrong_card();
    return ERR_CANCEL;
  }

  app_info_t info;
  if (application_info_parse(APDU_RESP(&g_core.keycard.apdu), &info) != ERR_OK) {
    ui_keycard_wrong_card();
    return ERR_CANCEL;
  }

  return ERR_OK;
}

static app_err_t core_usb_test() {
  usb_start_if_connected();
  while (!usb_connected()) {
    ui_info("Plug USB", "Plug USB and press OK", 1);
  }

  g_ui_cmd.type = UI_CMD_INFO;
  g_ui_cmd.params.info.dismissable = 1;
  g_ui_cmd.params.info.title = "Waiting USB command";
  g_ui_cmd.params.info.msg = "Send a GET_APP_CONF command via USB";

  while (ui_signal_wait(1) != CORE_EVT_USB_CMD) {
    continue;
  }

  apdu_t* apdu = &g_core.usb_command.apdu;

  if ((APDU_CLA(apdu) != 0xe0) || (APDU_INS(apdu) != INS_GET_APP_CONF)) {
    ui_info("USB Error", "Invalid USB command received", 1);
    return ERR_CANCEL;
  }

  uint8_t* data = APDU_RESP(apdu);
  data[0] = FW_VERSION[0];
  data[1] = FW_VERSION[1];
  data[2] = FW_VERSION[2];

  data[3] = 0x01;
  data[4] = 0x34;
  data[5] = 0xb3;
  data[6] = 0x71;
  data[7] = 0x90;
  data[8] = 0x00;
  apdu->lr = 9;

  command_init_send(&g_core.usb_command);
  vTaskDelay(pdMS_TO_TICKS(20));
  return ERR_OK;
}

void core_task_entry(void* pvParameters) {
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

  ui_info("Test OK", "All tests passed, press any key to reboot to application firmware", 1);

  g_bootcmd = BOOTCMD_SWITCH_FW;
  hal_reboot();
}
#endif
