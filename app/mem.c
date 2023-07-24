#include "mem.h"
#include "common.h"

APP_ALIGNED(uint8_t g_mem_heap[MEM_HEAP_SIZE], 4);
APP_ALIGNED(uint8_t g_flash_swap[HAL_FLASH_BLOCK_SIZE], 4);
