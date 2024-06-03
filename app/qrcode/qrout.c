#include "qrcodegen.h"
#include "qrout.h"
#include "crypto/util.h"
#include "screen/screen.h"
#include "ur/ur.h"
#include "ui/dialog.h"
#include "ui/theme.h"
#include "ui/ui_internal.h"

#define QR_DISPLAY_TIMEOUT 60000

app_err_t qrout_display(const char* str, const char* title, uint16_t max_y) {
  uint8_t tmpBuf[qrcodegen_BUFFER_LEN_MAX];
  uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];

  dialog_title_colors(title, SCREEN_COLOR_WHITE, SCREEN_COLOR_BLACK, SCREEN_COLOR_BLACK);
  dialog_footer_colors(TH_TITLE_HEIGHT, SCREEN_COLOR_WHITE);

  if (!qrcodegen_encodeText(str, tmpBuf, qrcode, qrcodegen_Ecc_LOW, 1, 40, qrcodegen_Mask_AUTO, 1)) {
    return ERR_DATA;
  }

  screen_area_t qrarea;
  qrarea.height = max_y - (TH_TITLE_HEIGHT + TH_QRCODE_VERTICAL_MARGIN);

  int qrsize = qrcodegen_getSize(qrcode);
  int scale = qrarea.height / qrsize;
  qrarea.height = scale * qrsize;

  qrarea.width = qrarea.height;
  qrarea.x = (SCREEN_WIDTH - qrarea.width) / 2;
  qrarea.y = TH_TITLE_HEIGHT + (((max_y - TH_TITLE_HEIGHT) - qrarea.height) / 2);

  screen_draw_qrcode(&qrarea, qrcode, qrsize, scale);
  dialog_nav_hints_colors(0, ICON_NAV_NEXT, SCREEN_COLOR_WHITE, SCREEN_COLOR_BLACK);

  return ERR_OK;
}

app_err_t qrout_display_ur() {
  ur_t ur;
  ur.data = (uint8_t*) g_ui_cmd.params.qrout.data;
  ur.data_len = g_ui_cmd.params.qrout.len;
  ur.type = g_ui_cmd.params.qrout.type;

  char urstr[qrcodegen_BUFFER_LEN_MAX/2];

  if (ur_encode(&ur, urstr, sizeof(urstr)) != ERR_OK) {
    return ERR_DATA;
  }

  if (qrout_display(urstr, g_ui_cmd.params.qrout.title, (SCREEN_HEIGHT - TH_QRCODE_VERTICAL_MARGIN)) != ERR_OK) {
    return ERR_DATA;
  }

  while(1) {
    switch(ui_wait_keypress(pdMS_TO_TICKS(QR_DISPLAY_TIMEOUT))) {
    case KEYPAD_KEY_INVALID:
    case KEYPAD_KEY_CONFIRM:
      return ERR_OK;
    default:
      break;
    }
  }

  return ERR_OK;
}

app_err_t qrout_display_address() {
  screen_text_ctx_t ctx = {
      .bg = SCREEN_COLOR_WHITE,
      .fg = SCREEN_COLOR_BLACK,
      .font = TH_FONT_DATA,
      .x = TH_QRCODE_ADDR_MARGIN,
      .y = (SCREEN_HEIGHT - TH_QRCODE_VERTICAL_MARGIN - (TH_DATA_HEIGHT * 2) - TH_TEXT_VERTICAL_MARGIN)
  };

  if (qrout_display(g_ui_cmd.params.address.address, "", ctx.y) != ERR_OK) {
    return ERR_DATA;
  }

  screen_draw_centered_string(&ctx, g_ui_cmd.params.address.address);

  ctx.x = 0;
  ctx.y = SCREEN_HEIGHT - (TH_FONT_TITLE)->yAdvance;
  ctx.font = TH_FONT_TITLE;

  uint8_t index_buf[UINT32_STRING_LEN];
  uint8_t* index_str = u32toa(*g_ui_cmd.params.address.index, index_buf, UINT32_STRING_LEN);

  screen_draw_centered_string(&ctx, (char*) index_str);

  while(1) {
    switch(ui_wait_keypress(portMAX_DELAY)) {
    case KEYPAD_KEY_CANCEL:
    case KEYPAD_KEY_BACK:
    case KEYPAD_KEY_INVALID:
    case KEYPAD_KEY_CONFIRM:
      *g_ui_cmd.params.address.index = UINT32_MAX;
      return ERR_OK;
    case KEYPAD_KEY_LEFT:
      if (*g_ui_cmd.params.address.index) {
        (*g_ui_cmd.params.address.index)--;
        return ERR_OK;
      }

      break;
    case KEYPAD_KEY_RIGHT:
      if (*g_ui_cmd.params.address.index < INT32_MAX) {
        (*g_ui_cmd.params.address.index)++;
        return ERR_OK;
      }
      break;
    default:
      break;
    }
  }
}
