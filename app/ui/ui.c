#include "app_tasks.h"
#include "core/core.h"
#include "error.h"
#include "ui.h"
#include "ui_internal.h"

core_evt_t ui_qrscan(ur_type_t type, void* out) {
  g_ui_cmd.type = UI_CMD_QRSCAN;
  g_ui_cmd.params.qrscan.out = out;
  g_ui_cmd.params.qrscan.type = type;
  return ui_signal_wait(0);
}

core_evt_t ui_menu(const char* title, const menu_t* menu, i18n_str_id_t* selected, i18n_str_id_t marked, uint8_t allow_usb) {
  g_ui_cmd.type = UI_CMD_MENU;
  g_ui_cmd.params.menu.title = title;
  g_ui_cmd.params.menu.menu = menu;
  g_ui_cmd.params.menu.selected = selected;
  g_ui_cmd.params.menu.marked = marked;
  return ui_signal_wait(allow_usb);
}

core_evt_t ui_display_tx(const uint8_t* address, const txContent_t* tx) {
  g_ui_cmd.type = UI_CMD_DISPLAY_TXN;
  g_ui_cmd.params.txn.addr = address;
  g_ui_cmd.params.txn.tx = tx;
  return ui_signal_wait(0);
}

core_evt_t ui_display_msg(const uint8_t* address, const uint8_t* msg, uint32_t len) {
  g_ui_cmd.type = UI_CMD_DISPLAY_MSG;
  g_ui_cmd.params.msg.addr = address;
  g_ui_cmd.params.msg.data = msg;
  g_ui_cmd.params.msg.len = len;
  return ui_signal_wait(0);
}

core_evt_t ui_display_eip712(const uint8_t* address, const eip712_ctx_t* eip712) {
  g_ui_cmd.type = UI_CMD_DISPLAY_EIP712;
  g_ui_cmd.params.eip712.addr = address;
  g_ui_cmd.params.eip712.data = eip712;
  return ui_signal_wait(0);
}

core_evt_t ui_display_qr(const uint8_t* data, uint32_t len, ur_type_t type) {
  g_ui_cmd.type = UI_CMD_DISPLAY_QR;
  g_ui_cmd.params.qrout.data = data;
  g_ui_cmd.params.qrout.len = len;
  g_ui_cmd.params.qrout.type = type;
  return ui_signal_wait(0);
}

core_evt_t ui_info(const char* msg, uint8_t dismissable) {
  g_ui_cmd.type = UI_CMD_INFO;
  g_ui_cmd.params.info.dismissable = dismissable;
  g_ui_cmd.params.info.msg = msg;
  return ui_signal_wait(0);
}

core_evt_t ui_prompt(const char* title, const char* msg) {
  g_ui_cmd.type = UI_CMD_PROMPT;
  g_ui_cmd.params.prompt.title = title;
  g_ui_cmd.params.prompt.msg = msg;
  return ui_signal_wait(0);
}

void ui_card_inserted() {

}

void ui_card_removed() {
}

void ui_card_transport_error() {
  ui_info(LSTR(INFO_CARD_ERROR_MSG), 0);
}

void ui_card_accepted() {
}

void ui_keycard_wrong_card() {
  ui_info(LSTR(INFO_NOT_KEYCARD), 0);
}

void ui_keycard_old_card() {
  ui_info(LSTR(INFO_OLD_KEYCARD), 0);
}

void ui_keycard_not_initialized() {
  ui_info(LSTR(INFO_NEW_CARD), 1);
}

void ui_keycard_init_failed() {
}

void ui_keycard_no_keys() {
}

void ui_keycard_ready() {
}

void ui_keycard_paired() {
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

void ui_keycard_pin_ok() {
}

void ui_keycard_puk_ok() {
}

void ui_keycard_wrong_pin(uint8_t retries) {
  if (retries > 0) {
    g_ui_cmd.type = UI_CMD_WRONG_AUTH;
    g_ui_cmd.params.wrong_auth.msg = LSTR(PIN_WRONG_WARNING);
    g_ui_cmd.params.wrong_auth.retries = retries;
    ui_signal_wait(0);
  } else {
    ui_info(LSTR(INFO_KEYCARD_BLOCKED), 1);
  }
}

void ui_keycard_wrong_puk(uint8_t retries) {
  g_ui_cmd.type = UI_CMD_WRONG_AUTH;
  g_ui_cmd.params.wrong_auth.msg = LSTR(PUK_WRONG_WARNING);
  g_ui_cmd.params.wrong_auth.retries = retries;
  ui_signal_wait(0);
}

core_evt_t ui_keycard_not_genuine() {
  return ui_prompt("", LSTR(INFO_NOT_GENUINE));
}

core_evt_t ui_prompt_try_puk() {
  i18n_str_id_t selected = MENU_UNBLOCK_PUK;
  while (ui_menu(LSTR(INFO_KEYCARD_BLOCKED), &menu_keycard_blocked, &selected, -1, 0) != CORE_EVT_UI_OK) {
    ;
  }

  return selected == MENU_RESET_CARD ? CORE_EVT_UI_CANCELLED : CORE_EVT_UI_OK;
}

core_evt_t ui_confirm_factory_reset() {
  return ui_prompt(LSTR(FACTORY_RESET_TITLE), LSTR(FACTORY_RESET_WARNING));
}

core_evt_t ui_keycard_no_pairing_slots() {
  return ui_info(LSTR(INFO_NO_PAIRING_SLOTS), 1);
}

core_evt_t ui_read_pin(uint8_t* out, int8_t retries, uint8_t dismissable) {
  g_ui_cmd.type = UI_CMD_INPUT_PIN;
  g_ui_cmd.params.input_pin.dismissable = dismissable;
  g_ui_cmd.params.input_pin.retries = retries;
  g_ui_cmd.params.input_pin.out = out;

  return ui_signal_wait(0);
}

core_evt_t ui_read_puk(uint8_t* out, int8_t retries, uint8_t dismissable) {
  g_ui_cmd.type = UI_CMD_INPUT_PUK;
  g_ui_cmd.params.input_pin.dismissable = dismissable;
  g_ui_cmd.params.input_pin.retries = retries;
  g_ui_cmd.params.input_pin.out = out;

  return ui_signal_wait(0);
}

core_evt_t ui_read_pairing(uint8_t* pairing, uint8_t *len) {
  while(ui_read_string(LSTR(PAIRING_INPUT_TITLE), (char*) pairing, len) != CORE_EVT_UI_OK) {
    continue;
  }

  return CORE_EVT_UI_OK;
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

  while (ui_menu(LSTR(MNEMO_TITLE), &menu_mnemonic, &selected, -1, 0) != CORE_EVT_UI_OK);

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

void ui_update_progress(const char* title, uint8_t progress) {
  g_ui_cmd.type = UI_CMD_PROGRESS;
  g_ui_cmd.params.progress.title = title;
  g_ui_cmd.params.progress.value = progress;

  return ui_signal();
}
