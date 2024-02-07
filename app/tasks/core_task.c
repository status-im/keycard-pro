#ifndef TEST_APP
#include "core/core.h"
#include "core/card.h"
#include "core/settings.h"
#include "keycard/keycard.h"
#include "pwr.h"
#include "usb/usb.h"
#include "crypto/aes.h"
#include "crypto/ecdsa.h"
#include "crypto/secp256k1.h"

static inline void core_action_run(i18n_str_id_t menu) {
  switch(menu) {
  // Top Level
  case MENU_QRCODE:
    core_qr_run();
    break;
  case MENU_DISPLAY_PUBLIC:
    core_display_public();
    break;
  // Keycard
  case MENU_CARD_NAME:
    card_change_name();
    break;
  case MENU_CHANGE_PIN:
    card_change_pin();
    break;
  case MENU_CHANGE_PUK:
    card_change_puk();
    break;
  case MENU_CHANGE_PAIRING:
    card_change_pairing();
    break;
  case MENU_RESET_CARD:
    card_reset();
    break;
  // Device
  case MENU_INFO:
    device_info();
    break;
  case MENU_DB_UPDATE:
    updater_database_run();
    break;
  case MENU_DEV_AUTH:
    device_auth_run();
    break;
  // Settings
  case MENU_BRIGHTNESS:
    settings_lcd_brightness();
    break;
  case MENU_SET_OFF_TIME:
    settings_set_off_time();
    break;
  case MENU_USB:
    settings_usb_onoff();
    break;
  default:
    //unknown commands
    break;
  }
}

const uint8_t* ec_uncompress_key(const ecdsa_curve *curve, const uint8_t* pub_key, uint8_t out[ECC256_POINT_SIZE]);
const uint8_t EXAMPLE_PRIV[] = {
    0x90, 0xf4, 0x55, 0x61, 0xb5, 0xa4, 0x3d, 0xa2, 0x7f, 0x35, 0x70, 0x63, 0x48, 0xbf, 0x86, 0xa4,
    0x75, 0x9b, 0x23, 0x8a, 0x58, 0xa0, 0xed, 0xdb, 0x24, 0x2a, 0xa2, 0x64, 0xd0, 0xf0, 0x2f, 0x55,
};
const uint8_t EXAMPLE_PUB[] = {
    0x03,
    0xf2, 0x56, 0xc6, 0xe6, 0xc8, 0x0b, 0x21, 0x3f, 0x2a, 0xa0, 0xb0, 0x17, 0x44, 0x23, 0x5d, 0x51,
    0x5c, 0x59, 0x44, 0x35, 0xbe, 0x65, 0x1b, 0x15, 0x88, 0x3a, 0x10, 0xdd, 0x47, 0x2f, 0xa6, 0x46,

    0xce, 0x62, 0xea, 0xf3, 0x67, 0x0d, 0xc5, 0xcb, 0x91, 0x00, 0xa0, 0xca, 0x2a, 0x55, 0xb2, 0xc1,
    0x47, 0xc1, 0xe9, 0xa3, 0x8c, 0xe4, 0x28, 0x87, 0x8e, 0x7d, 0x46, 0xe1, 0xfb, 0x71, 0x4a, 0x99,
};

void core_task_entry(void* pvParameters) {
  uint8_t out[ECC256_POINT_SIZE + 1];
  //ecdsa_get_public_key65(&secp256k1, EXAMPLE_PRIV, out);
  ec_uncompress_key(&secp256k1, EXAMPLE_PUB, out);

  if (hal_gpio_get(GPIO_SMARTCARD_PRESENT) == GPIO_RESET) {
    pwr_shutdown();
  }

  keycard_init(&g_core.keycard);
  keycard_activate(&g_core.keycard);

  if (g_core.keycard.sc.state != SC_READY) {
    pwr_reboot();
  }

  g_core.ready = true;

  usb_start_if_connected();

  while(1) {
    i18n_str_id_t selected = MENU_QRCODE;
    const char* title = g_core.keycard.name[0] ? g_core.keycard.name : LSTR(MENU_TITLE);

    switch(ui_menu(title, &menu_mainmenu, &selected, 1)) {
    case CORE_EVT_USB_CMD:
      core_usb_run();
      break;
    case CORE_EVT_UI_OK:
      core_action_run(selected);
      break;
    case CORE_EVT_UI_CANCELLED:
    default:
      break;
    }
  }
}
#endif
