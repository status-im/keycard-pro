#include <stddef.h>

#include "common.h"
#include "dialog.h"
#include "menu.h"
#include "keypad/keypad.h"
#include "screen/screen.h"
#include "theme.h"
#include "ui_internal.h"

const menu_t menu_keycard = {
  5, {
    {MENU_CARD_NAME, NULL},
    {MENU_CHANGE_PIN, NULL},
    {MENU_CHANGE_PUK, NULL},
    {MENU_CHANGE_PAIRING, NULL},
    {MENU_RESET_CARD, NULL},
  }
};

const menu_t menu_device = {
  3, {
    {MENU_INFO, NULL},
    {MENU_DEV_AUTH, NULL},
    {MENU_DB_UPDATE, NULL},
  }
};

const menu_t menu_settings = {
  3, {
    {MENU_BRIGHTNESS, NULL},
    {MENU_SET_OFF_TIME, NULL},
    {MENU_USB, NULL},
  }
};

const menu_t menu_mainmenu = {
  5, {
    {MENU_QRCODE, NULL},
    {MENU_DISPLAY_PUBLIC, NULL},
    {MENU_KEYCARD, &menu_keycard},
    {MENU_DEVICE, &menu_device},
    {MENU_SETTINGS, &menu_settings},
  }
};

const menu_t menu_mnemonic = {
  4, {
    {MENU_MNEMO_12WORDS, NULL},
    {MENU_MNEMO_18WORDS, NULL},
    {MENU_MNEMO_24WORDS, NULL},
    {MENU_MNEMO_GENERATE, NULL},
  }
};

const menu_t menu_autooff = {
  5, {
    {MENU_OFF_3MINS, NULL},
    {MENU_OFF_5MINS, NULL},
    {MENU_OFF_10MINS, NULL},
    {MENU_OFF_30MINS, NULL},
    {MENU_OFF_NEVER, NULL},
  }
};

const menu_t menu_onoff = {
  2, {
    {MENU_ON, NULL},
    {MENU_OFF, NULL},
  }
};

#define MENU_MAX_DEPTH 3

void menu_render_entry(const menu_entry_t* entry, uint8_t is_selected, uint16_t yOff) {
  screen_text_ctx_t ctx;
  ctx.font = TH_FONT_MENU;

  if (is_selected) {
    ctx.bg = TH_COLOR_MENU_SELECTED_BG;
    ctx.fg = TH_COLOR_MENU_SELECTED_FG;
  } else {
    ctx.bg = TH_COLOR_MENU_BG;
    ctx.fg = TH_COLOR_MENU_FG;
  }

  ctx.x = TH_MENU_LEFT_MARGIN;
  ctx.y = yOff;

  dialog_line(&ctx, LSTR(entry->label_id), (TH_MENU_HEIGHT - TH_SEP_HEIGHT));
  dialog_separator(yOff + (TH_MENU_HEIGHT - TH_SEP_HEIGHT));
}

enum menu_draw_mode {
  MENU_ALL,
  MENU_NEXT,
  MENU_PREV,
  MENU_NONE
};

void menu_render(const menu_t* menu, const char* title, uint8_t selected, enum menu_draw_mode mode) {
  uint16_t yOff = TH_TITLE_HEIGHT;

  int i, l;

  switch(mode) {
  case MENU_ALL:
    dialog_title(title);
    i = 0;
    l = menu->len;
    break;
  case MENU_NEXT:
    i = APP_MAX(0, selected);
    l = APP_MIN(menu->len, (selected + 2));
    yOff += (i * TH_MENU_HEIGHT);
    break;
  case MENU_PREV:
    i = APP_MAX(0, (selected - 1));
    l = APP_MIN(menu->len, (selected + 1));
    yOff += (i * TH_MENU_HEIGHT);
    break;
  case MENU_NONE:
  default:
    return;
  }

  for (; i < l; i++) {
    menu_render_entry(&menu->entries[i], i == selected, yOff);
    yOff += TH_MENU_HEIGHT;
  }

  if (mode == MENU_ALL) {
    dialog_footer(yOff);
  }
}

app_err_t menu_run() {
  const menu_t* menus[MENU_MAX_DEPTH];
  const char* titles[MENU_MAX_DEPTH];

  uint8_t depth = 0;
  enum menu_draw_mode draw = MENU_ALL;
  menus[depth] = g_ui_cmd.params.menu.menu;
  titles[depth] = g_ui_cmd.params.menu.title;

  uint8_t selected = 0;

  for (int i = 0; i < menus[0]->len; i++) {
    if (menus[0]->entries[i].label_id == *g_ui_cmd.params.menu.selected) {
      selected = i;
      break;
    }
  }

  while(1) {
    const menu_t* menu = menus[depth];
    menu_render(menu, titles[depth], selected, draw);

    switch(ui_wait_keypress(portMAX_DELAY)) {
      case KEYPAD_KEY_CANCEL:
        return ERR_CANCEL;
      case KEYPAD_KEY_BACK:
        if (depth) {
          selected = 0;
          depth--;
          draw = MENU_ALL;
        } else {
          return ERR_CANCEL;
        }
        break;
      case KEYPAD_KEY_UP:
        if (selected > 0) {
          selected--;
          draw = MENU_NEXT;
        }
        break;
      case KEYPAD_KEY_DOWN:
        if (selected < (menu->len - 1)) {
          selected++;
          draw = MENU_PREV;
        }
        break;
      case KEYPAD_KEY_CONFIRM:
        if (menu->entries[selected].submenu) {
          assert(depth < (MENU_MAX_DEPTH - 1));
          menus[++depth] = menu->entries[selected].submenu;
          titles[depth] = LSTR(menu->entries[selected].label_id);
          selected = 0;
          draw = MENU_ALL;
        } else {
          *g_ui_cmd.params.menu.selected = menu->entries[selected].label_id;
          return ERR_OK;
        }
        break;
      default:
        draw = MENU_NONE;
        break;
    }
  }
}
