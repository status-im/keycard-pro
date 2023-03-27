#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "hal.h"

#define _SCREEN_MODEL ST7789

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
} screen_area_t;

hal_err_t screen_init();
hal_err_t screen_draw_area(const screen_area_t* area, const uint16_t* pixels);
hal_err_t screen_camera_passthrough(const uint8_t* fb);

#endif