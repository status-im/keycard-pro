#include "dialog.h"
#include "crypto/address.h"
#include "ethereum/ethUstream.h"
#include "theme.h"
#include "ui/ui_internal.h"

#define TX_CONFIRM_TIMEOUT 30000

app_err_t dialog_line(screen_text_ctx_t* ctx, const char* str, uint16_t line_height) {
  screen_area_t fillarea = { 0, ctx->y, SCREEN_WIDTH, line_height };
  screen_fill_area(&fillarea, ctx->bg);

  uint16_t tmp = ctx->y;
  ctx->y += ((line_height - ctx->font->yAdvance) / 2);
  app_err_t err = screen_draw_string(ctx, str) == HAL_OK? ERR_OK : ERR_HW;
  ctx->y = tmp + line_height;
  return err;
}

app_err_t dialog_separator(uint16_t yOff) {
  screen_area_t fillarea = { 0, yOff, SCREEN_WIDTH, TH_SEP_HEIGHT };
  return screen_fill_area(&fillarea, TH_COLOR_SEP) == HAL_OK? ERR_OK : ERR_HW;
}

app_err_t dialog_title(const char* title) {
  screen_text_ctx_t ctx = { TH_FONT_TITLE, TH_COLOR_TITLE_FG, TH_COLOR_TITLE_BG, TH_TITLE_LEFT_MARGIN, 0 };
  return dialog_line(&ctx, title, TH_TITLE_HEIGHT);
}

app_err_t dialog_footer(uint16_t yOff) {
  screen_area_t area = { 0, yOff, SCREEN_WIDTH, SCREEN_HEIGHT - yOff };
  return screen_fill_area(&area, TH_COLOR_BG);
}

static inline void dialog_label(screen_text_ctx_t *ctx, const char* label) {
  ctx->font = TH_FONT_LABEL;
  ctx->fg = TH_COLOR_LABEL_FG;
  ctx->bg = TH_COLOR_LABEL_BG;
  ctx->x = TH_LABEL_LEFT_MARGIN;
  dialog_line(ctx, label, TH_LABEL_HEIGHT);
}

static inline void dialog_data(screen_text_ctx_t *ctx, const char* data) {
  ctx->font = TH_FONT_DATA;
  ctx->fg = TH_COLOR_DATA_FG;
  ctx->bg = TH_COLOR_DATA_BG;
  ctx->x = TH_DATA_LEFT_MARGIN;
  dialog_line(ctx, data, TH_DATA_HEIGHT);
}

app_err_t dialog_confirm_tx() {
  dialog_title(LSTR(TX_CONFIRM_TITLE));

  screen_text_ctx_t ctx;
  ctx.y = TH_TITLE_HEIGHT;

  dialog_label(&ctx, LSTR(TX_ADDRESS));

  char address[41];
  ethereum_address_checksum(g_ui_cmd.params.txn.tx->destination, address);

  dialog_data(&ctx, address);

  dialog_label(&ctx, LSTR(TX_AMOUNT));
  dialog_data(&ctx, address);

  dialog_label(&ctx, LSTR(TX_FEE));
  dialog_data(&ctx, address);

  dialog_footer(ctx.y);

  while(1) {
    switch(ui_wait_keypress(pdMS_TO_TICKS(TX_CONFIRM_TIMEOUT))) {
    case KEYPAD_KEY_CANCEL:
    case KEYPAD_KEY_BACK:
    case KEYPAD_KEY_INVALID:
      return ERR_CANCEL;
    case KEYPAD_KEY_CONFIRM:
      return ERR_OK;
    default:
      break;
    }
  }
}
