#include "FreeRTOS.h"
#include "task.h"
#include "app_tasks.h"

#include "core/core.h"
#include "core/settings.h"
#include "hal.h"
#include "pwr.h"
#include "usb/usb.h"

static void pwr_graceful_shutdown() {
  while(hal_flash_busy()) {
    ;
  }

  settings_commit();
}

void pwr_reboot() {
  pwr_graceful_shutdown();
  hal_reboot();
}

void pwr_shutdown() {
  pwr_graceful_shutdown();
  hal_gpio_set(GPIO_PWR_KILL, GPIO_SET);
}

void pwr_usb_plugged(bool from_isr) {
  if (g_settings.enable_usb && g_core.ready) {
    hal_usb_start();
  }

  if (from_isr) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveIndexedFromISR(APP_TASK(usb), USB_NOTIFICATION_IDX, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  } else {
    xTaskNotifyGiveIndexed(APP_TASK(usb), USB_NOTIFICATION_IDX);
  }
}

void pwr_usb_unplugged() {
  hal_usb_stop();
}

void pwr_smartcard_inserted() {
  pwr_reboot();
}

void pwr_smartcard_removed() {
  pwr_shutdown();
}

void pwr_inactivity_timer_elapsed() {
  pwr_shutdown();
}
