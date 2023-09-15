#ifndef __PWR__
#define __PWR__

#include <stdbool.h>

void pwr_reboot();
void pwr_shutdown();

void pwr_usb_plugged(bool from_isr);
void pwr_usb_unplugged();

void pwr_smartcard_inserted();
void pwr_smartcard_removed();

void pwr_inactivity_timer_elapsed();

#endif
