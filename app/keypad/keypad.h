#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#include <stdint.h>

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

typedef enum {
  KEYPAD_KEY_1 = 0,
  KEYPAD_KEY_2,
  KEYPAD_KEY_3,
  KEYPAD_KEY_UNUSED_1,

  KEYPAD_KEY_4,
  KEYPAD_KEY_5,
  KEYPAD_KEY_6,
  KEYPAD_KEY_UNUSED_2,

  KEYPAD_KEY_7,
  KEYPAD_KEY_8,
  KEYPAD_KEY_9,
  KEYPAD_KEY_UNUSED_3,

  KEYPAD_KEY_CONFIRM,
  KEYPAD_KEY_0,
  KEYPAD_KEY_BACK,
  KEYPAD_KEY_UNUSED_4,

  KEYPAD_KEY_CANCEL,
  KEYPAD_KEY_INVALID = UINT32_MAX
} keypad_key_t;

typedef struct {
  uint32_t matrix_state[KEYPAD_ROWS * KEYPAD_COLS];
  uint32_t last_key_duration;
  keypad_key_t last_key;
  uint8_t current_row;
} keypad_t;

#define KEYPAD_KEY_UP KEYPAD_KEY_2
#define KEYPAD_KEY_LEFT KEYPAD_KEY_4
#define KEYPAD_KEY_RIGHT KEYPAD_KEY_6
#define KEYPAD_KEY_DOWN KEYPAD_KEY_8

void keypad_scan_tick();

#endif
