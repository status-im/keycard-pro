#ifndef __I18N__
#define __I18N__

extern const char* i18n_english_strings[];

typedef enum {
  MENU_QRCODE = 0,
  MENU_DISPLAY_PUBLIC,
  MENU_SETTINGS,
} i18n_str_t;

//TODO: replace with proper lookup
#define LSTR(__ID__) i18n_english_strings[__ID__]

#endif
