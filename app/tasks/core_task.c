#include "FreeRTOS.h"
#include "task.h"
#include "log/log.h"
#include "ui/ui.h"

void core_task_entry(void* pvParameters) {
  while(1) {
    ui_qrscan();
    vTaskSuspend(NULL);
  }
}
