#include "FreeRTOS.h"
#include "task.h"

#include "app_tasks.h"
#include "core/core.h"
#include "usb/usb.h"

#define USB_TASK_INTERVAL 20

void usb_task_entry(void* pvParameters) {
  while(1) {
    if (g_core.usb_command.status == COMMAND_COMPLETE) {
      g_core.usb_command.status = COMMAND_PROCESSING;
      xTaskNotifyIndexed(APP_TASK(core), CORE_EVENT_IDX, CORE_USB_EVT, eSetBits);
    }

    if (g_core.usb_command.status == COMMAND_OUTBOUND) {
      usb_hid_send_rapdu();
    }

    vTaskDelay(pdMS_TO_TICKS(USB_TASK_INTERVAL));
  }
}
