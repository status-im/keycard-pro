#include "screen.h"
#include "st7789.h"
#include "common.h"
#include "FreeRTOS.h"
#include "task.h"

#if (_SCREEN_MODEL == ST7789)

static hal_err_t st7789_write_cmd(uint8_t cmd) {
  hal_gpio_set(GPIO_LCD_CMD_DATA, GPIO_RESET);
  return hal_spi_send(SPI_LCD, &cmd, 1);
}

static hal_err_t st7789_write_params(const uint8_t* params, size_t len) {
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

  if (st7789_write_cmd(ST7789_INVON) != HAL_OK) {
    return HAL_ERROR;
  }

  if (st7789_write_cmd(ST7789_NORON) != HAL_OK) {
    return HAL_ERROR;
  }

  return st7789_write_cmd(ST7789_DISPON);
}

hal_err_t screen_set_drawing_window(const screen_area_t* area) {
  if (st7789_write_cmd(ST7789_CASET) != HAL_OK) {
    return HAL_ERROR;
  }

  uint16_t data[2];
  data[0] = rev16(area->x);
  data[1] = rev16(area->x + area->width);

  if (st7789_write_params((uint8_t*) data, 4) != HAL_OK) {
    return HAL_ERROR;
  }

  if (st7789_write_cmd(ST7789_RASET) != HAL_OK) {
    return HAL_ERROR;
  }

  data[0] = rev16(area->y);
  data[1] = rev16(area->y + area->height);

  if (st7789_write_params((uint8_t*) data, 4) != HAL_OK) {
    return HAL_ERROR;
  }

  if (st7789_write_cmd(ST7789_RAMWR) != HAL_OK) {
    return HAL_ERROR;
  }

  hal_gpio_set(GPIO_LCD_CMD_DATA, GPIO_SET);

  return HAL_OK;
}

hal_err_t screen_draw_pixels(const uint16_t* pixels, size_t count) {
  //TODO: replace with DMA
  return hal_spi_send(SPI_LCD, (uint8_t*) pixels, (count << 1));
}

#endif
