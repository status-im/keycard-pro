#include <string.h>

#include "input.h"
#include "common.h"
#include "dialog.h"
#include "theme.h"
#include "ui/ui.h"
#include "ui/ui_internal.h"

#define PIN_LEN 6
#define DIG_INV ' '

const char KEYPAD_TO_DIGIT[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', DIG_INV, '0', DIG_INV, DIG_INV, DIG_INV};

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
