#ifndef _UI_INTERNAL_
#define _UI_INTERNAL_

#define UI_NOTIFICATION_IDX 2

extern struct ui_cmd g_ui_cmd;

enum cmd_type {
  UI_CMD_MSG,
  UI_CMD_MENU,
  UI_CMD_DISPLAY_TXN,
  UI_CMD_QRSCAN,
  UI_CMD_INPUT_PIN,
  UI_CMD_INPUT_MNEMO
};

struct cmd_msg {
  char* msg;
};

struct cmd_txn {

};

struct cmd_menu {

};

struct cmd_qrscan {

};

struct cmd_input_pin {
  char* prompt;
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
  union cmd_params params;
};

#endif
