#include "FreeRTOS.h"
#include "task.h"
#include "log/log.h"
#include "qrcode/qrscan.h"
#include "screen/screen.h"

void ui_task_entry(void* pvParameters) {
  LOG_MSG("Starting UI task");

  if (screen_init() != HAL_OK) {
    LOG_MSG("Failed to init screen");
  }

  screen_fill_area(&screen_fullarea, SCREEN_COLOR_MAROON);

  while(1) {
    qrscan_scan();
  }
}
