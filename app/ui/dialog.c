#include "dialog.h"
#include "crypto/address.h"
#include "crypto/bignum.h"
#include "crypto/secp256k1.h"
#include "crypto/segwit_addr.h"
#include "crypto/script.h"
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
#define MESSAGE_MAX_Y (SCREEN_HEIGHT - TH_NAV_HINT_HEIGHT)

const uint8_t ETH_ERC20_SIGNATURE[] = { 0xa9, 0x05, 0x9c, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#define ETH_ERC20_SIGNATURE_LEN 16
#define ETH_ERC20_ADDR_OFF 16
#define ETH_ERC20_VALUE_OFF 36
#define ETH_ERC20_TRANSFER_LEN 68

#define BTC_DIALOG_PAGE_ITEMS 1

typedef enum {
  ETH_DATA_ABSENT,
  ETH_DATA_UNKNOWN,
  ETH_DATA_ERC20
} eth_data_type_t;

static app_err_t dialog_wait_dismiss() {
  dialog_nav_hints(0, ICON_NAV_NEXT);

  while(1) {
    switch(ui_wait_keypress(portMAX_DELAY)) {
    case KEYPAD_KEY_CONFIRM:
      return ERR_OK;
    default:
      break;
    }
  }
}

static app_err_t dialog_wait_dismiss_cancellable() {
  dialog_nav_hints(ICON_NAV_BACK, ICON_NAV_NEXT);

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

static app_err_t dialog_wait_paged(size_t* page, size_t last_page) {
  dialog_nav_hints(ICON_NAV_BACK, ICON_NAV_NEXT);
  dialog_pager(*page, last_page);

  switch(ui_wait_keypress(pdMS_TO_TICKS(TX_CONFIRM_TIMEOUT))) {
  case KEYPAD_KEY_LEFT:
    if (*page > 0) {
      (*page)--;
    } else {
      *page = last_page;
    }
    return ERR_NEED_MORE_DATA;
  case KEYPAD_KEY_RIGHT:
    if (*page < last_page) {
      (*page)++;
    } else {
      *page = 0;
    }
    return ERR_NEED_MORE_DATA;
  case KEYPAD_KEY_CANCEL:
  case KEYPAD_KEY_BACK:
  case KEYPAD_KEY_INVALID:
    return ERR_CANCEL;
  case KEYPAD_KEY_CONFIRM:
    return ERR_OK;
  default:
    return ERR_NEED_MORE_DATA;
  }
}

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

app_err_t dialog_footer_colors(uint16_t yOff, uint16_t bg) {
  screen_area_t area = { 0, yOff, SCREEN_WIDTH, SCREEN_HEIGHT - yOff };
  return screen_fill_area(&area, bg);
}

app_err_t dialog_nav_hints_colors(icons_t left, icons_t right, uint16_t bg, uint16_t fg) {
  screen_text_ctx_t ctx = {
      .bg = bg,
      .fg = fg,
      .font = TH_FONT_ICONS,
      .x = TH_NAV_HINT_LEFT_X,
      .y = TH_NAV_HINT_TOP
  };

  screen_area_t hint_area = {
      .x = 0,
      .y = SCREEN_HEIGHT - TH_NAV_HINT_HEIGHT,
      .width = TH_NAV_HINT_WIDTH,
      .height = TH_NAV_HINT_HEIGHT
  };

  screen_fill_area(&hint_area, bg);

  if (left != 0) {
    screen_draw_char(&ctx, left);
  }

  ctx.fg = TH_COLOR_ACCENT;
  ctx.x = TH_NAV_HINT_RIGHT_X;
  hint_area.x = SCREEN_WIDTH - TH_NAV_HINT_WIDTH;

  screen_fill_area(&hint_area, bg);

  if (right != 0) {
    screen_draw_char(&ctx, right);
  }

  return ERR_OK;
}

app_err_t dialog_pager(size_t page, size_t last_page) {
  uint8_t page_indicator[UINT32_STRING_LEN * 2];
  size_t total_len = 0;

  uint8_t page_str[UINT32_STRING_LEN];
  uint8_t* p = u32toa(page + 1, page_str, UINT32_STRING_LEN);
  uint8_t p_len = strlen((char *) p);
  memcpy(&page_indicator[total_len], p, p_len);
  total_len += p_len;

  page_indicator[total_len++] = '/';

  p = u32toa(last_page + 1, page_str, UINT32_STRING_LEN);
  p_len = strlen((char *) p);
  memcpy(&page_indicator[total_len], p, p_len);
  total_len += p_len;
  page_indicator[total_len] = '\0';

  screen_text_ctx_t ctx = {
      .x = 0,
      .y = SCREEN_HEIGHT - (TH_FONT_TITLE)->yAdvance,
      .font = TH_FONT_TITLE,
      .bg = TH_COLOR_BG,
      .fg = TH_COLOR_INACTIVE,
  };

  screen_draw_centered_string(&ctx, (char*) page_indicator);
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

static void dialog_address(screen_text_ctx_t *ctx, i18n_str_id_t label, addr_type_t addr_type, const uint8_t* addr) {
  char str[MAX_ADDR_LEN];
  address_format(addr_type, addr, str);

  dialog_label(ctx, LSTR(label));
  dialog_data(ctx, str);
}

static void dialog_calculate_fees(bignum256* fees) {
  bignum256 gas_amount;
  bignum256 prime;
  bn_read_be(secp256k1.prime, &prime);
  bn_read_compact_be(g_ui_cmd.params.eth_tx.tx->startgas.value, g_ui_cmd.params.eth_tx.tx->startgas.length, &gas_amount);
  bn_read_compact_be(g_ui_cmd.params.eth_tx.tx->gasprice.value, g_ui_cmd.params.eth_tx.tx->gasprice.length, fees);
  bn_multiply(&gas_amount, fees, &prime);
}

static void dialog_amount(screen_text_ctx_t* ctx, i18n_str_id_t prompt, const bignum256* amount, int decimals, const char* ticker) {
  char tmp[BIGNUM_STRING_LEN+strlen(ticker)+2];
  bn_format(amount, NULL, ticker, decimals, 0, 0, ',', tmp, sizeof(tmp));

  dialog_label(ctx, LSTR(prompt));
  dialog_inline_data(ctx, tmp);
}

static void dialog_btc_amount(screen_text_ctx_t* ctx, i18n_str_id_t prompt, uint64_t amount) {
  //TODO: better formatting, add ticker
  uint8_t tmp[UINT64_STRING_LEN];
  uint8_t* p = u64toa(amount, tmp, UINT64_STRING_LEN);

  dialog_label(ctx, LSTR(prompt));
  dialog_inline_data(ctx, (char*) p);
}

static void dialog_indexed_string(char* dst, const char* label, size_t index) {
  size_t seg_len = strlen(label);
  memcpy(dst, label, seg_len);
  dst += seg_len;
  *(dst++) = ' ';
  *(dst++) = '#';

  uint8_t tmp[11];
  uint8_t* digits = u32toa(index, tmp, 11);
  seg_len = strlen((char* ) digits);
  memcpy(dst, digits, seg_len);
  dst += seg_len;
  *dst = '\0';
}

// TODO: move this to more general function to recognize data and display correct data accordingly
static eth_data_type_t dialog_recognize_data(const txContent_t* tx) {
  if (tx->dataLength == 0) {
    return ETH_DATA_ABSENT;
  } else if (tx->value.length == 0 && tx->dataLength == ETH_ERC20_TRANSFER_LEN && !memcmp(tx->data, ETH_ERC20_SIGNATURE, ETH_ERC20_SIGNATURE_LEN)) {
    return ETH_DATA_ERC20;
  } else {
    return ETH_DATA_UNKNOWN;
  }
}

app_err_t dialog_confirm_eth_tx() {
  chain_desc_t chain;
  erc20_desc_t token;
  chain.chain_id = g_ui_cmd.params.eth_tx.tx->chainID;

  uint8_t num[11];
  if (eth_db_lookup_chain(&chain) != ERR_OK) {
    chain.name = (char*) u32toa(chain.chain_id, num, 11);
    chain.ticker = "???";
  }

  i18n_str_id_t title;
  const uint8_t* to;
  eth_data_type_t data_type = dialog_recognize_data(g_ui_cmd.params.eth_tx.tx);

  uint8_t* data = g_camera_fb[0];
  size_t data_len = 0;

  screen_text_ctx_t ctx;
  size_t pages[MAX_PAGE_COUNT];
  size_t page = 0;
  size_t last_page = 0;

  if (data_type == ETH_DATA_ERC20) {
    title = TX_CONFIRM_ERC20_TITLE;
    token.chain = chain.chain_id;
    token.addr = g_ui_cmd.params.eth_tx.tx->destination;

    memmove((uint8_t*) g_ui_cmd.params.eth_tx.tx->value.value, &g_ui_cmd.params.eth_tx.tx->data[ETH_ERC20_VALUE_OFF], INT256_LENGTH);
    ((txContent_t*) g_ui_cmd.params.eth_tx.tx)->value.length = INT256_LENGTH;

    to = &g_ui_cmd.params.eth_tx.tx->data[ETH_ERC20_ADDR_OFF];

    if (eth_db_lookup_erc20(&token) != ERR_OK) {
      token.ticker = "???";
      token.decimals = 18;
    }
  } else {
    title = TX_CONFIRM_TITLE;
    token.ticker = chain.ticker;
    token.decimals = 18;
    to = g_ui_cmd.params.eth_tx.tx->destination;

    if (data_type == ETH_DATA_UNKNOWN) {
      base16_encode(g_ui_cmd.params.eth_tx.tx->data, (char *) data, g_ui_cmd.params.eth_tx.tx->dataLength);
      data_len = g_ui_cmd.params.eth_tx.tx->dataLength * 2;

      last_page = 1;
      pages[1] = 0;
      ctx.font = TH_FONT_TEXT;

      while(1) {
        ctx.x = TH_TEXT_HORIZONTAL_MARGIN;

        if (last_page > 1) {
          ctx.y = TH_TITLE_HEIGHT + TH_TEXT_VERTICAL_MARGIN;
        } else {
          ctx.y = TH_TITLE_HEIGHT + TH_LABEL_HEIGHT;
        }

        size_t offset = pages[last_page];
        size_t to_display = data_len - offset;
        size_t remaining = screen_draw_text(&ctx, MESSAGE_MAX_X, MESSAGE_MAX_Y, &data[offset], to_display, true, false);

        if (!remaining || last_page == (MAX_PAGE_COUNT - 1)) {
          break;
        }

        pages[++last_page] = offset + (to_display - remaining);
      }
    }
  }

  bignum256 value;
  bn_read_compact_be(g_ui_cmd.params.eth_tx.tx->value.value, g_ui_cmd.params.eth_tx.tx->value.length, &value);

  bignum256 fees;
  dialog_calculate_fees(&fees);

  dialog_title(LSTR(title));

  app_err_t ret = ERR_NEED_MORE_DATA;

  while(ret == ERR_NEED_MORE_DATA) {
    ctx.y = TH_TITLE_HEIGHT;

    if (page == 0) {
      dialog_address(&ctx, TX_SIGNER, ADDR_ETH, g_ui_cmd.params.eth_tx.addr);
      dialog_address(&ctx, TX_ADDRESS, ADDR_ETH, to);
      dialog_chain(&ctx, chain.name);

      dialog_amount(&ctx, TX_AMOUNT, &value, token.decimals, token.ticker);
      dialog_amount(&ctx, TX_FEE, &fees, 18, chain.ticker);
      dialog_footer(ctx.y);
    } else {
      size_t offset = pages[page];

      if (page == 1) {
        dialog_label(&ctx, LSTR(TX_DATA));
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

      screen_draw_text(&ctx, MESSAGE_MAX_X, MESSAGE_MAX_Y, &data[offset], (data_len - offset), false, false);
    }

    ret = dialog_wait_paged(&page, last_page);
  }

  return ret;
}

void dialog_confirm_btc_summary(const btc_tx_ctx_t* tx) {
  screen_text_ctx_t ctx;
  ctx.y = TH_TITLE_HEIGHT;

  uint64_t total_input = 0;
  uint64_t signed_amount = 0;
  uint64_t total_output = 0;

  bool has_sighash_none = false;

  for (int i = 0; i < tx->input_count; i++) {
    uint64_t t;
    memcpy(&t, tx->input_data[i].amount, sizeof(uint64_t));
    total_input += t;
    if (tx->input_data[i].can_sign) {
      signed_amount += t;
    }

    if ((tx->input_data[i].sighash_flag & SIGHASH_MASK) == SIGHASH_NONE) {
      has_sighash_none = true;
    }
  }

  uint64_t change = 0;
  int dest_idx = -1;

  for (int i = 0; i < tx->output_count; i++) {
    uint64_t t;
    memcpy(&t, tx->outputs[i].amount, sizeof(uint64_t));
    total_output += t;
    if (tx->output_is_change[i]) {
      change += t;
    } else {
      if (dest_idx == -1) {
        dest_idx = i;
      } else {
        dest_idx = -2;
      }
    }
  }

  dialog_btc_amount(&ctx, TX_AMOUNT, total_input);

  if (total_input != signed_amount) {
    dialog_btc_amount(&ctx, TX_SIGNED_AMOUNT, signed_amount);
  }

  if (change) {
    dialog_btc_amount(&ctx, TX_CHANGE, change);
  }

  dialog_btc_amount(&ctx, TX_FEE, total_input - total_output);


  if (has_sighash_none) {
    ctx.x = TH_DATA_LEFT_MARGIN;
    screen_draw_text(&ctx, MESSAGE_MAX_X, MESSAGE_MAX_Y, (const uint8_t*) LSTR(TX_SIGHASH_WARNING), strlen(LSTR(TX_SIGHASH_WARNING)), false, false);
  } else {
    char buf[BIGNUM_STRING_LEN];
    dialog_label(&ctx, LSTR(TX_ADDRESS));

    if (dest_idx >= 0) {
      script_output_to_address(tx->outputs[dest_idx].script, tx->outputs[dest_idx].script_len, buf);
      dialog_data(&ctx, buf);
    } else {
      dialog_data(&ctx, LSTR(TX_MULTIPLE_RECIPIENT));
    }
  }
}

static inline void dialog_btc_sign_scheme_format(char* buf, uint32_t flag) {
  int off = 0;
  if (flag & SIGHASH_ANYONECANPAY) {
    const char* anyonecanpay = LSTR(TX_SIGN_ANYONECANPAY);
    off = strlen(anyonecanpay);
    memcpy(buf, LSTR(TX_SIGN_ANYONECANPAY), off);
    buf[off++] = ' ';
    buf[off++] = '|';
    buf[off++] = ' ';
  }

  const char* str;

  switch(flag & SIGHASH_MASK) {
  case SIGHASH_ALL:
    str = LSTR(TX_SIGN_ALL);
    break;
  case SIGHASH_NONE:
    str = LSTR(TX_SIGN_NONE);
    break;
  case SIGHASH_SINGLE:
    str = LSTR(TX_SIGN_SINGLE);
    break;
  default:
    str = "";
    break;
  }

  strcpy(&buf[off], str);
}

void dialog_confirm_btc_inouts(const btc_tx_ctx_t* tx, size_t page) {
  screen_text_ctx_t ctx;
  ctx.y = TH_TITLE_HEIGHT;
  size_t i = page * BTC_DIALOG_PAGE_ITEMS;
  size_t displayed = 0;
  char buf[BIGNUM_STRING_LEN];

  while((i < tx->input_count) && (displayed < BTC_DIALOG_PAGE_ITEMS)) {
    dialog_indexed_string(buf, LSTR(TX_INPUT), i);
    dialog_label(&ctx, buf);

    dialog_label(&ctx, LSTR(TX_ADDRESS));
    script_output_to_address(tx->input_data[i].script_pubkey, tx->input_data[i].script_pubkey_len, buf);
    dialog_data(&ctx, buf);

    uint64_t t;
    memcpy(&t, tx->input_data[i].amount, sizeof(uint64_t));
    dialog_btc_amount(&ctx, TX_AMOUNT, t);

    dialog_label(&ctx, LSTR(TX_SIGN_SCHEME));
    dialog_btc_sign_scheme_format(buf, tx->input_data[i].sighash_flag);
    dialog_inline_data(&ctx, buf);

    dialog_label(&ctx, LSTR(TX_SIGNED));
    dialog_inline_data(&ctx, tx->input_data[i].can_sign ? LSTR(TX_YES) : LSTR(TX_NO));

    i++;
    displayed++;
  }

  i -= tx->input_count;

  while ((i < tx->output_count) && (displayed < BTC_DIALOG_PAGE_ITEMS)) {
    dialog_indexed_string(buf, LSTR(TX_OUTPUT), i);
    dialog_label(&ctx, buf);

    dialog_label(&ctx, LSTR(TX_ADDRESS));
    script_output_to_address(tx->outputs[i].script, tx->outputs[i].script_len, buf);
    dialog_data(&ctx, buf);

    uint64_t t;
    memcpy(&t, tx->outputs[i].amount, sizeof(uint64_t));
    dialog_btc_amount(&ctx, TX_AMOUNT, t);

    dialog_label(&ctx, LSTR(TX_CHANGE));
    dialog_inline_data(&ctx, tx->output_is_change[i] ? LSTR(TX_YES) : LSTR(TX_NO));

    i++;
    displayed++;
  }
}

app_err_t dialog_confirm_bip322(const btc_tx_ctx_t* tx) {
  dialog_title(LSTR(MSG_CONFIRM_TITLE));
  dialog_footer(TH_TITLE_HEIGHT);

  screen_text_ctx_t ctx;
  ctx.y = TH_TITLE_HEIGHT;
  char buf[BIGNUM_STRING_LEN];

  dialog_label(&ctx, LSTR(TX_ADDRESS));
  script_output_to_address(tx->input_data[0].script_pubkey, tx->input_data[0].script_pubkey_len, buf);
  dialog_data(&ctx, buf);

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

app_err_t dialog_confirm_btc_tx() {
  const btc_tx_ctx_t* tx = g_ui_cmd.params.btc_tx.tx;

  if (btc_is_bip322(tx)) {
    return dialog_confirm_bip322(tx);
  }

  dialog_title(LSTR(TX_CONFIRM_TITLE));

  size_t page = 0;
  size_t last_page = ((tx->input_count + tx->output_count) + (BTC_DIALOG_PAGE_ITEMS - 1)) / BTC_DIALOG_PAGE_ITEMS;

  app_err_t ret = ERR_NEED_MORE_DATA;

  while(ret == ERR_NEED_MORE_DATA) {
    dialog_footer(TH_TITLE_HEIGHT);

    if (page == 0) {
      dialog_confirm_btc_summary(tx);
    } else {
      dialog_confirm_btc_inouts(tx, (page - 1));
    }

    ret = dialog_wait_paged(&page, last_page);
  }

  return ret;
}

static void dialog_draw_message(const char* txt) {
  dialog_title("");
  dialog_footer(TH_TITLE_HEIGHT);

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

  size_t page = 0;

  app_err_t ret = ERR_NEED_MORE_DATA;

  while(ret == ERR_NEED_MORE_DATA) {
    size_t offset = pages[page];

    dialog_title(LSTR(eip712 ? EIP712_CONFIRM_TITLE : MSG_CONFIRM_TITLE));

    if (page == 0) {
      ctx.y = TH_TITLE_HEIGHT;
      dialog_address(&ctx, TX_SIGNER, g_ui_cmd.params.msg.addr_type, g_ui_cmd.params.msg.addr);

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
    ret = dialog_wait_paged(&page, last_page);
  }

  return ret;
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

  return dialog_wait_dismiss_cancellable();
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
