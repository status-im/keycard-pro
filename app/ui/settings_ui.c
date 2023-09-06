#include "settings_ui.h"
#include "keypad/keypad.h"
#include "ui/dialog.h"
#include "ui/theme.h"
#include "ui/ui.h"
#include "ui/ui_internal.h"

#include <stdbool.h>

#define LCD_MAX_BRIGHTNESS 100
#define LCD_MIN_BRIGHTNESS 5

static void handle_lcd_change(bool increase) {
  do {
    if (increase) {
      if (*g_ui_cmd.params.lcd.brightness < LCD_MAX_BRIGHTNESS) {
        (*g_ui_cmd.params.lcd.brightness)++;
      }
    } else {
      if (*g_ui_cmd.params.lcd.brightness > LCD_MIN_BRIGHTNESS) {
        (*g_ui_cmd.params.lcd.brightness)--;
      }
    }

    hal_pwm_set_dutycycle(PWM_BACKLIGHT, *g_ui_cmd.params.lcd.brightness);
    vTaskDelay(pdMS_TO_TICKS(50));
  } while(g_ui_ctx.keypad.last_key_long && !g_ui_ctx.keypad.last_key_released);
}

app_err_t settings_ui_lcd_brightness() {
  dialog_title(LSTR(LCD_BRIGHTNESS_TITLE));
  dialog_footer(TH_TITLE_HEIGHT);

  while(1) {
    switch(ui_wait_keypress(portMAX_DELAY)) {
    case KEYPAD_KEY_CANCEL:
      return ERR_CANCEL;
    case KEYPAD_KEY_BACK:
      return ERR_CANCEL;
    case KEYPAD_KEY_CONFIRM:
      return ERR_OK;
    case KEYPAD_KEY_LEFT:
      handle_lcd_change(false);
      break;
    case KEYPAD_KEY_RIGHT:
      handle_lcd_change(true);
      break;
    default:
      break;
    }
  }
}
