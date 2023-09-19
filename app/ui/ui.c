#include "app_tasks.h"
#include "core/core.h"
#include "error.h"
#include "ui.h"
#include "ui_internal.h"

static inline core_evt_t ui_signal_wait(uint8_t allow_usb) {
  xTaskNotifyIndexed(APP_TASK(ui), UI_NOTIFICATION_IDX, UI_CMD_EVT, eSetBits);
  return core_wait_event(allow_usb);
}

core_evt_t ui_qrscan(ur_type_t type, void* out) {
  g_ui_cmd.type = UI_CMD_QRSCAN;
  g_ui_cmd.params.qrscan.out = out;
  g_ui_cmd.params.qrscan.type = type;
  return ui_signal_wait(0);
}

core_evt_t ui_menu(const char* title, const menu_t* menu, i18n_str_id_t* selected, uint8_t allow_usb) {
  g_ui_cmd.type = UI_CMD_MENU;
  g_ui_cmd.params.menu.title = title;
  g_ui_cmd.params.menu.menu = menu;
  g_ui_cmd.params.menu.selected = selected;
  return ui_signal_wait(allow_usb);
}

core_evt_t ui_display_tx(const txContent_t* tx) {
  g_ui_cmd.type = UI_CMD_DISPLAY_TXN;
  g_ui_cmd.params.txn.tx = tx;
  return ui_signal_wait(0);
}

core_evt_t ui_display_msg(const uint8_t* msg, uint32_t len) {
  g_ui_cmd.type = UI_CMD_DISPLAY_MSG;
  g_ui_cmd.params.msg.data = msg;
  g_ui_cmd.params.msg.len = len;
  return ui_signal_wait(0);
}

core_evt_t ui_display_qr(const uint8_t* data, uint32_t len, ur_type_t type) {
  g_ui_cmd.type = UI_CMD_DISPLAY_QR;
  g_ui_cmd.params.qrout.data = data;
  g_ui_cmd.params.qrout.len = len;
  g_ui_cmd.params.qrout.type = type;
  return ui_signal_wait(0);
}

core_evt_t ui_info(i18n_str_id_t title, const char* msg, uint8_t dismissable) {
  g_ui_cmd.type = UI_CMD_INFO;
  g_ui_cmd.params.info.dismissable = dismissable;
  g_ui_cmd.params.info.title = title;
  g_ui_cmd.params.info.msg = msg;
  return ui_signal_wait(0);
}

void ui_card_inserted() {

}

void ui_card_removed() {
}

void ui_card_transport_error() {
  ui_info(INFO_ERROR_TITLE, LSTR(INFO_CARD_ERROR_MSG), 0);
}

void ui_card_accepted() {
}

void ui_keycard_wrong_card() {
  ui_info(INFO_ERROR_TITLE, LSTR(INFO_NOT_KEYCARD), 0);
}

void ui_keycard_not_initialized() {
  ui_info(INFO_NEW_CARD_TITLE, LSTR(INFO_NEW_CARD), 1);
}

void ui_keycard_init_failed() {
}

void ui_keycard_no_keys() {
}

void ui_keycard_ready() {
}

void ui_keyard_paired() {
}

void ui_keycard_already_paired() {
}

void ui_keycard_pairing_failed() {
}

void ui_keycard_flash_failed() {
}

void ui_keycard_secure_channel_failed() {
}

void ui_keycard_secure_channel_ok() {
}

void ui_keycard_wrong_pin() {
}

void ui_keycard_pin_ok() {
}

void ui_keycard_puk_ok() {
}

void ui_keycard_wrong_puk() {
}

core_evt_t ui_prompt_try_puk() {
  return CORE_EVT_UI_OK;
}

core_evt_t ui_confirm_factory_reset() {
  return CORE_EVT_UI_OK;
}

core_evt_t ui_read_pin(uint8_t* out, int8_t retries) {
  g_ui_cmd.type = UI_CMD_INPUT_PIN;
  g_ui_cmd.params.input_pin.retries = retries;
  g_ui_cmd.params.input_pin.out = out;

  return ui_signal_wait(0);
}

core_evt_t ui_read_puk(uint8_t* out, int8_t retries) {
  return CORE_EVT_UI_CANCELLED;
}

core_evt_t ui_read_pairing(uint8_t* pairing, uint8_t *len) {
  return ui_read_string(LSTR(PAIRING_INPUT_TITLE), (char*) pairing, len);
}

core_evt_t ui_read_string(const char* title, char* out, uint8_t* len) {
  g_ui_cmd.type = UI_CMD_INPUT_STRING;
  g_ui_cmd.params.input_string.title = title;
  g_ui_cmd.params.input_string.out = out;
  g_ui_cmd.params.input_string.len = len;

  return ui_signal_wait(0);
}

void ui_seed_loaded() {
}

core_evt_t ui_read_mnemonic_len(uint32_t* len) {
  i18n_str_id_t selected = MENU_MNEMO_12WORDS;

  while (ui_menu(LSTR(MNEMO_TITLE), &menu_mnemonic, &selected, 0) != CORE_EVT_UI_OK);

  switch(selected) {
  case MENU_MNEMO_12WORDS:
    *len = 12;
    break;
  case MENU_MNEMO_18WORDS:
    *len = 18;
    break;
  case MENU_MNEMO_24WORDS:
    *len = 24;
    break;
  case MENU_MNEMO_GENERATE:
  default:
    *len = 12;
    return CORE_EVT_UI_CANCELLED;
  }

  return CORE_EVT_UI_OK;
}

core_evt_t ui_backup_mnemonic(uint16_t* indexes, uint32_t len) {
  g_ui_cmd.type = UI_CMD_BACKUP_MNEMO;
  g_ui_cmd.params.mnemo.indexes = indexes;
  g_ui_cmd.params.mnemo.len = len;

  return ui_signal_wait(0);
}

core_evt_t ui_read_mnemonic(uint16_t* indexes, uint32_t len) {
  g_ui_cmd.type = UI_CMD_INPUT_MNEMO;
  g_ui_cmd.params.mnemo.indexes = indexes;
  g_ui_cmd.params.mnemo.len = len;

  return ui_signal_wait(0);
}

core_evt_t ui_confirm_eth_address(const char* address) {
  return CORE_EVT_UI_OK;
}

core_evt_t ui_device_auth(uint32_t first_auth, uint32_t auth_time, uint32_t auth_count) {
  g_ui_cmd.type = UI_CMD_DEV_AUTH;
  g_ui_cmd.params.auth.first_auth = first_auth;
  g_ui_cmd.params.auth.auth_time = auth_time;
  g_ui_cmd.params.auth.auth_count = auth_count;

  return ui_signal_wait(0);
}

core_evt_t ui_settings_brightness(uint8_t* brightness) {
  g_ui_cmd.type = UI_CMD_LCD_BRIGHTNESS;
  g_ui_cmd.params.lcd.brightness = brightness;

  return ui_signal_wait(0);
}
