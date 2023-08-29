#ifndef __HAL__
#define __HAL__

#include <stdint.h>
#include <stddef.h>
#include "crypto/sha2_soft.h"
#include "iso7816/smartcard.h"

#ifdef __MCUXPRESSO
#include "nxp.h"
#elif defined STM32_HAL
#include "stm32.h"
#else
#error "Unsupported platform"
#endif

// General
typedef enum {
	HAL_SUCCESS,
	HAL_FAIL,
} hal_err_t;

#define HAL_DEVICE_UID_LEN 16

hal_err_t hal_init();
hal_err_t hal_device_uid(uint8_t out[HAL_DEVICE_UID_LEN]);
void hal_reset();

// Camera
#define CAMERA_WIDTH 480
#define CAMERA_HEIGHT 480
#define CAMERA_BPP 1
#define CAMERA_FB_SIZE CAMERA_WIDTH*CAMERA_HEIGHT*CAMERA_BPP
#define CAMERA_FB_COUNT 2
#define CAMERA_TASK_NOTIFICATION_IDX 0
#define SMARTCARD_TASK_NOTIFICATION_IDX 1

hal_err_t hal_camera_init();
hal_err_t hal_camera_start(uint8_t fb[CAMERA_FB_COUNT][CAMERA_FB_SIZE]);
hal_err_t hal_camera_stop();
hal_err_t hal_camera_next_frame(uint8_t** fb);
hal_err_t hal_camera_submit(uint8_t* fb);

// Screen
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define SCREEN_TASK_NOTIFICATION_IDX 1

// GPIO
typedef enum {
  GPIO_CAMERA_PWDN = 0,
  GPIO_CAMERA_PWR,
  GPIO_CAMERA_RST,
  GPIO_LCD_CMD_DATA,
  GPIO_LCD_RST,
  GPIO_KEYPAD_ROW_0,
  GPIO_KEYPAD_ROW_1,
  GPIO_KEYPAD_ROW_2,
  GPIO_KEYPAD_ROW_3,
  GPIO_KEYPAD_COL_0,
  GPIO_KEYPAD_COL_1,
  GPIO_KEYPAD_COL_2,
  GPIO_VUSB_OK,
  GPIO_SMARTCARD_PRESENT,
  GPIO_HALT_REQ,
  GPIO_PWR_KILL
} hal_gpio_pin_t;

typedef enum {
  GPIO_RESET = 0,
  GPIO_SET = 1,
} hal_gpio_state_t;

void hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state);
hal_gpio_state_t hal_gpio_get(hal_gpio_pin_t pin);

// I2C
typedef enum {
  I2C_CAMERA,
} hal_i2c_port_t;

hal_err_t hal_i2c_send(hal_i2c_port_t port, uint8_t addr, const uint8_t* data, size_t len);

// SPI
typedef enum {
  SPI_LCD
} hal_spi_port_t;

hal_err_t hal_spi_send(hal_spi_port_t port, const uint8_t* data, size_t len);
hal_err_t hal_spi_send_dma(hal_spi_port_t port, const uint8_t* data, size_t len, void (*cb)());

// SmartCard
hal_err_t hal_smartcard_start();
hal_err_t hal_smartcard_stop();
hal_err_t hal_smartcard_pps(smartcard_protocol_t protocol, uint32_t baud, uint32_t freq, uint8_t guard, uint32_t timeout);
hal_err_t hal_smartcard_set_timeout(uint32_t timeout);
hal_err_t hal_smartcard_set_blocklen(uint32_t len);
hal_err_t hal_smartcard_send(const uint8_t* data, size_t len);
hal_err_t hal_smarcard_recv(uint8_t* data, size_t len);
void hal_smartcard_abort();

// Crypto (only use in crypto library)
hal_err_t hal_rng_next(uint8_t *buf, size_t len);

#ifndef SOFT_SHA256
hal_err_t hal_sha256_init(hal_sha256_ctx_t* ctx);
hal_err_t hal_sha256_update(hal_sha256_ctx_t* ctx, const uint8_t* data, size_t len);
hal_err_t hal_sha256_finish(hal_sha256_ctx_t* ctx, uint8_t out[SHA256_DIGEST_LENGTH]);
#endif

#ifndef SOFT_CRC32
hal_err_t hal_crc32_init(hal_crc32_ctx_t* ctx);
hal_err_t hal_crc32_update(hal_crc32_ctx_t* ctx, uint8_t data);
hal_err_t hal_crc32_finish(hal_crc32_ctx_t* ctx, uint32_t *out);
#endif

// Timer
hal_err_t hal_delay_us(uint32_t usec);
void hal_tick();

// Flash
#define HAL_FLASH_BLOCK_ADDR(__BLOCK__) (HAL_FLASH_ADDR + (HAL_FLASH_BLOCK_SIZE * __BLOCK__))
#define HAL_FLASH_ADDR_TO_BLOCK(__ADDR__) ((__ADDR__ - HAL_FLASH_ADDR) / HAL_FLASH_BLOCK_SIZE)

typedef struct {
  uint32_t addr;
  uint32_t count;
} hal_flash_data_segment_t;

hal_err_t hal_flash_begin_program();
hal_err_t hal_flash_program(const uint8_t* data, uint8_t* addr, size_t len);
hal_err_t hal_flash_erase(uint32_t block);
hal_err_t hal_flash_end_program();
const hal_flash_data_segment_t* hal_flash_get_data_segments();

// PWM
typedef enum {
  PWM_BACKLIGHT,
} hal_pwm_output_t;

hal_err_t hal_pwm_set_dutycycle(hal_pwm_output_t out, uint8_t cycle);

// USB
#define HAL_USB_EPOUT_ADDR 0x01
#define HAL_USB_EPIN_ADDR 0x81
#define HAL_USB_MPS 64

hal_err_t hal_usb_start();
hal_err_t hal_usb_stop();
hal_err_t hal_usb_send(const uint8_t* data, size_t len);
hal_err_t hal_usb_send_ctrl(const uint8_t* data, size_t len);
hal_err_t hal_usb_recv_cb(const uint8_t* data, size_t len);
hal_err_t hal_usb_recv_ctrl_cb(const uint8_t* data, size_t len);

#endif
