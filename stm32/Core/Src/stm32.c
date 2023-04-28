#include "main.h"
#include "hal.h"
#include <string.h>

hal_err_t hal_init() {
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_GPDMA1_Init();
  MX_USART3_UART_Init();
  MX_DCMI_Init();
  MX_RNG_Init();
  MX_ICACHE_Init();
  MX_HASH_Init();
  MX_SPI1_Init();
  MX_I2C2_Init();
  MX_USART1_SMARTCARD_Init();

  return HAL_SUCCESS;
}

hal_err_t hal_camera_init() {
  return HAL_FAIL;
}

hal_err_t hal_camera_start(uint8_t fb[CAMERA_FB_COUNT][CAMERA_FB_SIZE]) {
  return HAL_FAIL;
}
hal_err_t hal_camera_stop() {
  return HAL_FAIL;
}

hal_err_t hal_camera_next_frame(uint8_t** fb) {
  return HAL_FAIL;
}

hal_err_t hal_camera_submit(uint8_t* fb) {
  return HAL_FAIL;
}

hal_err_t hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state) {
  return HAL_FAIL;
}

hal_err_t hal_i2c_send(hal_i2c_port_t port, uint8_t addr, const uint8_t* data, size_t len) {
  return HAL_FAIL;
}

hal_err_t hal_uart_send(hal_uart_port_t port, const uint8_t* data, size_t len) {
  return HAL_FAIL;
}

hal_err_t hal_spi_send(hal_spi_port_t port, const uint8_t* data, size_t len) {
  return HAL_FAIL;
}

hal_err_t hal_spi_send_dma(hal_spi_port_t port, const uint8_t* data, size_t len, void (*cb)()) {
  return HAL_FAIL;
}

hal_err_t hal_rng_next(uint8_t *buf, size_t len) {
  uint32_t rnd;

  while(len >= 4) {
    while (HAL_RNG_GenerateRandomNumber(&hrng, &rnd) != HAL_OK) {}
    len -= 4;
    memcpy(buf, &rnd, 4);
    buf += 4;
  }

  if (len) {
    while (HAL_RNG_GenerateRandomNumber(&hrng, &rnd) != HAL_OK) {}
    memcpy(buf, &rnd, len);
  }

  return HAL_SUCCESS;
}

hal_err_t hal_delay_us(uint32_t usec) {
  return HAL_FAIL;
}

