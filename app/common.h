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

#ifdef __MCUXPRESSO
#include "fsl_debug_console.h"
#define APP_NOCACHE AT_NONCACHEABLE_SECTION_ALIGN
#define APP_PRINTF PRINTF
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
#endif //__APP_COMMON_H__
