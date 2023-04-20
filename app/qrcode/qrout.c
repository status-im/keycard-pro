#include "camera/camera.h"
#include "log/log.h"
#include "qrcodegen.h"
#include "qrout.h"
#include "screen/screen.h"
#include "ur/ur.h"
#include "ui/theme.h"
#include "ui/ui_internal.h"

#define QR_DISPLAY_TIMEOUT 60000

app_err_t qrout_run() {
  g_ui_ctx.ur.data = (uint8_t*) g_ui_cmd.params.qrout.data;
  g_ui_ctx.ur.data_len = g_ui_cmd.params.qrout.len;
  g_ui_ctx.ur.type = g_ui_cmd.params.qrout.type;
  g_ui_ctx.ur.is_complete = 1;
  g_ui_ctx.ur.is_multipart = 0;

  uint8_t* tmpBuf = g_camera_fb[0];
  uint8_t* qrcode = &tmpBuf[qrcodegen_BUFFER_LEN_MAX];
  char* urstr = (char*)&qrcode[qrcodegen_BUFFER_LEN_MAX];

  if (ur_encode(&g_ui_ctx.ur, urstr, CAMERA_FB_SIZE - (qrcodegen_BUFFER_LEN_MAX * 2)) != ERR_OK) {
    return ERR_DATA;
  }

  if (!qrcodegen_encodeText(urstr, tmpBuf, qrcode, qrcodegen_Ecc_LOW, 4, 40, qrcodegen_Mask_AUTO, 1)) {
    return ERR_DATA;
  }

  screen_fill_area(&screen_fullarea, SCREEN_COLOR_WHITE);
  screen_draw_qrcode(&screen_camarea, qrcode);

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
