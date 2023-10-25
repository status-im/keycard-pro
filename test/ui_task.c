#ifdef TEST_APP
#include "FreeRTOS.h"
#include "task.h"

#include "app_tasks.h"
#include "qrcode/qrscan.h"
#include "screen/screen.h"
#include "ui/dialog.h"
#include "ui/settings_ui.h"
#include "ui/theme.h"
#include "ui/ui_internal.h"

struct ui_cmd g_ui_cmd;
struct ui_ctx g_ui_ctx;

#define TH_FIELD_MARGIN ((SCREEN_WIDTH - ((TH_PIN_FIELD_WIDTH * 3) + (TH_PIN_FIELD_DIGIT_MARGIN * 2))) / 2)

static app_err_t test_keypad() {
  dialog_title("Keypad test");
  dialog_footer(TH_TITLE_HEIGHT);

  screen_area_t area = {
      .x = TH_FIELD_MARGIN,
      .y = TH_TITLE_HEIGHT + TH_PIN_FIELD_VERTICAL_MARGIN,
      .width = TH_PIN_FIELD_WIDTH,
      .height = TH_PIN_FIELD_HEIGHT
  };

  for (int i = 0; i < 12; i++) {
    screen_fill_area(&area, TH_COLOR_PIN_FIELD_BG);

    if ((i % 3) == 2) {
      area.x = TH_FIELD_MARGIN;
      area.y += TH_PIN_FIELD_HEIGHT + TH_PIN_FIELD_VERTICAL_MARGIN;
    } else {
      area.x += TH_PIN_FIELD_WIDTH + TH_PIN_FIELD_DIGIT_MARGIN;
    }
  }

  area.x = TH_FIELD_MARGIN;
  area.y = TH_TITLE_HEIGHT + TH_PIN_FIELD_VERTICAL_MARGIN;

  for (int i = 0; i < 12; i++) {
    while(ui_wait_keypress(portMAX_DELAY) != i) {
      continue;
    }

    screen_fill_area(&area, TH_COLOR_PIN_FIELD_SELECTED_BG);

    if ((i % 3) == 2) {
      area.x = TH_FIELD_MARGIN;
      area.y += TH_PIN_FIELD_HEIGHT + TH_PIN_FIELD_VERTICAL_MARGIN;
    } else {
      area.x += TH_PIN_FIELD_WIDTH + TH_PIN_FIELD_DIGIT_MARGIN;
    }
  }

  return ERR_OK;
}

static app_err_t test_colors() {
  dialog_title("R G B");

  screen_area_t area = {
      .x = 0,
      .y = TH_TITLE_HEIGHT,
      .width = (SCREEN_WIDTH / 3),
      .height = (SCREEN_HEIGHT - TH_TITLE_HEIGHT)
  };

  screen_fill_area(&area, SCREEN_COLOR_RED);

  area.x += (SCREEN_WIDTH / 3);
  screen_fill_area(&area, SCREEN_COLOR_GREEN);

  area.x += (SCREEN_WIDTH / 3);
  screen_fill_area(&area, SCREEN_COLOR_BLUE);

  while(1) {
    switch(ui_wait_keypress(portMAX_DELAY)) {
    case KEYPAD_KEY_CANCEL:
    case KEYPAD_KEY_BACK:
      return ERR_CANCEL;
    case KEYPAD_KEY_CONFIRM:
      return ERR_OK;
    default:
      break;
    }
  }

  return ERR_OK;
}

void ui_task_entry(void* pvParameters) {
  if (screen_init() != HAL_SUCCESS) {
    vTaskSuspend(NULL);
  }

  hal_pwm_set_dutycycle(PWM_BACKLIGHT, 50);
  screen_fill_area(&screen_fullarea, SCREEN_COLOR_BLACK);

  g_ui_cmd.received = 0;

  while(1) {
    if (!g_ui_cmd.received && ((ui_wait_event(portMAX_DELAY) & UI_CMD_EVT) == 0)) {
      continue;
    }

    g_ui_cmd.received = 0;

    switch(g_ui_cmd.type) {
    case UI_CMD_INFO:
      g_ui_cmd.result = dialog_info();
      break;
    case UI_CMD_QRSCAN:
      g_ui_cmd.result = qrscan_scan();
      break;
    case UI_CMD_INPUT_STRING:
      g_ui_cmd.result = test_keypad();
      break;
    case UI_CMD_LCD_BRIGHTNESS:
      g_ui_cmd.result = test_colors();
      break;
    default:
      g_ui_cmd.result = ERR_CANCEL;
      break;
    }

    if (!g_ui_cmd.received) {
      xTaskNotifyIndexed(APP_TASK(core), CORE_EVENT_IDX, CORE_UI_EVT, eSetBits);
    }
  }
}
#endif
