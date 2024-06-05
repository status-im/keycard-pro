#ifndef _UI_INPUT_
#define _UI_INPUT_

#include <stdint.h>
#include "error.h"

app_err_t input_pin();
app_err_t input_puk();
app_err_t input_mnemonic();
app_err_t input_display_mnemonic();
app_err_t input_string();

#endif
