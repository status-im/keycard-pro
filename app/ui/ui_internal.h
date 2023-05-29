#ifndef _UI_INTERNAL_
#define _UI_INTERNAL_

#include "FreeRTOS.h"
#include "task.h"

#include "menu.h"
#include "keypad/keypad.h"
#include "ethereum/ethUstream.h"
#include "qrcode/qrcode.h"
#include "ur/eip4527_types.h"
#include "ur/ur.h"

#define UI_NOTIFICATION_IDX 2

#define UI_CMD_EVT 1
#define UI_KEY_EVT 2

extern struct ui_cmd g_ui_cmd;
extern struct ui_ctx g_ui_ctx;

enum cmd_type {
  UI_CMD_DIALOG,
  UI_CMD_MENU,
  UI_CMD_DISPLAY_TXN,
  UI_CMD_DISPLAY_MSG,
  UI_CMD_DISPLAY_QR,
  UI_CMD_QRSCAN,
  UI_CMD_INPUT_PIN,
  UI_CMD_INPUT_MNEMO
};

struct cmd_dialog {
  const char* msg;
};

struct cmd_txn {
  const txContent_t* tx;
};

struct cmd_msg {
  const uint8_t* data;
  uint32_t len;
};

struct cmd_qrout {
  const uint8_t* data;
  uint32_t len;
  ur_type_t type;
};

struct cmd_menu {
  const menu_t* menu;
  i18n_str_id_t* selected;
};

struct cmd_qrscan {
  struct eth_sign_request* out;
};

struct cmd_input_pin {
  int8_t retries;
  uint8_t len;
  uint8_t* out;
};

struct cmd_input_mnemo {

};

union cmd_params {
  struct cmd_dialog dialog;
  struct cmd_txn txn;
  struct cmd_msg msg;
  struct cmd_qrout qrout;
  struct cmd_menu menu;
  struct cmd_qrscan qrscan;
  struct cmd_input_pin input_pin;
  struct cmd_input_mnemo input_mnemo;
};

struct ui_cmd {
  enum cmd_type type;
  uint8_t received;
  app_err_t result;
  union cmd_params params;
};

struct ui_ctx {
  keypad_t keypad;
};

static inline uint32_t ui_wait_event(uint32_t timeout) {
  uint32_t evt;
  return xTaskNotifyWaitIndexed(UI_NOTIFICATION_IDX, 0, UINT32_MAX, &evt, timeout) == pdPASS ? evt : 0;
}

static inline keypad_key_t ui_wait_keypress(uint32_t timeout) {
  uint32_t evt = ui_wait_event(timeout);

  if (evt & UI_CMD_EVT) {
    g_ui_cmd.received = 1;
    return KEYPAD_KEY_CANCEL;
  } else if (evt & UI_KEY_EVT) {
    return g_ui_ctx.keypad.last_key;
  }

  return KEYPAD_KEY_INVALID;
}

#endif
