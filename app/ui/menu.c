#include <stddef.h>
#include "theme.h"
#include "menu.h"
#include "screen/screen.h"
#include "ui_internal.h"
#include "keypad/keypad.h"

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

  ctx.xStart = TH_MENU_LEFT_MARGIN;
  ctx.x = TH_MENU_LEFT_MARGIN;
  ctx.y = yOff + (((TH_MENU_HEIGHT - TH_MENU_SEP_HEIGHT) - ctx.font->yAdvance) / 2);

  screen_area_t fillarea = { 0, yOff, SCREEN_WIDTH, (TH_MENU_HEIGHT - TH_MENU_SEP_HEIGHT) };
  screen_fill_area(&fillarea, ctx.bg);
  fillarea.y += fillarea.height;
  fillarea.height = TH_MENU_SEP_HEIGHT;
  screen_fill_area(&fillarea, TH_COLOR_MENU_SEPARATOR);

  screen_draw_string(&ctx, LSTR(entry->label_id));
}

void menu_render(const menu_t* menu, uint8_t selected) {
  uint16_t yOff = 0;

  for (int i = 0; i < menu->len; i++) {
    menu_render_entry(&menu->entries[i], i == selected, yOff);
    yOff += TH_MENU_HEIGHT;
  }

  screen_area_t area = { 0, yOff, SCREEN_WIDTH, SCREEN_HEIGHT - yOff };
  screen_fill_area(&area, TH_COLOR_BG);
}

app_err_t menu_run() {
  const menu_t* menus[MENU_MAX_DEPTH];
  uint8_t selected = 0;
  uint8_t depth = 0;
  menus[depth] = g_ui_cmd.params.menu.menu;

  while(1) {
    const menu_t* menu = menus[depth];
    menu_render(menu, selected);

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
