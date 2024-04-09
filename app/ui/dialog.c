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
#define MESSAGE_MAX_X (SCREEN_WIDTH - TH_TEXT_HORIZONTAL_MARGIN)
#define MESSAGE_MAX_Y (SCREEN_HEIGHT - TH_TEXT_VERTICAL_MARGIN)
#define MAX_MSG_TITLE_LEN 80

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

  ctx.font = TH_FONT_ICONS;
  ctx.y = ((TH_TITLE_HEIGHT - ctx.font->yAdvance) / 2);
  ctx.x = 280;

  uint8_t i = g_ui_ctx.battery / 25;
  if (i > 5) {
    i = 4;
  } else if (i >= 4) {
    i = 3;
  }

  return screen_draw_glyph(&ctx, &ctx.font->glyph[i]) == HAL_SUCCESS? ERR_OK : ERR_HW;
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

static inline void dialog_inline_data(screen_text_ctx_t *ctx, const char* data) {
  ctx->font = TH_FONT_DATA;
  ctx->fg = TH_COLOR_DATA_FG;
  ctx->bg = TH_COLOR_DATA_BG;
  ctx->x = TH_INLINE_DATA_LEFT_MARGIN;

  uint16_t tmp = ctx->y;
  ctx->y = ctx->y - TH_LABEL_HEIGHT + ((TH_LABEL_HEIGHT - ctx->font->yAdvance) / 2);
  screen_draw_string(ctx, data);
  ctx->y = tmp;
}

static void dialog_chain(screen_text_ctx_t *ctx, const char* name) {
  dialog_label(ctx, LSTR(TX_CHAIN));
  dialog_inline_data(ctx, name);
}

static void dialog_tx_data(screen_text_ctx_t *ctx, i18n_str_id_t data_type) {
  dialog_label(ctx, LSTR(TX_DATA));
  dialog_inline_data(ctx, LSTR(data_type));
}

static void dialog_address(screen_text_ctx_t *ctx, i18n_str_id_t label, const uint8_t* addr) {
  char address[41];
  ethereum_address_checksum(addr, address);

  dialog_label(ctx, LSTR(label));
  dialog_data(ctx, address);
}

static void dialog_calculate_fees(bignum256* fees) {
  bignum256 gas_amount;
  bignum256 prime;
  bn_read_be(secp256k1.prime, &prime);
  bn_read_compact_be(g_ui_cmd.params.txn.tx->startgas.value, g_ui_cmd.params.txn.tx->startgas.length, &gas_amount);
  bn_read_compact_be(g_ui_cmd.params.txn.tx->gasprice.value, g_ui_cmd.params.txn.tx->gasprice.length, fees);
  bn_multiply(&gas_amount, fees, &prime);
}

static void dialog_amount(screen_text_ctx_t* ctx, i18n_str_id_t prompt, const bignum256* amount, int decimals, const char* ticker) {
  char tmp[BIGNUM_STRING_LEN+strlen(ticker)+2];
  bn_format(amount, NULL, ticker, decimals, 0, 0, ',', tmp, sizeof(tmp));

  dialog_label(ctx, LSTR(prompt));
  dialog_inline_data(ctx, tmp);
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
  i18n_str_id_t data_type;

  if (g_ui_cmd.params.txn.tx->dataType == DATA_ERC20) {
    title = TX_CONFIRM_ERC20_TITLE;
    token.chain = chain.chain_id;
    token.addr = g_ui_cmd.params.txn.tx->destination;

    to = g_ui_cmd.params.txn.tx->finalRecipient;

    if (eth_db_lookup_erc20(&token) != ERR_OK) {
      token.ticker = "???";
      token.decimals = 18;
    }

    data_type = TX_DATA_ERC20;
  } else {
    title = TX_CONFIRM_TITLE;
    token.ticker = chain.ticker;
    token.decimals = 18;
    to = g_ui_cmd.params.txn.tx->destination;
    data_type = g_ui_cmd.params.txn.tx->dataType == DATA_NONE ? TX_DATA_NONE : TX_DATA_PRESENT;
  }

  dialog_title(LSTR(title));

  screen_text_ctx_t ctx;
  ctx.y = TH_TITLE_HEIGHT;

  dialog_address(&ctx, TX_SIGNER, g_ui_cmd.params.txn.addr);
  dialog_address(&ctx, TX_ADDRESS, to);
  dialog_chain(&ctx, chain.name);
  dialog_tx_data(&ctx, data_type);

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

static void dialog_draw_message(const char* title, const char* txt, size_t len) {
  dialog_title(title);
  dialog_footer(TH_TITLE_HEIGHT);

  screen_text_ctx_t ctx = {
      .font = TH_FONT_TEXT,
      .fg = TH_COLOR_TEXT_FG,
      .bg = TH_COLOR_TEXT_BG,
      .x = TH_TEXT_HORIZONTAL_MARGIN,
      .y = TH_TITLE_HEIGHT + TH_TEXT_VERTICAL_MARGIN
  };

  screen_draw_text(&ctx, MESSAGE_MAX_X, MESSAGE_MAX_Y, (uint8_t*) txt, len, false);
}

static inline void _dialog_paged_title(const char* base, char title[MAX_MSG_TITLE_LEN], size_t page, size_t last_page) {
  size_t base_len = strlen(base);
  assert(base_len < (MAX_MSG_TITLE_LEN - 9));

  memcpy(title, base, base_len);
  title[base_len++] = ' ';
  title[base_len++] = '(';

  uint8_t page_str[4];
  uint8_t* p = u32toa(page + 1, page_str, 4);
  uint8_t p_len = strlen((char *) p);
  memcpy(&title[base_len], p, p_len);
  base_len += p_len;

  title[base_len++] = '/';

  p = u32toa(last_page + 1, page_str, 4);
  p_len = strlen((char *) p);
  memcpy(&title[base_len], p, p_len);
  base_len += p_len;
  title[base_len++] = ')';
  title[base_len] = '\0';
}

app_err_t dialog_confirm_msg() {
  size_t pages[MAX_PAGE_COUNT];
  size_t last_page = 0;
  pages[0] = 0;

  screen_text_ctx_t ctx = {
      .font = TH_FONT_TEXT,
      .fg = TH_COLOR_TEXT_FG,
      .bg = TH_COLOR_TEXT_BG,
  };

  while(1) {
    ctx.x = TH_TEXT_HORIZONTAL_MARGIN;
    ctx.y = last_page ? (TH_TITLE_HEIGHT + TH_TEXT_VERTICAL_MARGIN) : (TH_TITLE_HEIGHT + TH_DATA_HEIGHT + (TH_LABEL_HEIGHT * 2));

    size_t offset = pages[last_page];
    size_t to_display = g_ui_cmd.params.msg.len - offset;
    size_t remaining = screen_draw_text(&ctx, MESSAGE_MAX_X, MESSAGE_MAX_Y, &g_ui_cmd.params.msg.data[offset], to_display, true);

    if (!remaining || last_page == (MAX_PAGE_COUNT - 1)) {
      break;
    }

    pages[++last_page] = offset + (to_display - remaining);
  }

  int page = 0;

  while(1) {
    size_t offset = pages[page];
    char title[MAX_MSG_TITLE_LEN];
    _dialog_paged_title(LSTR(MSG_CONFIRM_TITLE), title, page, last_page);

    dialog_title(title);

    if (page == 0) {
      ctx.y = TH_TITLE_HEIGHT;
      dialog_address(&ctx, TX_SIGNER, g_ui_cmd.params.txn.addr);
      dialog_label(&ctx, LSTR(MSG_LABEL));
      ctx.font = TH_FONT_TEXT;
      ctx.fg = TH_COLOR_TEXT_FG;
      ctx.bg = TH_COLOR_TEXT_BG;
      ctx.x = TH_TEXT_HORIZONTAL_MARGIN;
      dialog_footer(ctx.y);
    } else {
      ctx.x = TH_TEXT_HORIZONTAL_MARGIN;
      ctx.y = TH_TITLE_HEIGHT + TH_TEXT_VERTICAL_MARGIN;
      dialog_footer(TH_TITLE_HEIGHT);
    }

    screen_draw_text(&ctx, MESSAGE_MAX_X, MESSAGE_MAX_Y, &g_ui_cmd.params.msg.data[offset], (g_ui_cmd.params.msg.len - offset), false);

    switch(ui_wait_keypress(pdMS_TO_TICKS(TX_CONFIRM_TIMEOUT))) {
    case KEYPAD_KEY_LEFT:
      if (page > 0) {
        page--;
      }
      break;
    case KEYPAD_KEY_RIGHT:
      if (page < last_page) {
        page++;
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

app_err_t dialog_confirm_eip712() {
  return ERR_OK;
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
  dialog_draw_message(g_ui_cmd.params.info.title, g_ui_cmd.params.info.msg, strlen(g_ui_cmd.params.info.msg));

  if (!g_ui_cmd.params.info.dismissable) {
    vTaskSuspend(NULL);
    return ERR_CANCEL;
  }

  return dialog_wait_dismiss();
}

app_err_t dialog_dev_auth() {
  if (g_ui_cmd.params.auth.auth_count > 1) {
    dialog_draw_message(LSTR(DEV_AUTH_TITLE_WARNING), LSTR(DEV_AUTH_INFO_WARNING), strlen(LSTR(DEV_AUTH_INFO_WARNING)));
  } else {
    dialog_draw_message(LSTR(DEV_AUTH_TITLE_SUCCESS), LSTR(DEV_AUTH_INFO_SUCCESS), strlen(LSTR(DEV_AUTH_INFO_SUCCESS)));
  }

  return dialog_wait_dismiss();
}
