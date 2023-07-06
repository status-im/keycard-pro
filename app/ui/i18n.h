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

  // Settings menu
  MENU_INFO,

  // TX Confirmation
  TX_CONFIRM_TITLE,
  TX_CONFIRM_ERC20_TITLE,
  TX_CHAIN,
  TX_ADDRESS,
  TX_AMOUNT,
  TX_FEE,

  // MSG Confirmation
  MSG_CONFIRM_TITLE,

  // QR output
  QR_OUTPUT_TITLE,

  // PIN input
  PIN_INPUT_TITLE,
  PIN_CREATE_TITLE,
  PIN_LABEL_REMAINING_ATTEMPTS,
  PIN_LABEL_REPEAT,
  PIN_LABEL_MISMATCH,

  // Info messages
  INFO_ERROR_TITLE,
  INFO_CARD_ERROR_MSG,
  INFO_NOT_KEYCARD,
  INFO_NEW_CARD_TITLE,
  INFO_NEW_CARD,
} i18n_str_id_t;

//TODO: replace with proper lookup
#define LSTR(__ID__) i18n_english_strings[__ID__]

#endif
