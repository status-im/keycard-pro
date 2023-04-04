#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "hal.h"
#include "font/font.h"

#define _SCREEN_MODEL ST7789
#define _SCREEN_MSB_FIRST

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
} screen_area_t;

typedef struct {
  const font_t *font;
  uint16_t fg;
  uint16_t bg;
  uint16_t x;
  uint16_t y;
} screen_text_ctx_t;

extern const screen_area_t screen_fullarea;

#define SCREEN_R_POS_RGB 11
#define SCREEN_G_POS_RGB 5
#define SCREEN_B_POS_RGB 0

#define _SCREEN_RGB(R,G,B) \
  (((R >> 3) << SCREEN_R_POS_RGB) | \
  ((G >> 2) << SCREEN_G_POS_RGB) | \
  ((B >> 3) << SCREEN_B_POS_RGB))

#ifdef _SCREEN_MSB_FIRST
#define SCREEN_RGB(R,G,B) ((_SCREEN_RGB(R, G, B) >> 8) | ((_SCREEN_RGB(R, G, B) & 0xff) << 8))
#else
#define SCREEN_RGB(R,G,B) _SCREEN_RGB(R, G, B)
#endif

#define SCREEN_COLOR_BLACK       SCREEN_RGB(0,     0,   0)
#define SCREEN_COLOR_NAVY        SCREEN_RGB(0,     0, 123)
#define SCREEN_COLOR_DARKGREEN   SCREEN_RGB(0,   125,   0)
#define SCREEN_COLOR_DARKCYAN    SCREEN_RGB(0,   125, 123)
#define SCREEN_COLOR_MAROON      SCREEN_RGB(123,   0,   0)
#define SCREEN_COLOR_PURPLE      SCREEN_RGB(123,   0, 123)
#define SCREEN_COLOR_OLIVE       SCREEN_RGB(123, 125,   0)
#define SCREEN_COLOR_LIGHTGREY   SCREEN_RGB(198, 195, 198)
#define SCREEN_COLOR_DARKGREY    SCREEN_RGB(123, 125, 123)
#define SCREEN_COLOR_BLUE        SCREEN_RGB(0,     0, 255)
#define SCREEN_COLOR_GREEN       SCREEN_RGB(0,   255,   0)
#define SCREEN_COLOR_CYAN        SCREEN_RGB(0,   255, 255)
#define SCREEN_COLOR_RED         SCREEN_RGB(255,   0,   0)
#define SCREEN_COLOR_MAGENTA     SCREEN_RGB(255,   0, 255)
#define SCREEN_COLOR_YELLOW      SCREEN_RGB(255, 255,   0)
#define SCREEN_COLOR_WHITE       SCREEN_RGB(255, 255, 255)
#define SCREEN_COLOR_ORANGE      SCREEN_RGB(255, 165,   0)
#define SCREEN_COLOR_GREENYELLOW SCREEN_RGB(173, 255,  41)
#define SCREEN_COLOR_PINK        SCREEN_RGB(255, 130, 198)

// Low level API
hal_err_t screen_init();
hal_err_t screen_wait();
hal_err_t screen_set_drawing_window(const screen_area_t* area);
hal_err_t screen_draw_pixels(const uint16_t* pixels, size_t count, void (*cb)());

// High level API
hal_err_t screen_draw_char(screen_text_ctx_t* ctx, char c);
hal_err_t screen_draw_string(screen_text_ctx_t* ctx, const char* str);
hal_err_t screen_fill_area(const screen_area_t* area, uint16_t color);
hal_err_t screen_draw_area(const screen_area_t* area, const uint16_t* pixels);
hal_err_t screen_camera_passthrough(const uint8_t* fb);

#endif
