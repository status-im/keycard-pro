#include "core/core.h"

#ifdef DEBUG
#include "ui/ui_internal.h"
void core_simulate_keypress(keypad_key_t key) {
  vTaskDelay(pdMS_TO_TICKS(1000));
  g_last_key = key;
  xTaskNotifyIndexed(APP_TASK(ui), UI_NOTIFICATION_IDX, UI_KEY_EVT, eSetBits);
}
#endif

void core_task_entry(void* pvParameters) {
  while(1) {
    i18n_str_id_t selected;
    ui_menu(&menu_mainmenu, &selected);

#ifdef DEBUG
    core_simulate_keypress(KEYPAD_KEY_DOWN);
    core_simulate_keypress(KEYPAD_KEY_DOWN);
    core_simulate_keypress(KEYPAD_KEY_DOWN);
    core_simulate_keypress(KEYPAD_KEY_CONFIRM);
    core_simulate_keypress(KEYPAD_KEY_BACK);
    core_simulate_keypress(KEYPAD_KEY_CONFIRM);
#endif

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
