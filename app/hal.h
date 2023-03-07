#ifndef __HAL__
#define __HAL__

#include <stdint.h>

// General
typedef enum {
	HAL_OK,
	HAL_ERROR
} hal_err_t;


hal_err_t hal_init();

// Camera
#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480
#define CAMERA_BPP 1
#define CAMERA_FB_SIZE CAMERA_WIDTH*CAMERA_HEIGHT*CAMERA_BPP
#define CAMERA_FB_COUNT 2
#define CAMERA_TASK_NOTIFICATION_IDX 0

hal_err_t hal_camera_start(uint8_t fb[CAMERA_FB_COUNT][CAMERA_FB_SIZE]);
hal_err_t hal_camera_stop();
hal_err_t hal_camera_next_frame(uint8_t** fb);
hal_err_t hal_camera_submit(uint8_t* fb);

// GPIO
typedef enum {
  GPIO_CAMERA_PWDN = 0,
  GPIO_CAMERA_RST,
} hal_gpio_pin_t;

typedef enum {
  GPIO_RESET = 0,
  GPIO_SET = 1,
} hal_gpio_state_t;

hal_err_t hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state);

// I2C
typedef enum {
  I2C_CAMERA,
} hal_i2c_port_t;

// I2C
hal_err_t hal_i2c_send(hal_i2c_port_t port, uint8_t addr, uint8_t* data, size_t len);

#endif
