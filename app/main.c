#include <app_tasks.h>
#include "common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hal.h"

#define QRSCAN_STACK_SIZE 512
#define QRSCAN_TASK_PRIO 1


int main(void) {
  hal_init();

  APP_DEF_CREATE_TASK(qrscan, QRSCAN_TASK_PRIO, QRSCAN_STACK_SIZE);

  vTaskStartScheduler();

  while(1) {
    __asm volatile ("nop");
  }
}
