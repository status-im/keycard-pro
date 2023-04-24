/*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#include <string.h>
#include "xoshiro256.h"

/* This is xoshiro256** 1.0, one of our all-purpose, rock-solid
   generators. It has excellent (sub-ns) speed, a state (256 bits) that is
   large enough for any parallel application, and it passes all tests we
   are aware of.

   For generating just floating-point numbers, xoshiro256+ is even faster.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s. */

static inline uint64_t rotl(const uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}

void xoshiro256_seed(xoshiro_ctx_t* ctx, const uint8_t seed[XOSHIRO256_SEED_LEN]) {
  memcpy(ctx->s, seed, XOSHIRO256_SEED_LEN);
}

uint64_t xoshiro256_next(xoshiro_ctx_t* ctx) {
  const uint64_t result = rotl(ctx->s[1] * 5, 7) * 9;

  const uint64_t t = ctx->s[1] << 17;

  ctx->s[2] ^= ctx->s[0];
  ctx->s[3] ^= ctx->s[1];
  ctx->s[1] ^= ctx->s[2];
  ctx->s[0] ^= ctx->s[3];

  ctx->s[2] ^= t;

  ctx->s[3] = rotl(ctx->s[3], 45);

  return result;
}

double xoshiro256_next_double(xoshiro_ctx_t* ctx) {
  return ((double) xoshiro256_next(ctx)) / ((double)(UINT64_MAX + 1));
}

uint32_t xoshiro256_next_int(xoshiro_ctx_t* ctx, uint32_t low, uint32_t high) {
  return (uint32_t)(xoshiro256_next_double(ctx) * (high - low + 1)) + low;
}

uint8_t xoshiro256_next_byte(xoshiro_ctx_t* ctx) {
  return (uint8_t) xoshiro256_next_int(ctx, 0, UINT8_MAX);
}
