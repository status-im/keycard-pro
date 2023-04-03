#ifndef _APP_TASKS_H_
#define _APP_TASKS_H_

#include "FreeRTOS.h"
#include "task.h"
#include "common.h"

#define CORE_EVENT_IDX 0
#define CORE_UI_EVT 1
#define CORE_USB_EVT 2

APP_DEF_EXTERN_TASK(core);
APP_DEF_EXTERN_TASK(ui);

#endif
