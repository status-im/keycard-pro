#include "settings.h"
#include "storage/fs.h"
#include <string.h>

#define FS_SETTINGS 0x5331

struct __attribute__((packed)) settings_entry {
  fs_entry_t entry;
  settings_t settings;
};

static settings_t g_settings;
static struct settings_entry* _settings_entry;

fs_action_t _settings_match_settings(void* ctx, fs_entry_t* entry) {
  return entry->magic == FS_SETTINGS ? FS_ACCEPT : FS_REJECT;
}

void settings_load() {
  _settings_entry = (struct settings_entry*) fs_find(_settings_match_settings, NULL);

  if (!_settings_entry) {
    g_settings.enable_usb = SETTINGS_DEF_ENABLE_USB;
    g_settings.lang = SETTINGS_DEF_LANG;
    g_settings.lcd_brightness = SETTINGS_DEF_LCD_BRIGHTNESS;
    g_settings.shutdown_timeout = SETTINGS_DEF_SHUTDOWN_TIMEOUT;
  } else {
    memcpy(&g_settings, &_settings_entry->settings, sizeof(settings_t));
  }
}

void settings_commit() {
  if (_settings_entry) {
    if (!memcmp(&g_settings, &_settings_entry->settings, sizeof(settings_t))) {
      return;
    }

    if (fs_erase(&_settings_entry->entry) != ERR_OK) {
      return;
    }
  }

  struct settings_entry entry;
  entry.entry.magic = FS_SETTINGS;
  entry.entry.len = sizeof(settings_t);
  memcpy(&entry.settings, &g_settings, sizeof(settings_t));
  fs_write(&entry.entry, sizeof(struct settings_entry));
}
