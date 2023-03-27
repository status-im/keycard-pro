#include "screen.h"
#include "common.h"

#define _SCREEN_FB_HEIGHT 24
#define _SCREEN_FB_SIZE SCREEN_WIDTH * _SCREEN_FB_HEIGHT
APP_NOCACHE(uint16_t g_screen_fb[_SCREEN_FB_SIZE], 2);

const screen_area_t screen_fullarea = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

hal_err_t screen_camera_passthrough(const uint8_t* fb) {
  screen_set_drawing_window(&screen_fullarea);

  int fboff = 0;

  for(int y = 0; y < SCREEN_HEIGHT; y += _SCREEN_FB_HEIGHT) {
    for(int x = 0; x < _SCREEN_FB_SIZE; x++) {
      uint8_t luma = fb[fboff];
      fboff += 2;
      g_screen_fb[x] = ~((luma << 8) | luma);
    }

    if (screen_draw_pixels(g_screen_fb, _SCREEN_FB_SIZE) != HAL_OK) {
      return HAL_ERROR;
    }
  }

  return HAL_OK;
}

hal_err_t screen_draw_area(const screen_area_t* area, const uint16_t* pixels) {
  if (screen_set_drawing_window(area) != HAL_OK) {
    return HAL_ERROR;
  }

  return screen_draw_pixels(pixels, (area->width*area->height));
}
