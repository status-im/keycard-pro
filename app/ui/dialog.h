#ifndef _UI_DIALOG_
#define _UI_DIALOG_

#include <stdint.h>
#include "error.h"
#include "screen/screen.h"

app_err_t dialog_line(screen_text_ctx_t* ctx, const char* str, uint16_t line_height);
app_err_t dialog_separator(uint16_t yOff);

app_err_t dialog_title(const char* title);
app_err_t dialog_title_colors(const char* title, uint16_t bg, uint16_t fg, uint16_t icon);
app_err_t dialog_footer(uint16_t yOff);
app_err_t dialog_margin(uint16_t yOff, uint16_t height);

app_err_t dialog_confirm_tx();
app_err_t dialog_confirm_msg();
app_err_t dialog_confirm_eip712();

app_err_t dialog_internal_info(const char* title, const char* msg);
app_err_t dialog_info();
app_err_t dialog_dev_auth();

#endif
