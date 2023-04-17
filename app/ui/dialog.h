#ifndef _UI_DIALOG_
#define _UI_DIALOG_

#include <stdint.h>
#include "error.h"
#include "screen/screen.h"

app_err_t dialog_line(screen_text_ctx_t* ctx, const char* str, uint16_t line_height);
app_err_t dialog_separator(uint16_t yOff);

app_err_t dialog_title(const char* title);
app_err_t dialog_footer(uint16_t yOff);

app_err_t dialog_confirm_tx();

#endif
