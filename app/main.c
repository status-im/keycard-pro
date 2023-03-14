#include <app_tasks.h>
#include "common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hal.h"

#define QRSCAN_STACK_SIZE 512
#define QRSCAN_TASK_PRIO 1

APP_DEF_TASK(qrscan, QRSCAN_STACK_SIZE);

int main(void) {
  hal_init();

  APP_CREATE_TASK(qrscan, QRSCAN_TASK_PRIO);

  vTaskStartScheduler();

  while(1) {
    __asm volatile ("nop");
  }
}
