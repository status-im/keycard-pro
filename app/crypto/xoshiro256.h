#ifndef _XOSHIRO_256_
#define _XOSHIRO_256_

#include <stdint.h>

#define XOSHIRO256_SEED_LEN 32

typedef struct {
  uint64_t s[4];
} xoshiro_ctx_t;

void xoshiro256_seed(xoshiro_ctx_t* ctx, const uint8_t seed[XOSHIRO256_SEED_LEN]);
uint64_t xoshiro256_next(xoshiro_ctx_t* ctx);

double xoshiro256_next_double(xoshiro_ctx_t* ctx);
uint32_t xoshiro256_next_int(xoshiro_ctx_t* ctx, uint32_t low, uint32_t high);
uint8_t xoshiro256_next_byte(xoshiro_ctx_t* ctx);

#endif
