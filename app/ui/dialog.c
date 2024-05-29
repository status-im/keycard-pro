#include "dialog.h"
#include "crypto/address.h"
#include "crypto/bignum.h"
#include "crypto/secp256k1.h"
#include "crypto/util.h"
#include "ethereum/eth_db.h"
#include "ethereum/ethUstream.h"
#include "ethereum/ethUtils.h"
#include "mem.h"
#include "theme.h"
#include "ui/ui_internal.h"

#define TX_CONFIRM_TIMEOUT 30000
#define BIGNUM_STRING_LEN 84
#define MAX_PAGE_COUNT 50
#define MESSAGE_MAX_X (SCREEN_WIDTH - TH_TEXT_HORIZONTAL_MARGIN)
#define MESSAGE_MAX_Y (SCREEN_HEIGHT - TH_TEXT_VERTICAL_MARGIN)
#define MAX_MSG_TITLE_LEN 80

const uint8_t ETH_ERC20_SIGNATURE[] = { 0xa9, 0x05, 0x9c, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#define ETH_ERC20_SIGNATURE_LEN 16
#define ETH_ERC20_ADDR_OFF 16
#define ETH_ERC20_VALUE_OFF 36
#define ETH_ERC20_TRANSFER_LEN 68

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

app_err_t dialog_title_colors(const char* title, uint16_t bg, uint16_t fg, uint16_t icon) {
  screen_text_ctx_t ctx = { TH_FONT_TITLE, fg, bg, TH_TITLE_LEFT_MARGIN, 0 };
  if (dialog_line(&ctx, title, TH_TITLE_HEIGHT) != ERR_OK) {
    return ERR_HW;
  }

  ctx.font = TH_FONT_ICONS;
  ctx.fg = icon;
  ctx.y = 0;
  ctx.x = TH_TITLE_ICON_POSITION;

  uint8_t i = g_ui_ctx.battery / 25;
  if (i > 5) {
    i = 4;
  } else if (i >= 4) {
    i = 3;
  }

  return screen_draw_glyph(&ctx, &ctx.font->glyph[i]) == HAL_SUCCESS? ERR_OK : ERR_HW;
}

app_err_t dialog_title(const char* title) {
  return dialog_title_colors(title, TH_COLOR_TITLE_BG, TH_COLOR_TITLE_FG, TH_COLOR_TITLE_ICON_FG);
}

app_err_t dialog_footer(uint16_t yOff) {
  screen_area_t area = { 0, yOff, SCREEN_WIDTH, SCREEN_HEIGHT - yOff };
  return screen_fill_area(&area, TH_COLOR_BG);
}

app_err_t dialog_nav_hints(icons_t left, icons_t right) {
  screen_text_ctx_t ctx = {
      .bg = TH_COLOR_BG,
      .fg = TH_COLOR_FG,
      .font = TH_FONT_ICONS,
      .x = TH_DEF_LEFT_MARGIN,
      .y = SCREEN_HEIGHT - TH_DEF_LEFT_MARGIN - 28
  };

  if (left != 0) {
    screen_draw_char(&ctx, left);
  }

  ctx.fg = TH_COLOR_ACCENT;
  ctx.x = SCREEN_WIDTH - 28 - TH_DEF_LEFT_MARGIN;

  if (right != 0) {
    screen_draw_char(&ctx, right);
  }

  return ERR_OK;
}

app_err_t dialog_margin(uint16_t yOff, uint16_t height) {
  screen_area_t area = { 0, yOff, SCREEN_WIDTH, height };
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

// TODO: move this to more general function to recognize data and display correct data accordingly
static i18n_str_id_t dialog_recognize_data(const txContent_t* tx) {
  if (tx->dataLength == 0) {
    return TX_DATA_NONE;
  } else if (tx->value.length == 0 && tx->dataLength == ETH_ERC20_TRANSFER_LEN && !memcmp(tx->data, ETH_ERC20_SIGNATURE, ETH_ERC20_SIGNATURE_LEN)) {
    return TX_DATA_ERC20;
  } else {
    return TX_DATA_PRESENT;
  }
}

app_err_t dialog_confirm_tx() {
  chain_desc_t chain;
  erc20_desc_t token;
  chain.chain_id = g_ui_cmd.params.txn.tx->chainID;

  uint8_t num[11];
  if (eth_db_lookup_chain(&chain) != ERR_OK) {
    chain.name = (char*) u32toa(chain.chain_id, num, 11);
    chain.ticker = "???";
  }

  i18n_str_id_t title;
  const uint8_t* to;
  i18n_str_id_t data_type = dialog_recognize_data(g_ui_cmd.params.txn.tx);

  if (data_type == TX_DATA_ERC20) {
    title = TX_CONFIRM_ERC20_TITLE;
    token.chain = chain.chain_id;
    token.addr = g_ui_cmd.params.txn.tx->destination;

    memmove((uint8_t*) g_ui_cmd.params.txn.tx->value.value, &g_ui_cmd.params.txn.tx->data[ETH_ERC20_VALUE_OFF], INT256_LENGTH);
    ((txContent_t*) g_ui_cmd.params.txn.tx)->value.length = INT256_LENGTH;

    to = &g_ui_cmd.params.txn.tx->data[ETH_ERC20_ADDR_OFF];

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
  dialog_nav_hints(ICON_NAV_BACK, ICON_NAV_NEXT);

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

static void dialog_draw_message(const char* txt) {
  dialog_title("");
  dialog_footer(TH_TITLE_HEIGHT);
  dialog_nav_hints(0, ICON_NAV_NEXT);

  screen_text_ctx_t ctx = {
      .font = TH_FONT_TEXT,
      .fg = TH_COLOR_TEXT_FG,
      .bg = TH_COLOR_TEXT_BG,
      .x = TH_TEXT_HORIZONTAL_MARGIN,
      .y = 0
  };

  size_t len = strlen(txt);
  screen_draw_text(&ctx, MESSAGE_MAX_X, SCREEN_HEIGHT, (uint8_t*) txt, len, true, true);
  ctx.x = TH_TEXT_HORIZONTAL_MARGIN;
  ctx.y = (SCREEN_HEIGHT - ctx.y) / 2;
  screen_draw_text(&ctx, MESSAGE_MAX_X, SCREEN_HEIGHT, (uint8_t*) txt, len, false, true);
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

app_err_t dialog_confirm_text_based(const uint8_t* data, size_t len, eip712_domain_t* eip712) {
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

    if (last_page > 0) {
      ctx.y = TH_TITLE_HEIGHT + TH_TEXT_VERTICAL_MARGIN;
    } else if (eip712) {
      ctx.y = TH_TITLE_HEIGHT + (TH_DATA_HEIGHT * 4) + (TH_LABEL_HEIGHT * 4);
    } else {
      ctx.y = TH_TITLE_HEIGHT + (TH_DATA_HEIGHT * 1) + (TH_LABEL_HEIGHT * 2);
    }

    size_t offset = pages[last_page];
    size_t to_display = len - offset;
    size_t remaining = screen_draw_text(&ctx, MESSAGE_MAX_X, MESSAGE_MAX_Y, &data[offset], to_display, true, false);

    if (!remaining || last_page == (MAX_PAGE_COUNT - 1)) {
      break;
    }

    pages[++last_page] = offset + (to_display - remaining);
  }

  int page = 0;

  while(1) {
    size_t offset = pages[page];
    char title[MAX_MSG_TITLE_LEN];
    _dialog_paged_title(LSTR(eip712 ? EIP712_CONFIRM_TITLE : MSG_CONFIRM_TITLE), title, page, last_page);

    dialog_title(title);

    if (page == 0) {
      ctx.y = TH_TITLE_HEIGHT;
      dialog_address(&ctx, TX_SIGNER, g_ui_cmd.params.msg.addr);

      if (eip712) {
        dialog_label(&ctx, LSTR(TX_CHAIN));

        chain_desc_t chain;
        chain.chain_id = eip712->chainID;

        uint8_t num[11];
        if (eth_db_lookup_chain(&chain) != ERR_OK) {
          chain.name = (char*) u32toa(chain.chain_id, num, 11);
        }

        dialog_data(&ctx, chain.name);
        dialog_label(&ctx, LSTR(EIP712_NAME));
        dialog_data(&ctx, eip712->name);
        dialog_label(&ctx, LSTR(EIP712_CONTRACT));
        dialog_data(&ctx, eip712->address);
      } else {
        dialog_label(&ctx, LSTR(MSG_LABEL));
      }

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

    screen_draw_text(&ctx, MESSAGE_MAX_X, MESSAGE_MAX_Y, &data[offset], (len - offset), false, false);
    dialog_nav_hints(ICON_NAV_BACK, ICON_NAV_NEXT);

    switch(ui_wait_keypress(pdMS_TO_TICKS(TX_CONFIRM_TIMEOUT))) {
    case KEYPAD_KEY_LEFT:
      if (page > 0) {
        page--;
      } else {
        page = last_page;
      }
      break;
    case KEYPAD_KEY_RIGHT:
      if (page < last_page) {
        page++;
      } else {
        page = 0;
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

app_err_t dialog_confirm_msg() {
  return dialog_confirm_text_based(g_ui_cmd.params.msg.data, g_ui_cmd.params.msg.len, NULL);
}

app_err_t dialog_confirm_eip712() {
  size_t len = eip712_to_string(g_ui_cmd.params.eip712.data, g_camera_fb[0]);
  eip712_domain_t domain;
  eip712_extract_domain(g_ui_cmd.params.eip712.data, &domain);
  return dialog_confirm_text_based(g_camera_fb[0], len, &domain);
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

app_err_t dialog_internal_info(const char* msg) {
  dialog_draw_message(msg);
  return dialog_wait_dismiss();
}

app_err_t dialog_info() {
  dialog_draw_message(g_ui_cmd.params.info.msg);

  if (!g_ui_cmd.params.info.dismissable) {
    vTaskSuspend(NULL);
    return ERR_CANCEL;
  }

  return dialog_wait_dismiss();
}

app_err_t dialog_prompt() {
  dialog_title(g_ui_cmd.params.prompt.title);
  dialog_footer(TH_TITLE_HEIGHT);

  screen_text_ctx_t ctx = {
      .font = TH_FONT_TEXT,
      .fg = TH_COLOR_TEXT_FG,
      .bg = TH_COLOR_TEXT_BG,
      .x = TH_TEXT_HORIZONTAL_MARGIN,
      .y = TH_TITLE_HEIGHT + TH_TEXT_VERTICAL_MARGIN
  };

  size_t len = strlen(g_ui_cmd.params.prompt.msg);
  screen_draw_text(&ctx, MESSAGE_MAX_X, MESSAGE_MAX_Y, (uint8_t*) g_ui_cmd.params.prompt.msg, len, false, false);

  return dialog_wait_dismiss();
}

app_err_t dialog_dev_auth() {
  if (g_ui_cmd.params.auth.auth_count > 1) {
    dialog_draw_message(LSTR(DEV_AUTH_INFO_WARNING));
  } else {
    dialog_draw_message(LSTR(DEV_AUTH_INFO_SUCCESS));
  }

  return dialog_wait_dismiss();
}

app_err_t dialog_wrong_auth() {
  dialog_title("");
  dialog_footer(TH_TITLE_HEIGHT);
  dialog_nav_hints(0, ICON_NAV_NEXT);

  screen_text_ctx_t ctx = {
      .font = TH_FONT_TEXT,
      .fg = TH_COLOR_TEXT_FG,
      .bg = TH_COLOR_TEXT_BG,
      .x = 0,
      .y = (SCREEN_HEIGHT - ((TH_FONT_TEXT)->yAdvance + TH_TEXT_VERTICAL_MARGIN + (TH_FONT_TEXT)->yAdvance)) / 2
  };

  screen_draw_centered_string(&ctx, g_ui_cmd.params.wrong_auth.msg);

  ctx.x = 0;
  ctx.y += TH_TEXT_VERTICAL_MARGIN;
  ctx.fg = TH_COLOR_ERROR;

  size_t label_len = strlen(LSTR(PIN_LABEL_REMAINING_ATTEMPTS));
  char remaining_attempts[label_len + 2];
  memcpy(remaining_attempts, LSTR(PIN_LABEL_REMAINING_ATTEMPTS), label_len);
  remaining_attempts[label_len] = g_ui_cmd.params.wrong_auth.retries + '0';
  remaining_attempts[label_len + 1] = '\0';

  screen_draw_centered_string(&ctx, remaining_attempts);

  return dialog_wait_dismiss();
}
