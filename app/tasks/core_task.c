#include "FreeRTOS.h"
#include "task.h"
#include "log/log.h"
#include "ui/ui.h"
#include "ur/eip4527_types.h"

static struct eth_sign_request sign_request;

void core_task_entry(void* pvParameters) {
  while(1) {
    ui_qrscan(&sign_request);

    vTaskSuspend(NULL);
  }
}
