#include "screen.h"
#include "common.h"
#include "log/log.h"
#include "FreeRTOS.h"
#include "task.h"

APP_NOCACHE(uint16_t g_screen_fb[SCREEN_WIDTH], 2);

#define SCREEN_TIMEOUT 1000

const screen_area_t screen_fullarea = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };


static TaskHandle_t g_screen_task = NULL;

struct _screen_camera_passthrough_ctx {
  uint8_t* fb;
  int y;
};

static struct _screen_camera_passthrough_ctx _cp_ctx;

static void screen_signal() {
  configASSERT(g_screen_task);

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveIndexedFromISR(g_screen_task, SCREEN_TASK_NOTIFICATION_IDX, &xHigherPriorityTaskWoken);
  g_screen_task = NULL;
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
    g_screen_fb[x] = luma ? 0 : 0xffff;
  }

  _cp_ctx.fb += CAMERA_WIDTH;
  _cp_ctx.y++;

  screen_draw_pixels(g_screen_fb, SCREEN_WIDTH, screen_camera_line);
}

hal_err_t screen_camera_passthrough(const uint8_t* fb) {
  if (g_screen_task) {
    return HAL_ERROR;
  }

  screen_set_drawing_window(&screen_fullarea);

  _cp_ctx.fb = (uint8_t*) fb;
  _cp_ctx.y = 0;

  g_screen_task = xTaskGetCurrentTaskHandle();
  screen_camera_line();

  return HAL_OK;
}

hal_err_t screen_draw_area(const screen_area_t* area, const uint16_t* pixels) {
  if (g_screen_task) {
    return HAL_ERROR;
  }

  if (screen_set_drawing_window(area) != HAL_OK) {
    return HAL_ERROR;
  }

  g_screen_task = xTaskGetCurrentTaskHandle();
  return screen_draw_pixels(pixels, (area->width*area->height), screen_signal);
}

hal_err_t screen_wait() {
  return ulTaskNotifyTakeIndexed(SCREEN_TASK_NOTIFICATION_IDX, pdTRUE, pdMS_TO_TICKS(SCREEN_TIMEOUT)) ? HAL_OK : HAL_ERROR;
}
