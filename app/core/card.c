#include "card.h"
#include "core.h"
#include "keycard/keycard_cmdset.h"

void card_change_name() {

}

void card_change_pin() {
  SC_BUF(pin, KEYCARD_PIN_LEN);

  if (ui_read_pin(pin, PIN_NEW_CODE) != CORE_EVT_UI_OK) {
    return;
  }

  app_err_t err = keycard_cmd_change_credential(&g_core.keycard, KEYCARD_PIN, pin, KEYCARD_PIN_LEN);
  memset(pin, 0, KEYCARD_PIN_LEN);

  if (err != ERR_OK) {
    ui_card_transport_error();
  }
}

void card_change_puk() {
  SC_BUF(puk, KEYCARD_PUK_LEN);

  if (ui_read_puk(puk, PUK_NEW_CODE) != CORE_EVT_UI_OK) {
    return;
  }

  app_err_t err = keycard_cmd_change_credential(&g_core.keycard, KEYCARD_PUK, puk, KEYCARD_PUK_LEN);
  memset(puk, 0, KEYCARD_PUK_LEN);

  if (err != ERR_OK) {
    ui_card_transport_error();
  }
}

void card_change_pairing() {

}

void card_reset() {

}
