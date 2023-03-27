#include "screen.h"
#include "st7789.h"
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

static hal_err_t st7789_write_data(const uint8_t* data, size_t len) {
  //TODO: replace with DMA
  hal_gpio_set(GPIO_LCD_CMD_DATA, GPIO_SET);
  return hal_spi_send(SPI_LCD, data, len);
}

static inline hal_err_t st7789_set_reg8(uint8_t reg, uint8_t value) {
  if (st7789_write_cmd(reg) != HAL_OK) {
    return HAL_ERROR;
  }

  return st7789_write_params(&value, 1);
}

static hal_err_t st7789_set_drawing_window(const screen_area_t* area) {
  if (st7789_write_cmd(ST7789_CASET) != HAL_OK) {
    return HAL_ERROR;
  }

  uint16_t end = (area->x + area->width);
  uint8_t data[4];
  data[0] = area->x >> 8;
  data[1] = area->x & 0xff;
  data[2] = end >> 8;
  data[3] = end & 0xff;

  if (st7789_write_params(data, 4) != HAL_OK) {
    return HAL_ERROR;
  }

  if (st7789_write_cmd(ST7789_RASET) != HAL_OK) {
    return HAL_ERROR;
  }

  end = (area->y + area->height);
  data[0] = area->y >> 8;
  data[1] = area->y & 0xff;
  data[2] = end >> 8;
  data[3] = end & 0xff;

  return st7789_write_params(data, 4);
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

hal_err_t screen_draw_area(const screen_area_t* area, const uint16_t* pixels) {
  st7789_set_drawing_window(area);

  if (st7789_write_cmd(ST7789_RAMWR) != HAL_OK) {
    return HAL_ERROR;
  }

  size_t len = area->width * area->height * sizeof(uint16_t);
  st7789_write_data((uint8_t*) pixels, len);

  return HAL_OK;
}

#endif
