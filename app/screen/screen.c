#include "screen.h"
#include "common.h"

#define _SCREEN_FB_HEIGHT 24
#define _SCREEN_FB_SIZE SCREEN_WIDTH * _SCREEN_FB_HEIGHT
APP_NOCACHE(uint16_t g_screen_fb[_SCREEN_FB_SIZE], 2);

hal_err_t screen_camera_passthrough(const uint8_t* fb) {
  screen_area_t screen = { 0, 0, SCREEN_WIDTH, _SCREEN_FB_HEIGHT };

  int fboff = 0;

  while(screen.y < SCREEN_HEIGHT) {
    for(int i = 0; i < _SCREEN_FB_SIZE; i++) {
      uint8_t luma = fb[fboff];
      fboff += 2;
      g_screen_fb[i] = ~((luma << 8) | luma);
    }

    if (screen_draw_area(&screen, g_screen_fb) != HAL_OK) {
      return HAL_ERROR;
    }

    screen.y += _SCREEN_FB_HEIGHT;
  }

  return HAL_OK;
}
