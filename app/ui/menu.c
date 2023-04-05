#include <stddef.h>
#include "theme.h"
#include "menu.h"
#include "screen/screen.h"

const menu_t menu_settings = {
  0, {}
};

const menu_t menu_mainmenu = {
  3,
  {
    {MENU_QRCODE, NULL},
    {MENU_DISPLAY_PUBLIC, NULL},
    {MENU_SETTINGS, &menu_settings},
  }
};

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

void menu_run(const menu_t* menu) {
  uint8_t selected = 0;
  menu_render(menu, selected);
  while(1) {

  }
}
