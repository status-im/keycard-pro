#include <string.h>

#include "input.h"
#include "common.h"
#include "dialog.h"
#include "theme.h"
#include "crypto/bip39.h"
#include "crypto/util.h"
#include "crypto/rand.h"
#include "keypad/keypad.h"
#include "ui/ui.h"
#include "ui/ui_internal.h"

#define PIN_LEN 6
#define PUK_LEN 12
#define DIG_INV ' '

#define KEY_BACKSPACE 0x08
#define KEY_RETURN 0x0d
#define KEY_ESCAPE 0x1b

#define WORD_MAX_LEN 8
#define WORDS_TO_CONFIRM 4

#define KEYBOARD_TOP_Y (SCREEN_HEIGHT - (TH_KEYBOARD_KEY_SIZE * 3))

#define KEYBOARD_ROW1_LEN 10
#define KEYBOARD_ROW2_LEN 9
#define KEYBOARD_ROW3_LEN(__HAS_SPACE__) (7 + __HAS_SPACE__)

#define KEYBOARD_ROW1_LIMIT KEYBOARD_ROW1_LEN
#define KEYBOARD_ROW2_LIMIT (KEYBOARD_ROW1_LIMIT + KEYBOARD_ROW2_LEN)
#define KEYBOARD_ROW3_LIMIT(__HAS_SPACE__) (KEYBOARD_ROW2_LIMIT + KEYBOARD_ROW3_LEN(__HAS_SPACE__))

const char KEYPAD_TO_DIGIT[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', DIG_INV, '0', DIG_INV, DIG_INV, DIG_INV};
const char KEYBOARD_MAP[] = {
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ' '
};

static app_err_t input_render_secret(uint16_t yOff, int len, int pos) {
  uint16_t width = (len * (TH_PIN_FIELD_WIDTH + TH_PIN_FIELD_DIGIT_MARGIN)) - TH_PIN_FIELD_DIGIT_MARGIN;
  screen_area_t area = { .x = (SCREEN_WIDTH - width)/2, .y = yOff, .width = TH_PIN_FIELD_WIDTH, .height = TH_PIN_FIELD_HEIGHT};

  for (int i = 0; i < len; i++) {
    if (screen_fill_area(&area, (i < pos) ? TH_COLOR_PIN_FIELD_SELECTED_BG : TH_COLOR_PIN_FIELD_BG) != HAL_SUCCESS) {
      return ERR_HW;
    }

    area.x += TH_PIN_FIELD_WIDTH + TH_PIN_FIELD_DIGIT_MARGIN;
  }

  return ERR_OK;
}

app_err_t input_new_pin() {
  dialog_title(LSTR(PIN_CREATE_TITLE));
  dialog_footer(TH_TITLE_HEIGHT);

  screen_text_ctx_t ctx = {
      .bg = TH_COLOR_TEXT_BG,
      .fg = TH_COLOR_TEXT_FG,
      .font = TH_FONT_TEXT,
      .x = TH_LABEL_LEFT_MARGIN,
      .y = TH_TITLE_HEIGHT + TH_PIN_FIELD_HEIGHT + (TH_PIN_FIELD_VERTICAL_MARGIN * 2)
  };

  screen_draw_string(&ctx, LSTR(PIN_LABEL_REPEAT));

  ctx.y = TH_TITLE_HEIGHT + (TH_PIN_FIELD_HEIGHT * 2) + (TH_PIN_FIELD_VERTICAL_MARGIN * 4) + TH_LABEL_HEIGHT;

  screen_area_t mismatch_area = {
      .width = SCREEN_WIDTH,
      .height = ctx.font->yAdvance,
      .x = 0,
      .y = ctx.y
  };

  char* out = (char *) g_ui_cmd.params.input_pin.out;
  int8_t position = 0;
  char repeat[PIN_LEN];
  uint8_t matches = 0;

  while(1) {
    input_render_secret(TH_TITLE_HEIGHT + TH_PIN_FIELD_VERTICAL_MARGIN, PIN_LEN, position);
    input_render_secret(TH_TITLE_HEIGHT + TH_PIN_FIELD_HEIGHT + (TH_PIN_FIELD_VERTICAL_MARGIN * 3) + TH_LABEL_HEIGHT, PIN_LEN, APP_MAX(0, (position - PIN_LEN)));

    keypad_key_t key = ui_wait_keypress(portMAX_DELAY);

    if (key == KEYPAD_KEY_BACK) {
      if (position > 0) {
        position--;
      } else if (g_ui_cmd.params.input_pin.dismissable) {
        return ERR_CANCEL;
      }
    } else if (key == KEYPAD_KEY_CONFIRM) {
      if ((position == (PIN_LEN * 2)) && matches) {
        memset(repeat, 0, PIN_LEN);
        return ERR_OK;
      }
    } else if (position < (PIN_LEN * 2)) {
      char digit = KEYPAD_TO_DIGIT[key];
      if (digit != DIG_INV) {
        if (position < PIN_LEN) {
          out[position++] = digit;
        } else {
          repeat[(position++) - PIN_LEN] = digit;
        }
      }
    }

    matches = !strncmp(out, repeat, PIN_LEN);
    if (matches || (position <= PIN_LEN)) {
      screen_fill_area(&mismatch_area, TH_COLOR_TEXT_BG);
    } else {
      ctx.x = TH_LABEL_LEFT_MARGIN;
      screen_draw_string(&ctx, LSTR(PIN_LABEL_MISMATCH));
    }
  }
}

app_err_t input_pin() {
  if (g_ui_cmd.params.input_pin.retries == PIN_NEW_CODE) {
    return input_new_pin();
  }

  dialog_title(LSTR(PIN_INPUT_TITLE));
  dialog_footer(TH_TITLE_HEIGHT);

  screen_text_ctx_t ctx = {
      .bg = TH_COLOR_TEXT_BG,
      .fg = TH_COLOR_TEXT_FG,
      .font = TH_FONT_TEXT,
      .x = TH_LABEL_LEFT_MARGIN,
      .y = TH_TITLE_HEIGHT + TH_PIN_FIELD_HEIGHT + (TH_PIN_FIELD_VERTICAL_MARGIN * 2)
  };

  screen_draw_string(&ctx, LSTR(PIN_LABEL_REMAINING_ATTEMPTS));
  screen_draw_char(&ctx, (g_ui_cmd.params.input_pin.retries + '0'));

  char* out = (char *) g_ui_cmd.params.input_pin.out;
  uint8_t position = 0;

  while(1) {
    input_render_secret(TH_TITLE_HEIGHT + TH_PIN_FIELD_VERTICAL_MARGIN, PIN_LEN, position);
    keypad_key_t key = ui_wait_keypress(portMAX_DELAY);
    if (key == KEYPAD_KEY_BACK) {
      if (position > 0) {
        position--;
      } else if (g_ui_cmd.params.input_pin.dismissable) {
        return ERR_CANCEL;
      }
    } else if (key == KEYPAD_KEY_CONFIRM) {
      if (position == PIN_LEN) {
        return ERR_OK;
      }
    } else if (position < PIN_LEN) {
      char digit = KEYPAD_TO_DIGIT[key];
      if (digit != DIG_INV) {
        out[position++] = digit;
      }
    }
  }
}

app_err_t input_puk() {
  dialog_footer(TH_TITLE_HEIGHT);

  if (g_ui_cmd.params.input_pin.retries == PUK_NEW_CODE) {
    dialog_title(LSTR(PUK_CREATE_TITLE));
  } else {
    dialog_title(LSTR(PUK_INPUT_TITLE));
    screen_text_ctx_t ctx = {
        .bg = TH_COLOR_TEXT_BG,
        .fg = TH_COLOR_TEXT_FG,
        .font = TH_FONT_TEXT,
        .x = TH_LABEL_LEFT_MARGIN,
        .y = TH_TITLE_HEIGHT + (TH_PIN_FIELD_HEIGHT * 3) + (TH_PIN_FIELD_VERTICAL_MARGIN * 2) + (TH_PUK_FIELD_VERTICAL_MARGIN * 4)
    };

    screen_draw_string(&ctx, LSTR(PIN_LABEL_REMAINING_ATTEMPTS));
    screen_draw_char(&ctx, (g_ui_cmd.params.input_pin.retries + '0'));
  }

  char* out = (char *) g_ui_cmd.params.input_pin.out;
  uint8_t position = 0;

  while(1) {
    input_render_secret(TH_TITLE_HEIGHT + TH_PIN_FIELD_VERTICAL_MARGIN, 4, position);
    input_render_secret((TH_TITLE_HEIGHT + TH_PIN_FIELD_VERTICAL_MARGIN) + (TH_PIN_FIELD_HEIGHT + TH_PUK_FIELD_VERTICAL_MARGIN) , 4, position - 4);
    input_render_secret((TH_TITLE_HEIGHT + TH_PIN_FIELD_VERTICAL_MARGIN) + ((TH_PIN_FIELD_HEIGHT + TH_PUK_FIELD_VERTICAL_MARGIN) * 2), 4, position - 8);

    keypad_key_t key = ui_wait_keypress(portMAX_DELAY);
    if (key == KEYPAD_KEY_BACK) {
      if (position > 0) {
        position--;
      } else if (g_ui_cmd.params.input_pin.retries == PUK_NEW_CODE) {
        return ERR_CANCEL;
      }
    } else if (key == KEYPAD_KEY_CONFIRM) {
      if (position == PUK_LEN) {
        return ERR_OK;
      }
    } else if (position < PUK_LEN) {
      char digit = KEYPAD_TO_DIGIT[key];
      if (digit != DIG_INV) {
        out[position++] = digit;
      }
    }
  }
}

static inline void input_keyboard_render_key(char c, uint16_t x, uint16_t y, bool selected) {
  screen_area_t key_area = { .x = x, .y = y, .width = TH_KEYBOARD_KEY_SIZE, .height = TH_KEYBOARD_KEY_SIZE };
  screen_text_ctx_t ctx = { .font = TH_FONT_TEXT, .fg = TH_COLOR_TEXT_FG, .y = y };

  const glyph_t* glyph = screen_lookup_glyph(ctx.font, c);
  ctx.bg = selected ? TH_KEYBOARD_KEY_SELECTED_BG : TH_KEYBOARD_KEY_BG;
  ctx.x = x + ((TH_KEYBOARD_KEY_SIZE - glyph->width) / 2);

  screen_fill_area(&key_area, ctx.bg);
  screen_draw_glyph(&ctx, glyph);
}

static inline void input_keyboard_render(int idx, bool show_space) {
  int i = 0;

  while (i < KEYBOARD_ROW1_LIMIT) {
    input_keyboard_render_key(KEYBOARD_MAP[i], (i * TH_KEYBOARD_KEY_SIZE), KEYBOARD_TOP_Y, idx == i);
    i++;
  }

  while (i < KEYBOARD_ROW2_LIMIT) {
    input_keyboard_render_key(KEYBOARD_MAP[i], ((i - 10) * TH_KEYBOARD_KEY_SIZE), (KEYBOARD_TOP_Y + TH_KEYBOARD_KEY_SIZE), idx == i);
    i++;
  }

  screen_area_t padding = {
      .x = KEYBOARD_ROW2_LEN * TH_KEYBOARD_KEY_SIZE,
      .y = KEYBOARD_TOP_Y + TH_KEYBOARD_KEY_SIZE,
      .width = TH_KEYBOARD_KEY_SIZE,
      .height = TH_KEYBOARD_KEY_SIZE
  };

  screen_fill_area(&padding, TH_KEYBOARD_KEY_BG);

  while (i < KEYBOARD_ROW3_LIMIT(show_space)) {
    input_keyboard_render_key(KEYBOARD_MAP[i], ((i - 19) * TH_KEYBOARD_KEY_SIZE), (KEYBOARD_TOP_Y + (TH_KEYBOARD_KEY_SIZE * 2)), idx == i);
    i++;
  }

  padding.x = KEYBOARD_ROW3_LEN(show_space) * TH_KEYBOARD_KEY_SIZE;
  padding.y = KEYBOARD_TOP_Y + (TH_KEYBOARD_KEY_SIZE * 2);
  padding.width = (TH_KEYBOARD_KEY_SIZE * 3);

  screen_fill_area(&padding, TH_KEYBOARD_KEY_BG);
}

static char input_keyboard(int *idx, bool show_space) {
  while(1) {
    input_keyboard_render(*idx, show_space);

    switch(ui_wait_keypress(portMAX_DELAY)) {
    case KEYPAD_KEY_UP:
      if (*idx >= KEYBOARD_ROW2_LIMIT) {
        *idx -= KEYBOARD_ROW2_LEN;
      } else if (*idx >= KEYBOARD_ROW1_LIMIT) {
        *idx -= KEYBOARD_ROW1_LEN;
      } else {
        *idx = APP_MIN(*idx + KEYBOARD_ROW2_LIMIT, (KEYBOARD_ROW3_LIMIT(show_space) - 1));
      }
      break;
    case KEYPAD_KEY_LEFT:
      if ((*idx > KEYBOARD_ROW2_LIMIT) ||
          ((*idx > KEYBOARD_ROW1_LIMIT) && (*idx < KEYBOARD_ROW2_LIMIT)) ||
          ((*idx > 0) && (*idx < KEYBOARD_ROW1_LIMIT))) {
        (*idx)--;
      } else {
        if (*idx == 0) {
          *idx = KEYBOARD_ROW1_LIMIT - 1;
        } else if (*idx == KEYBOARD_ROW1_LIMIT) {
          *idx = KEYBOARD_ROW2_LIMIT - 1;
        } else {
          *idx = KEYBOARD_ROW3_LIMIT(show_space) - 1;
        }
      }
      break;
    case KEYPAD_KEY_RIGHT:
      if ((*idx < (KEYBOARD_ROW1_LIMIT - 1)) ||
          ((*idx < (KEYBOARD_ROW2_LIMIT - 1)) && (*idx >= KEYBOARD_ROW1_LIMIT)) ||
          ((*idx < (KEYBOARD_ROW3_LIMIT(show_space) - 1)) && (*idx >= KEYBOARD_ROW2_LIMIT))) {
        (*idx)++;
      }  else {
        if (*idx == KEYBOARD_ROW1_LIMIT - 1) {
          *idx = 0;
        } else if (*idx == KEYBOARD_ROW2_LIMIT - 1) {
          *idx = KEYBOARD_ROW1_LIMIT;
        } else {
          *idx = KEYBOARD_ROW2_LIMIT;
        }
      }
      break;
    case KEYPAD_KEY_DOWN:
      if (*idx < KEYBOARD_ROW1_LIMIT) {
        *idx = APP_MIN(*idx + KEYBOARD_ROW1_LEN, (KEYBOARD_ROW2_LIMIT - 1));
      } else if (*idx < KEYBOARD_ROW2_LIMIT) {
        *idx = APP_MIN(*idx + KEYBOARD_ROW2_LEN, (KEYBOARD_ROW3_LIMIT(show_space) - 1));
      } else {
        *idx -= KEYBOARD_ROW2_LIMIT;
      }
      break;
    case KEYPAD_KEY_BACK:
      return g_ui_ctx.keypad.last_key_long ? KEY_ESCAPE : KEY_BACKSPACE;
    case KEYPAD_KEY_CONFIRM:
      return g_ui_ctx.keypad.last_key_long ? KEY_RETURN : KEYBOARD_MAP[*idx];
    default:
      break;
    }
  }
}

static void input_render_text_field(const char* str, screen_area_t* field_area, int len, int suggestion_len) {
  screen_text_ctx_t ctx = {
      .font = TH_FONT_TEXT,
      .fg = TH_TEXT_FIELD_FG,
      .bg = TH_TEXT_FIELD_BG,
      .x = field_area->x + TH_TEXT_FIELD_INNER_LEFT_MARGIN,
      .y = field_area->y
  };

  screen_fill_area(field_area, ctx.bg);

  screen_draw_chars(&ctx, str, len);
  ctx.fg = TH_TEXT_FIELD_SUGGESTION_FG;
  screen_draw_chars(&ctx, &str[len], suggestion_len);
}

static void input_mnemonic_title(uint8_t i) {
  const char* base_title = LSTR(MNEMO_WORD_TITLE);
  int base_len = strlen(base_title);
  int buf_len = base_len + 4;
  uint8_t title_buf[buf_len];
  char* title = (char *) u32toa(i + 1, title_buf, buf_len);
  title -= base_len;
  memcpy(title, base_title, base_len);
  dialog_title(title);
}

static void input_render_editable_text_field(const char* str, int len, int suggestion_len) {
  screen_area_t field_area = {
      .x = TH_TEXT_FIELD_MARGIN,
      .y = TH_TITLE_HEIGHT + TH_TEXT_FIELD_MARGIN,
      .width = SCREEN_WIDTH - (TH_TEXT_FIELD_MARGIN * 2),
      .height = TH_TEXT_FIELD_HEIGHT
  };

  input_render_text_field(str, &field_area, len, suggestion_len);
}

static void input_mnemonic_render(const char* word, int len, uint16_t idx) {
  int suggestion_len;

  if (idx != UINT16_MAX) {
    word = BIP39_WORDLIST_ENGLISH[idx];
    suggestion_len = strlen(word) - len;
  } else {
    suggestion_len = 0;
  }

  input_render_editable_text_field(word, len, suggestion_len);
}

static uint16_t input_mnemonic_lookup(char* word, int len, uint16_t idx) {
  if (len == 0) {
    return UINT16_MAX;
  }

  while (idx < BIP39_WORD_COUNT) {
    int cmp = strncmp(word, BIP39_WORDLIST_ENGLISH[idx], len);

    if (cmp == 0) {
      return idx;
    } else if (cmp < 0) {
      return UINT16_MAX;
    } else {
      idx++;
    }
  }

  return UINT16_MAX;
}

static app_err_t input_mnemonic_get_word(int i, uint16_t* idx) {
  input_mnemonic_title(i);
  dialog_footer(TH_TITLE_HEIGHT);

  char word[WORD_MAX_LEN];
  int len = 0;
  int key_idx = 0;

  while(1) {
    input_mnemonic_render(word, len, *idx);
    char c = input_keyboard(&key_idx, false);

    if (c == KEY_RETURN) {
      if (*idx != UINT16_MAX) {
        return ERR_OK;
      }
    } else if (c == KEY_BACKSPACE) {
      if (len > 0) {
        len--;
        *idx = input_mnemonic_lookup(word, len, 0);
      }
    } else if (c == KEY_ESCAPE) {
      return ERR_CANCEL;
    } else if (len < WORD_MAX_LEN) {
      word[len++] = c;
      *idx = input_mnemonic_lookup(word, len, ((*idx) == UINT16_MAX ? 0 : *idx));
    }
  }
}

app_err_t input_mnemonic() {
  dialog_footer(TH_TITLE_HEIGHT);

  memset(g_ui_cmd.params.mnemo.indexes, 0xff, (sizeof(uint16_t) * g_ui_cmd.params.mnemo.len));

  int i = 0;

  while (i < g_ui_cmd.params.mnemo.len) {
    app_err_t err = input_mnemonic_get_word(i, &g_ui_cmd.params.mnemo.indexes[i]);

    if (err == ERR_OK) {
      i++;
    } else if (i > 0){
      i--;
    } else {
      return ERR_CANCEL;
    }
  }

  return ERR_OK;
}

static app_err_t input_backup_show_mnemonic() {
  dialog_title(LSTR(MNEMO_BACKUP_TITLE));
  dialog_footer(TH_TITLE_HEIGHT);

  //TODO: change if we need to support different lengths
  if (g_ui_cmd.params.mnemo.len != 12) {
    return ERR_CANCEL;
  }

  screen_area_t field_area = {
      .y = TH_TITLE_HEIGHT + TH_MNEMONIC_TOP_MARGIN,
      .width = TH_MNEMONIC_FIELD_WIDTH,
      .height = TH_TEXT_FIELD_HEIGHT
  };

  for (int i = 0; i < 4; i++) {
    field_area.x = TH_MNEMONIC_LEFT_MARGIN;

    for (int j = 0; j < 3; j++) {
      const char* word = BIP39_WORDLIST_ENGLISH[g_ui_cmd.params.mnemo.indexes[(i * 3) + j]];
      input_render_text_field(word, &field_area, strlen(word), 0);
      field_area.x += TH_MNEMONIC_FIELD_WIDTH + TH_MNEMONIC_LEFT_MARGIN;
    }

    field_area.y += TH_TEXT_FIELD_HEIGHT + TH_MNEMONIC_TOP_MARGIN;
  }

  while(1) {
    switch(ui_wait_keypress(portMAX_DELAY)) {
    case KEYPAD_KEY_CANCEL:
      return ERR_CANCEL;
    case KEYPAD_KEY_BACK:
      if (g_ui_ctx.keypad.last_key_long) {
        return ERR_CANCEL;
      }
      break;
    case KEYPAD_KEY_CONFIRM:
      return ERR_OK;
    default:
      break;
    }
  }
}

static app_err_t input_backup_confirm_mnemonic(uint8_t positions[WORDS_TO_CONFIRM]) {
  dialog_footer(TH_TITLE_HEIGHT);

  int i = 0;

  while (i < WORDS_TO_CONFIRM) {
    uint16_t idx = UINT16_MAX;
    app_err_t err = input_mnemonic_get_word(positions[i], &idx);

    if (err == ERR_OK) {
      if (idx == g_ui_cmd.params.mnemo.indexes[positions[i]]) {
        i++;
      } else {
        dialog_internal_info(LSTR(INFO_ERROR_TITLE), LSTR(MNENO_MISMATCH));
      }
    } else if (i > 0) {
      i--;
    } else {
      return ERR_CANCEL;
    }
  }

  return ERR_OK;
}

app_err_t input_backup_mnemonic() {
  uint8_t positions[WORDS_TO_CONFIRM];

  do {
    if (input_backup_show_mnemonic() == ERR_CANCEL) {
      return ERR_CANCEL;
    }

    random_unique_in_range(g_ui_cmd.params.mnemo.len, WORDS_TO_CONFIRM, positions);
  } while(input_backup_confirm_mnemonic(positions) != ERR_OK);

  return ERR_OK;
}

app_err_t input_string() {
  dialog_title(g_ui_cmd.params.input_string.title);
  dialog_footer(TH_TITLE_HEIGHT);

  int len = 0;
  int key_idx = 0;

  while(1) {
    input_render_editable_text_field(g_ui_cmd.params.input_string.out, len, 0);
    char c = input_keyboard(&key_idx, true);

    if (c == KEY_RETURN) {
      g_ui_cmd.params.input_string.out[len] = '\0';
      *g_ui_cmd.params.input_string.len = len;
      return ERR_OK;
    } else if (c == KEY_BACKSPACE) {
      if (len > 0) {
        len--;
      }
    } else if (c == KEY_ESCAPE) {
      return ERR_CANCEL;
    } else if (len < *g_ui_cmd.params.input_string.len) {
      g_ui_cmd.params.input_string.out[len++] = c;
    }
  }
}
