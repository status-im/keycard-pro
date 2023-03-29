#include "app_tasks.h"
#include "screen.h"
#include "common.h"
#include "log/log.h"
#include "FreeRTOS.h"
#include "task.h"

#define SCREEN_TIMEOUT 100

const screen_area_t screen_fullarea = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

static uint32_t g_screen_busy;

struct _screen_camera_passthrough_ctx {
  uint8_t* fb;
  int y;
};

static struct _screen_camera_passthrough_ctx _cp_ctx;

APP_NOCACHE(uint16_t g_screen_fb[SCREEN_WIDTH], 2);

static inline hal_err_t _screen_preamble(const screen_area_t* area) {
  if (g_screen_busy) {
    return HAL_ERROR;
  }

  if (screen_set_drawing_window(area) != HAL_OK) {
    return HAL_ERROR;
  }

  g_screen_busy = 1;

  return HAL_OK;
}

static void screen_signal() {
  configASSERT(g_screen_busy);

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveIndexedFromISR(ui_task, SCREEN_TASK_NOTIFICATION_IDX, &xHigherPriorityTaskWoken);
  g_screen_busy = 0;
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void screen_camera_line() {
  if (_cp_ctx.y >= SCREEN_HEIGHT) {
    screen_signal();
    return;
  }

  for(int x = 0; x < SCREEN_WIDTH; x++) {
    uint8_t luma = *_cp_ctx.fb;
    _cp_ctx.fb += 2;
    g_screen_fb[x] = luma ? SCREEN_COLOR_BLACK : SCREEN_COLOR_WHITE;
  }

  _cp_ctx.fb += CAMERA_WIDTH;
  _cp_ctx.y++;

  screen_draw_pixels(g_screen_fb, SCREEN_WIDTH, screen_camera_line);
}

hal_err_t screen_camera_passthrough(const uint8_t* fb) {
  if (_screen_preamble(&screen_fullarea) != HAL_OK) {
    return HAL_ERROR;
  }

  _cp_ctx.fb = (uint8_t*) fb;
  _cp_ctx.y = 0;

  screen_camera_line();

  return HAL_OK;
}

hal_err_t screen_fill_area(const screen_area_t* area, uint16_t color) {
  if (_screen_preamble(&screen_fullarea) != HAL_OK) {
    return HAL_ERROR;
  }

  for(int x = 0; x < area->width; x++) {
    g_screen_fb[x] = color;
  }

  for(int y = area->y; y < area->height; y++) {
    g_screen_busy = 1;
    if (screen_draw_pixels(g_screen_fb, area->width, screen_signal) != HAL_OK) {
      return HAL_ERROR;
    }

    if (screen_wait() != HAL_OK) {
      return HAL_ERROR;
    }
  }

  return HAL_OK;
}

hal_err_t screen_draw_area(const screen_area_t* area, const uint16_t* pixels) {
  if (_screen_preamble(&screen_fullarea) != HAL_OK) {
    return HAL_ERROR;
  }

  if (screen_draw_pixels(pixels, (area->width*area->height), screen_signal) != HAL_OK) {
    return HAL_ERROR;
  }

  return screen_wait();
}

hal_err_t screen_wait() {
  return ulTaskNotifyTakeIndexed(SCREEN_TASK_NOTIFICATION_IDX, pdTRUE, pdMS_TO_TICKS(SCREEN_TIMEOUT)) ? HAL_OK : HAL_ERROR;
}
