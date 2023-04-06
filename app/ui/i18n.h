#ifndef __I18N__
#define __I18N__

extern const char* i18n_english_strings[];

typedef enum {
  // Main menu
  MENU_QRCODE = 0,
  MENU_DISPLAY_PUBLIC,
  MENU_KEYCARD,
  MENU_SETTINGS,
  MENU_OFF,

  // Keycard menu
  MENU_CHANGE_PIN,

  // Settings meu
  MENU_INFO
} i18n_str_t;

//TODO: replace with proper lookup
#define LSTR(__ID__) i18n_english_strings[__ID__]

#endif
