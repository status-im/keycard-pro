#ifndef __HAL__
#define __HAL__

#include <stdint.h>

typedef enum {
	HAL_OK,
	HAL_ERROR
} hal_err_t;

hal_err_t hal_init();

#endif
