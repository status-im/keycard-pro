#include "common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hal.h"

#define USB_STACK_SIZE 100
#define USB_TASK_PRIO 1

#define CORE_STACK_SIZE 1024
#define CORE_TASK_PRIO 2

#define UI_STACK_SIZE 6000
#define UI_TASK_PRIO 3

APP_DEF_TASK(usb, USB_STACK_SIZE);
APP_DEF_TASK(core, CORE_STACK_SIZE);
APP_DEF_TASK(ui, UI_STACK_SIZE);

int main(void) {
  hal_init();

  APP_CREATE_TASK(usb, USB_TASK_PRIO);
  APP_CREATE_TASK(core, CORE_TASK_PRIO);
  APP_CREATE_TASK(ui, UI_TASK_PRIO);

  vTaskSuspend(APP_TASK(usb));
  vTaskStartScheduler();

  while(1) {
    __asm volatile ("nop");
  }
}
