#include "screen.h"
#include "st7789.h"
#include "FreeRTOS.h"
#include "task.h"

#if (_SCREEN_MODEL == ST7789)

hal_err_t st7789_write_cmd(uint8_t cmd) {
  hal_gpio_set(GPIO_LCD_CMD_DATA, GPIO_RESET);
  return hal_spi_send(SPI_LCD, &cmd, 1);
}

hal_err_t st7789_write_params(const uint8_t* params, size_t len) {
  hal_gpio_set(GPIO_LCD_CMD_DATA, GPIO_SET);
  return hal_spi_send(SPI_LCD, params, len);
}

static inline hal_err_t st7789_set_reg8(uint8_t reg, uint8_t value) {
  if (st7789_write_cmd(reg) != HAL_OK) {
    return HAL_ERROR;
  }

  return st7789_write_params(&value, 1);
}

hal_err_t screen_init() {
  hal_gpio_set(GPIO_LCD_RST, GPIO_RESET);
  hal_delay_us(ST7789_RST_PULSE_US);
  hal_gpio_set(GPIO_LCD_RST, GPIO_SET);
  vTaskDelay(pdMS_TO_TICKS(ST7789_CMD_DELAY_MS));

  if (st7789_write_cmd(ST7789_SLPOUT) != HAL_OK) {
    return HAL_ERROR;
  }

  vTaskDelay(pdMS_TO_TICKS(ST7789_CMD_DELAY_MS));

  if (st7789_set_reg8(ST7789_COLMOD, ST7789_COLOR_MODE_65K | ST7789_COLOR_MODE_16BIT) != HAL_OK) {
    return HAL_ERROR;
  }

  if (st7789_set_reg8(ST7789_MADCTL, ST7789_MADCTL_RGB) != HAL_OK) {
    return HAL_ERROR;
  }

  if (st7789_write_cmd(ST7789_INVOFF) != HAL_OK) {
    return HAL_ERROR;
  }

  if (st7789_write_cmd(ST7789_NORON) != HAL_OK) {
    return HAL_ERROR;
  }

  return st7789_write_cmd(ST7789_DISPON);
}

hal_err_t screen_draw_area(screen_area_t* area, uint16_t* pixels) {
  return HAL_ERROR;
}

#endif
