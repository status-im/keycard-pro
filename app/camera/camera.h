#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "hal.h"

#define _CAM_REG_NULL 0xFFFF
#define _CAM_REG_DELAY 0X0000
#define _CAM_MODEL SC031GS

extern uint8_t g_camera_fb[CAMERA_FB_COUNT][CAMERA_FB_SIZE];

struct camera_regval {
  uint16_t addr;
  uint8_t val;
};

hal_err_t camera_start();
hal_err_t camera_stop();

hal_err_t camera_next_frame(uint8_t** frame);
hal_err_t camera_submit(uint8_t* frame);

#endif /* __CAMERA_H__ */
