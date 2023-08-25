#include "core/core.h"
#include "keycard/keycard.h"
#include "pwr.h"

static inline void core_action_run(i18n_str_id_t menu) {
  switch(menu) {
  case MENU_QRCODE:
    core_qr_run();
    break;
  case MENU_DISPLAY_PUBLIC:
    core_display_public();
    break;
  case MENU_DB_UPDATE:
    updater_database_run();
    break;
  case MENU_DEV_AUTH:
    device_auth_run();
    break;
  default:
    //unhandled commands
    break;
  }
}

void core_task_entry(void* pvParameters) {
  if (hal_gpio_get(GPIO_SMARTCARD_PRESENT) == GPIO_RESET) {
    pwr_shutdown();
  }

  keycard_init(&g_core.keycard);
  keycard_activate(&g_core.keycard);

  if (g_core.keycard.sc.state != SC_READY) {
    pwr_reboot();
  }

  while(1) {
    i18n_str_id_t selected;

    switch(ui_menu(LSTR(MENU_TITLE), &menu_mainmenu, &selected)) {
    case CORE_EVT_USB_CMD:
      core_usb_run();
      break;
    case CORE_EVT_UI_OK:
      core_action_run(selected);
      break;
    default:
      // should not happen, if it does we restart the main menu
      break;
    }
  }
}
