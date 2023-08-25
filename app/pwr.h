#ifndef __PWR__
#define __PWR__

void pwr_reboot();
void pwr_shutdown();

void pwr_usb_plugged();
void pwr_usb_unplugged();

void pwr_smartcard_inserted();
void pwr_smartcard_removed();

#endif
