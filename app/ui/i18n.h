#ifndef __I18N__
#define __I18N__

extern const char* i18n_english_strings[];

typedef enum {
  // Main menu
  MENU_TITLE = 0,
  MENU_QRCODE,
  MENU_DISPLAY_PUBLIC,
  MENU_KEYCARD,
  MENU_SETTINGS,
  MENU_OFF,

  // Keycard menu
  MENU_CHANGE_PIN,

  // Settings meu
  MENU_INFO,

  // TX Confirmation
  TX_CONFIRM_TITLE,
  TX_ADDRESS,
  TX_AMOUNT,
  TX_FEE,
  TX_TOTAL
} i18n_str_id_t;

//TODO: replace with proper lookup
#define LSTR(__ID__) i18n_english_strings[__ID__]

#endif
