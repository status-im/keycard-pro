#include "pwr.h"
#include "hal.h"

void pwr_reboot() {
  //TODO: implement
}

void pwr_shutdown() {
  //TODO: wait flash and/or smartcard communication to finish
  hal_gpio_set(GPIO_PWR_KILL, GPIO_SET);
}

void pwr_usb_plugged() {
  //TODO: start usb
}

void pwr_usb_unplugged() {
  //TODO: stop usb
}

void pwr_smartcard_inserted() {
  pwr_reboot();
}

void pwr_smartcard_removed() {
  pwr_shutdown();
}
