#ifndef __HAL_STM32__
#define __HAL_STM32__

#include "stm32h573xx.h"

typedef uint32_t hal_sha256_ctx_t;
typedef uint32_t hal_crc32_ctx_t;

#define APP_NOCACHE APP_ALIGNED
#define APP_RAMFUNC
#define CAMERA_BUFFER_ALIGN 4

#define HAL_FLASH_SIZE FLASH_SIZE
#define HAL_FLASH_BLOCK_SIZE FLASH_SECTOR_SIZE
#define HAL_FLASH_WORD_SIZE 16
#define HAL_FLASH_BLOCK_COUNT (FLASH_SECTOR_NB << 1)
#define HAL_FLASH_ADDR FLASH_BASE

#define HAL_FLASH_DATA_BLOCK_COUNT 96

#endif
