/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Common macros.
 */
#ifndef __APP_COMMON_H__
#define __APP_COMMON_H__

#define APP_ALIGNED(x, a) x __attribute__((aligned(a)))
#define APP_SECTION(x, s) x __attribute__((section(s)))
#define APP_ALWAYS_INLINE inline __attribute__((always_inline))
#define APP_WEAK __attribute__((weak))

#ifdef __MCUXPRESSO
#include "fsl_common.h"
#include "cr_section_macros.h"
#define APP_NOCACHE AT_NONCACHEABLE_SECTION_ALIGN
#define APP_RAMFUNC __RAMFUNC(SRAM_ITC)
#define CAMERA_BUFFER_ALIGN 64
#else
#error "Unknown platform"
#endif

#define OMG_BREAK() __asm__ volatile ("BKPT")

#define APP_MAX(a,b)                    \
({                                      \
    __typeof__ (a) _a = (a);            \
    __typeof__ (b) _b = (b);            \
    _a > _b ? _a : _b;                  \
})

#define APP_MIN(a,b)                    \
({                                      \
    __typeof__ (a) _a = (a);            \
    __typeof__ (b) _b = (b);            \
    _a < _b ? _a : _b;                  \
})

#define _APP_DEF_TASK(__NAME__, __STACK_SIZE__) \
  static StaticTask_t __NAME__##_task_memory; \
  static StackType_t __NAME__##_task_stack[__STACK_SIZE__]

#define _APP_CREATE_TASK(__NAME__, __PRIO__) \
  xTaskCreateStatic(__NAME__##_task_entry, #__NAME__, sizeof(__NAME__##_task_stack), NULL, __PRIO__, __NAME__##_task_stack, &__NAME__##_task_memory)

#define APP_DEF_TASK(__NAME__, __STACK_SIZE__) \
  _APP_DEF_TASK(__NAME__, __STACK_SIZE__); \
  TaskHandle_t __NAME__##_task

#define APP_CREATE_TASK(__NAME__, __PRIO__) \
  __NAME__##_task = _APP_CREATE_TASK(__NAME__, __PRIO__)

#define APP_DEF_CREATE_TASK(__NAME__, __PRIO__, __STACK_SIZE__) \
  _APP_DEF_TASK(__NAME__, __STACK_SIZE__); \
  _APP_CREATE_TASK(__NAME__, __PRIO__)

#endif //__APP_COMMON_H__
