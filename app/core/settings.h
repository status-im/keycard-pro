#include "common.h"
#include <stdbool.h>

#define SETTINGS_DEF_SHUTDOWN_TIMEOUT 18000
#define SETTINGS_DEF_LCD_BRIGHTNESS 75
#define SETTINGS_DEF_ENABLE_USB 1
#define SETTINGS_DEF_LANG 0

typedef struct __attribute__((packed)) {
  uint8_t lcd_brightness;
  uint8_t lang;
  bool enable_usb;
  uint32_t shutdown_timeout;
} settings_t;

void settings_load();
void settings_commit();

void settings_lcd_brightness();
