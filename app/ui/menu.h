#ifndef __MENU__
#define __MENU__

#include <stdint.h>
#include "i18n.h"
#include "error.h"

struct _menu;

typedef struct {
  i18n_str_id_t label_id;
  const struct _menu* submenu;
} menu_entry_t;

typedef struct _menu {
  uint8_t len;
  menu_entry_t entries[];
} menu_t;

extern const menu_t menu_mainmenu;

app_err_t menu_run();

#endif
