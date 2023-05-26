#include "core/core.h"
#include "keycard/keycard.h"

void core_task_entry(void* pvParameters) {
  keycard_init(&g_core.keycard);
  keycard_activate(&g_core.keycard);

  configASSERT(g_core.keycard.sc.state == SC_READY);

  while(1) {
    i18n_str_id_t selected;
    ui_menu(&menu_mainmenu, &selected);

    switch(core_wait_event(1)) {
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
