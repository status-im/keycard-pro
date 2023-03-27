#include "screen.h"
#include "common.h"

APP_NOCACHE(uint16_t g_screen_fb[SCREEN_WIDTH], 2);

const screen_area_t screen_fullarea = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

hal_err_t screen_camera_passthrough(const uint8_t* fb) {
  screen_set_drawing_window(&screen_fullarea);

  int fboff = 0;

  for(int y = 0; y < SCREEN_HEIGHT; y++) {
    for(int x = 0; x < SCREEN_WIDTH; x++) {
      uint8_t luma = fb[fboff];
      fboff += 2;
      g_screen_fb[x] = luma ? 0 : 0xffff;
    }

    fboff += CAMERA_WIDTH;

    if (screen_draw_pixels(g_screen_fb, SCREEN_WIDTH) != HAL_OK) {
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
