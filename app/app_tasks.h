#ifndef _APP_TASKS_H_
#define _APP_TASKS_H_

#include "FreeRTOS.h"
#include "task.h"
#include "common.h"

APP_DEF_EXTERN_TASK(core);
APP_DEF_EXTERN_TASK(ui);

#endif
