#ifndef __HAL__
#define __HAL__

#include <stdint.h>
#include <stddef.h>
#include "crypto/sha2_soft.h"
#include "iso7816/smartcard.h"

#ifdef __MCUXPRESSO
#include "fsl_dcp.h"
typedef dcp_hash_ctx_t hal_sha256_ctx_t;
#define SOFT_CRC32
#elif defined STM32_HAL
#define SOFT_SHA256
#define SOFT_CRC32
#else
#define SOFT_SHA256
#define SOFT_CRC32
#endif

// General
typedef enum {
	HAL_SUCCESS,
	HAL_FAIL,
} hal_err_t;

hal_err_t hal_init();

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
  GPIO_CAMERA_RST,
  GPIO_LCD_CMD_DATA,
  GPIO_LCD_RST,
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

hal_err_t hal_i2c_send(hal_i2c_port_t port, uint8_t addr, const uint8_t* data, size_t len);

// UART
typedef enum {
  UART_LOG,
} hal_uart_port_t;

hal_err_t hal_uart_send(hal_uart_port_t port, const uint8_t* data, size_t len);

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


// Crypto (only use in crypto library)
hal_err_t hal_rng_next(uint8_t *buf, size_t len);

#ifndef SOFT_SHA256
hal_err_t hal_sha256_init(hal_sha256_ctx_t* ctx);
hal_err_t hal_sha256_update(hal_sha256_ctx_t* ctx, const uint8_t* data, size_t len);
hal_err_t hal_sha256_finish(hal_sha256_ctx_t* ctx, uint8_t out[SHA256_DIGEST_LENGTH]);
#endif

#ifndef SOFT_CRC32
hal_err_t hal_crc32_init(hal_crc32_ctx_t* ctx);
hal_err_t hal_crc32_update(hal_crc32_ctx_t* ctx, const uint8_t* data, size_t len);
hal_err_t hal_crc32_finish(hal_crc32_ctx_t* ctx, uint32_t *out);
#endif

// Timer
hal_err_t hal_delay_us(uint32_t usec);

#endif
