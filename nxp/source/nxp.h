#ifndef __HAL_NXP__
#define __HAL_NXP__

#include "fsl_dcp.h"
#include "fsl_common.h"
#include "cr_section_macros.h"

typedef dcp_hash_ctx_t hal_sha256_ctx_t;
#define SOFT_CRC32

#define APP_NOCACHE AT_NONCACHEABLE_SECTION_ALIGN
#define APP_RAMFUNC __RAMFUNC(SRAM_ITC)
#define CAMERA_BUFFER_ALIGN 64

#endif