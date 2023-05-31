#ifndef __HAL_STM32__
#define __HAL_STM32__

#include "stm32h563xx.h"

#define SOFT_SHA256

typedef uint32_t hal_crc32_ctx_t;

#define APP_NOCACHE APP_ALIGNED
#define APP_RAMFUNC
#define CAMERA_BUFFER_ALIGN 4

#define HAL_FLASH_SIZE FLASH_SIZE
#define HAL_FLASH_BLOCK_SIZE FLASH_SECTOR_SIZE
#define HAL_FLASH_WORD_SIZE 16
#define HAL_FLASH_BLOCK_COUNT (FLASH_SECTOR_NB << 1)
#define HAL_FLASH_ADDR FLASH_BASE

#endif
