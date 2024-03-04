#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "hal.h"

#define _CAM_MODEL SC031GS
#define CAMERA_MAX_ANALOG_GAIN 16
#define CAMERA_MAX_DIGITAL_GAIN 8

typedef enum {
  EXPOSURE_1000,
  EXPOSURE_750,
  EXPOSURE_500,
  EXPOSURE_400,
  EXPOSURE_300,
  EXPOSURE_250,
  EXPOSURE_125,
} camera_exposure_t;

hal_err_t camera_start();
hal_err_t camera_stop();

hal_err_t camera_next_frame(uint8_t** frame);
hal_err_t camera_submit(uint8_t* frame);

hal_err_t camera_autoexposure(uint32_t total_luma);

hal_err_t camera_set_exposure(camera_exposure_t exposure);
hal_err_t camera_set_analog_gain(uint8_t gain);
hal_err_t camera_set_digital_gain(uint8_t gain);

#endif /* __CAMERA_H__ */
