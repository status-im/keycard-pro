#include "qrout.h"
#include "log/log.h"
#include "qrcodegen.h"
#include "ur/ur.h"
#include "ui/theme.h"
#include "ui/ui_internal.h"

#define QR_DISPLAY_TIMEOUT 60000

app_err_t qrout_run() {

  while(1) {
    switch(ui_wait_keypress(pdMS_TO_TICKS(QR_DISPLAY_TIMEOUT))) {
    case KEYPAD_KEY_CANCEL:
    case KEYPAD_KEY_BACK:
    case KEYPAD_KEY_INVALID:
    case KEYPAD_KEY_CONFIRM:
      return ERR_OK;
    default:
      break;
    }
  }

  return ERR_OK;
}
