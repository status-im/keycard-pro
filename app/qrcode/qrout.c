#include "qrcodegen.h"
#include "qrout.h"
#include "screen/screen.h"
#include "ur/ur.h"
#include "ui/dialog.h"
#include "ui/theme.h"
#include "ui/ui_internal.h"

#define QR_DISPLAY_TIMEOUT 60000

app_err_t qrout_run() {
  ur_t ur;
  ur.data = (uint8_t*) g_ui_cmd.params.qrout.data;
  ur.data_len = g_ui_cmd.params.qrout.len;
  ur.type = g_ui_cmd.params.qrout.type;

  uint8_t tmpBuf[qrcodegen_BUFFER_LEN_MAX];
  uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
  char urstr[qrcodegen_BUFFER_LEN_MAX/2];

  dialog_title_colors(LSTR(QR_OUTPUT_TITLE), SCREEN_COLOR_WHITE, SCREEN_COLOR_BLACK, SCREEN_COLOR_BLACK);
  screen_area_t bgarea = { 0, TH_TITLE_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - TH_TITLE_HEIGHT };
  screen_fill_area(&bgarea, SCREEN_COLOR_WHITE);

  if (ur_encode(&ur, urstr, sizeof(urstr)) != ERR_OK) {
    return ERR_DATA;
  }

  if (!qrcodegen_encodeText(urstr, tmpBuf, qrcode, qrcodegen_Ecc_LOW, 1, 40, qrcodegen_Mask_AUTO, 1)) {
    return ERR_DATA;
  }

  screen_area_t qrarea;
  qrarea.height = SCREEN_HEIGHT - (TH_TITLE_HEIGHT + (TH_QRCODE_VERTICAL_MARGIN * 2));

  int qrsize = qrcodegen_getSize(qrcode);
  int scale = qrarea.height / qrsize;
  qrarea.height = scale * qrsize;

  qrarea.width = qrarea.height;
  qrarea.x = (SCREEN_WIDTH - qrarea.width) / 2;
  qrarea.y = TH_TITLE_HEIGHT + (((SCREEN_HEIGHT - TH_TITLE_HEIGHT) - qrarea.height) / 2);

  screen_draw_qrcode(&qrarea, qrcode, qrsize, scale);

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
