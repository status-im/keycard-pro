#include "dialog.h"
#include "crypto/address.h"
#include "crypto/bignum.h"
#include "crypto/secp256k1.h"
#include "crypto/util.h"
#include "ethereum/eth_db.h"
#include "ethereum/ethUstream.h"
#include "ethereum/ethUtils.h"
#include "theme.h"
#include "ui/ui_internal.h"

#define TX_CONFIRM_TIMEOUT 30000
#define BIGNUM_STRING_LEN 84
#define MAX_PAGE_COUNT 50

app_err_t dialog_line(screen_text_ctx_t* ctx, const char* str, uint16_t line_height) {
  screen_area_t fillarea = { 0, ctx->y, SCREEN_WIDTH, line_height };
  screen_fill_area(&fillarea, ctx->bg);

  uint16_t tmp = ctx->y;
  ctx->y += ((line_height - ctx->font->yAdvance) / 2);
  app_err_t err = screen_draw_string(ctx, str) == HAL_SUCCESS? ERR_OK : ERR_HW;
  ctx->y = tmp + line_height;
  return err;
}

app_err_t dialog_separator(uint16_t yOff) {
  screen_area_t fillarea = { 0, yOff, SCREEN_WIDTH, TH_SEP_HEIGHT };
  return screen_fill_area(&fillarea, TH_COLOR_SEP) == HAL_SUCCESS? ERR_OK : ERR_HW;
}

app_err_t dialog_title(const char* title) {
  screen_text_ctx_t ctx = { TH_FONT_TITLE, TH_COLOR_TITLE_FG, TH_COLOR_TITLE_BG, TH_TITLE_LEFT_MARGIN, 0 };
  if (dialog_line(&ctx, title, TH_TITLE_HEIGHT) != ERR_OK) {
    return ERR_HW;
  }

  ctx.y = ((TH_TITLE_HEIGHT - ctx.font->yAdvance) / 2);
  ctx.x = 280;

  uint8_t buf[11];
  uint8_t *battery = u32toa(g_ui_ctx.battery, buf, 11);

  return screen_draw_string(&ctx, (char *) battery) == HAL_SUCCESS? ERR_OK : ERR_HW;;
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

static void dialog_chain(screen_text_ctx_t *ctx, const char* name) {
  dialog_label(ctx, LSTR(TX_CHAIN));
  dialog_data(ctx, name);
}

static void dialog_tx_address(screen_text_ctx_t *ctx, const uint8_t* to) {
  char address[41];
  ethereum_address_checksum(to, address);

  dialog_label(ctx, LSTR(TX_ADDRESS));
  dialog_data(ctx, address);
}

static void dialog_calculate_fees(bignum256* fees) {
  bignum256 gas_amount;

  bn_read_compact_be(g_ui_cmd.params.txn.tx->startgas.value, g_ui_cmd.params.txn.tx->startgas.length, &gas_amount);
  bn_read_compact_be(g_ui_cmd.params.txn.tx->gasprice.value, g_ui_cmd.params.txn.tx->gasprice.length, fees);
  bn_multiply(&gas_amount, fees, &secp256k1.prime);
}

static void dialog_amount(screen_text_ctx_t* ctx, i18n_str_id_t prompt, const bignum256* amount, int decimals, const char* ticker) {
  char tmp[BIGNUM_STRING_LEN+strlen(ticker)+2];
  bn_format(amount, NULL, ticker, decimals, 0, 0, ',', tmp, sizeof(tmp));

  dialog_label(ctx, LSTR(prompt));
  dialog_data(ctx, tmp);
}

app_err_t dialog_confirm_tx() {
  chain_desc_t chain;
  erc20_desc_t token;
  chain.chain_id = g_ui_cmd.params.txn.tx->chainID;

  if (eth_db_lookup_chain(&chain) != ERR_OK) {
    uint8_t num[11];
    chain.name = (char*) u32toa(chain.chain_id, num, 11);
    chain.ticker = "???";
  }

  i18n_str_id_t title;
  const uint8_t* to;

  if (g_ui_cmd.params.txn.tx->dataType == DATA_ERC20) {
    title = TX_CONFIRM_ERC20_TITLE;
    token.chain = chain.chain_id;
    token.addr = g_ui_cmd.params.txn.tx->destination;

    to = g_ui_cmd.params.txn.tx->finalRecipient;

    if (eth_db_lookup_erc20(&token) != ERR_OK) {
      token.ticker = "???";
      token.decimals = 18;
    }
  } else {
    title = TX_CONFIRM_TITLE;
    token.ticker = chain.ticker;
    token.decimals = 18;
    to = g_ui_cmd.params.txn.tx->destination;
  }

  dialog_title(LSTR(title));

  screen_text_ctx_t ctx;
  ctx.y = TH_TITLE_HEIGHT;

  dialog_chain(&ctx, chain.name);

  dialog_tx_address(&ctx, to);

  bignum256 data;
  bn_read_compact_be(g_ui_cmd.params.txn.tx->value.value, g_ui_cmd.params.txn.tx->value.length, &data);
  dialog_amount(&ctx, TX_AMOUNT, &data, token.decimals, token.ticker);

  dialog_calculate_fees(&data);
  dialog_amount(&ctx, TX_FEE, &data, 18, chain.ticker);

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

static size_t dialog_draw_message(i18n_str_id_t title, const uint8_t* txt, size_t len) {
  dialog_title(LSTR(title));
  screen_text_ctx_t ctx;
  ctx.y = TH_TITLE_HEIGHT;
  dialog_footer(ctx.y);
  ctx.y += TH_TEXT_VERTICAL_MARGIN;

  ctx.font = TH_FONT_TEXT;
  ctx.fg = TH_COLOR_TEXT_FG;
  ctx.bg = TH_COLOR_TEXT_BG;
  ctx.x = TH_TEXT_HORIZONTAL_MARGIN;

  return screen_draw_text(&ctx, (SCREEN_WIDTH - TH_TEXT_HORIZONTAL_MARGIN), (SCREEN_HEIGHT - TH_TEXT_VERTICAL_MARGIN), txt, len);
}

app_err_t dialog_confirm_msg() {
  size_t pages[MAX_PAGE_COUNT];
  int page = 0;
  pages[0] = 0;

  while(1) {
    size_t offset = pages[page];
    size_t to_display = g_ui_cmd.params.msg.len - offset;
    size_t remaining = dialog_draw_message(MSG_CONFIRM_TITLE, &g_ui_cmd.params.msg.data[offset], to_display);

    switch(ui_wait_keypress(pdMS_TO_TICKS(TX_CONFIRM_TIMEOUT))) {
    case KEYPAD_KEY_LEFT:
      if (page > 0) {
        page--;
      }
      break;
    case KEYPAD_KEY_RIGHT:
      if (remaining && (page < MAX_PAGE_COUNT)) {
        pages[++page] = offset + (to_display - remaining);
      }
      break;
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

static app_err_t dialog_wait_dismiss() {
  while(1) {
    switch(ui_wait_keypress(portMAX_DELAY)) {
    case KEYPAD_KEY_CANCEL:
    case KEYPAD_KEY_BACK:
      return ERR_CANCEL;
    case KEYPAD_KEY_CONFIRM:
      return ERR_OK;
    default:
      break;
    }
  }
}

app_err_t dialog_info() {
  dialog_draw_message(g_ui_cmd.params.info.title, (uint8_t*) g_ui_cmd.params.info.msg, strlen(g_ui_cmd.params.info.msg));

  if (!g_ui_cmd.params.info.dismissable) {
    vTaskSuspend(NULL);
    return ERR_CANCEL;
  }

  return dialog_wait_dismiss();
}

app_err_t dialog_dev_auth() {
  if (g_ui_cmd.params.auth.auth_count > 1) {
    dialog_draw_message(DEV_AUTH_TITLE_WARNING, (uint8_t*) LSTR(DEV_AUTH_INFO_WARNING), strlen(LSTR(DEV_AUTH_INFO_WARNING)));
  } else {
    dialog_draw_message(DEV_AUTH_TITLE_SUCCESS, (uint8_t*) LSTR(DEV_AUTH_INFO_SUCCESS), strlen(LSTR(DEV_AUTH_INFO_SUCCESS)));
  }

  return dialog_wait_dismiss();
}
