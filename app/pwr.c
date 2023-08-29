#include "pwr.h"
#include "hal.h"

static void pwr_wait_idle() {
  //TODO: check that no flash and smartcard communication is active
}

void pwr_reboot() {
  pwr_wait_idle();
  hal_reboot();
}

void pwr_shutdown() {
  pwr_wait_idle();
  hal_gpio_set(GPIO_PWR_KILL, GPIO_SET);
}

void pwr_usb_plugged() {
  //TODO: start usb
  hal_usb_start();
}

void pwr_usb_unplugged() {
  //TODO: stop usb
  hal_usb_stop();
}

void pwr_smartcard_inserted() {
  pwr_reboot();
}

void pwr_smartcard_removed() {
  pwr_shutdown();
}
