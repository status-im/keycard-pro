#include <app_tasks.h>
#include "FreeRTOS.h"
#include "task.h"
#include "hal.h"

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  static StaticTask_t xIdleTaskTCBBuffer;
  static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = xIdleStack;
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if (configUSE_TIMERS == 1)
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  static StaticTask_t xTimerTaskTCB;
  static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif

#define QRSCAN_STACK_SIZE 512
static StaticTask_t qrscan_task_memory;
static StackType_t qrscan_task_stack[QRSCAN_STACK_SIZE];
TaskHandle_t qrscan_task;

int main(void) {
  hal_init();

  qrscan_task = xTaskCreateStatic(qrscan_task_entry, "QR Scan", QRSCAN_STACK_SIZE, NULL, 1, qrscan_task_stack, &qrscan_task_memory);
  vTaskStartScheduler();

  while(1) {
    __asm volatile ("nop");
  }
}
