#include <stddef.h>

#include "dialog.h"
#include "menu.h"
#include "keypad/keypad.h"
#include "screen/screen.h"
#include "theme.h"
#include "ui_internal.h"

const menu_t menu_keycard = {
  1, {
    {MENU_CHANGE_PIN, NULL}
  }
};

const menu_t menu_settings = {
  1, {
    {MENU_INFO, NULL}
  }
};

const menu_t menu_mainmenu = {
  5, {
    {MENU_QRCODE, NULL},
    {MENU_DISPLAY_PUBLIC, NULL},
    {MENU_KEYCARD, &menu_keycard},
    {MENU_SETTINGS, &menu_settings},
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

void menu_render(const menu_t* menu, const char* title, uint8_t selected) {
  dialog_title(title);
  uint16_t yOff = TH_TITLE_HEIGHT;

  for (int i = 0; i < menu->len; i++) {
    menu_render_entry(&menu->entries[i], i == selected, yOff);
    yOff += TH_MENU_HEIGHT;
  }

  dialog_footer(yOff);
}

app_err_t menu_run() {
  const menu_t* menus[MENU_MAX_DEPTH];
  i18n_str_id_t titles[MENU_MAX_DEPTH];

  uint8_t selected = 0;
  uint8_t depth = 0;
  menus[depth] = g_ui_cmd.params.menu.menu;
  titles[depth] = MENU_TITLE;

  while(1) {
    const menu_t* menu = menus[depth];
    menu_render(menu, LSTR(titles[depth]), selected);

    switch(ui_wait_keypress(portMAX_DELAY)) {
      case KEYPAD_KEY_CANCEL:
        return ERR_CANCEL;
      case KEYPAD_KEY_BACK:
        if (depth) {
          selected = 0;
          depth--;
        }
        break;
      case KEYPAD_KEY_UP:
        if (selected > 0) {
          selected--;
        }
        break;
      case KEYPAD_KEY_DOWN:
        if (selected < (menu->len - 1)) {
          selected++;
        }
        break;
      case KEYPAD_KEY_CONFIRM:
        if (menu->entries[selected].submenu) {
          assert(depth < (MENU_MAX_DEPTH - 1));
          menus[++depth] = menu->entries[selected].submenu;
          titles[depth] = menu->entries[selected].label_id;
          selected = 0;
        } else {
          *g_ui_cmd.params.menu.selected = menu->entries[selected].label_id;
          return ERR_OK;
        }
        break;
      default:
        break;
    }
  }
}
