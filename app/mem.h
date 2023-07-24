#ifndef _MEM_H_
#define _MEM_H_

#include <stdint.h>
#include "hal.h"

#define MEM_HEAP_SIZE (64 * 1024)

extern uint8_t g_mem_heap[MEM_HEAP_SIZE];
extern uint8_t g_flash_swap[HAL_FLASH_BLOCK_SIZE];

#endif
