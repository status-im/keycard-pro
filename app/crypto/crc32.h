#ifndef _CRC32_
#define _CRC32_
#include <stdint.h>
#include "hal.h"

#ifdef SOFT_CRC32
typedef uint32_t crc32_ctx_t;
#else
typedef hal_crc32_ctx_t crc32_ctx_t;
#endif

#ifdef SOFT_CRC32
void crc32_init(crc32_ctx_t* ctx);
void crc32_update_one(crc32_ctx_t* ctx, uint8_t b);
void crc32_finish(crc32_ctx_t* ctx, uint32_t* out);
#else
static inline void crc32_init(crc32_ctx_t* ctx) {
  hal_crc32_init(ctx);
}

static inline void crc32_update_one(crc32_ctx_t* ctx, uint8_t b) {
 hal_crc32_update(ctx, b);
}

static inline void crc32_finish(crc32_ctx_t* ctx, uint32_t* out) {
  hal_crc32_finish(ctx, out);
}
#endif

void crc32_update(crc32_ctx_t* ctx, const uint8_t* data, size_t len);

#endif
