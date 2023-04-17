#ifndef _UI_INTERNAL_
#define _UI_INTERNAL_

#include "FreeRTOS.h"
#include "task.h"

#include "menu.h"
#include "keypad/keypad.h"
#include "ethereum/ethUstream.h"
#include "ur/eip4527_types.h"

#define UI_NOTIFICATION_IDX 2

#define UI_CMD_EVT 1
#define UI_KEY_EVT 2

extern struct ui_cmd g_ui_cmd;
extern keypad_key_t g_last_key;

enum cmd_type {
  UI_CMD_MSG,
  UI_CMD_MENU,
  UI_CMD_DISPLAY_TXN,
  UI_CMD_QRSCAN,
  UI_CMD_INPUT_PIN,
  UI_CMD_INPUT_MNEMO
};

struct cmd_msg {
  const char* msg;
};

struct cmd_txn {
  const txContent_t* tx;
};

struct cmd_menu {
  const menu_t* menu;
  i18n_str_id_t* selected;
};

struct cmd_qrscan {
  struct eth_sign_request* out;
};

struct cmd_input_pin {
  const char* prompt;
  uint8_t len;
  uint8_t* out;
};

struct cmd_input_mnemo {

};

union cmd_params {
  struct cmd_msg msg;
  struct cmd_txn txn;
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
    return g_last_key;
  }

  return KEYPAD_KEY_INVALID;
}

#endif
