#include "FreeRTOS.h"
#include "task.h"
#include "app_tasks.h"

#include "core/settings.h"
#include "hal.h"
#include "pwr.h"

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

void pwr_usb_plugged() {
  hal_usb_start();
  vTaskResume(APP_TASK(usb));
}

void pwr_usb_unplugged() {
  vTaskSuspend(APP_TASK(usb));
  hal_usb_stop();
}

void pwr_smartcard_inserted() {
  pwr_reboot();
}

void pwr_smartcard_removed() {
  pwr_shutdown();
}
