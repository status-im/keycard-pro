#ifndef __HAL__
#define __HAL__

#include <stdint.h>

typedef enum {
	HAL_OK,
	HAL_ERROR
} hal_err_t;

#define CAMERA_OV7725  0
#define CAMERA_MT9M114 1

#define APP_CAMERA CAMERA_MT9M114

#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480
#define CAMERA_BPP 1
#define CAMERA_FB_SIZE CAMERA_WIDTH*CAMERA_HEIGHT*CAMERA_BPP
#define CAMERA_FB_COUNT 2

hal_err_t hal_init();
hal_err_t hal_camera_init(uint8_t fb[CAMERA_FB_COUNT][CAMERA_FB_SIZE]);
hal_err_t hal_camera_next_frame(uint8_t** fb);
hal_err_t hal_camera_submit(uint8_t* fb);
#endif
