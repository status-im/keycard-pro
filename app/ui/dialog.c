#include "dialog.h"
#include "crypto/address.h"
#include "ethereum/ethUstream.h"
#include "screen/screen.h"
#include "theme.h"
#include "ui/ui_internal.h"

#define TX_CONFIRM_TIMEOUT 30000

app_err_t display_tx() {
  screen_fill_area(&screen_fullarea, TH_COLOR_BG);
  char address[41];
  ethereum_address_checksum(g_ui_cmd.params.txn.tx->destination, address);

  screen_text_ctx_t ctx = {0};
  ctx.bg = TH_COLOR_BG;
  ctx.fg = TH_COLOR_FG;
  ctx.font = TH_FONT_CAPTION;
  ctx.xStart = TH_MSG_LEFT_MARGIN;
  ctx.y = TH_MSG_TOP_MARGIN;

  if (screen_draw_string(&ctx, LSTR(TX_CAPTION_ADDRESS)) != HAL_OK) {
    return ERR_HW;
  }

  screen_newline(&ctx);
  ctx.font = TH_FONT_DATA;

  if (screen_draw_string(&ctx, address) != HAL_OK) {
    return ERR_HW;
  }

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
