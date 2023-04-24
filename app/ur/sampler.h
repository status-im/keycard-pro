#ifndef _RANDOM_SAMPLER_
#define _RANDOM_SAMPLER_
#include "crypto/xoshiro256.h"

#define RANDOM_SAMPLER_MAX_LEN 20

int random_sampler_next(xoshiro_ctx_t* rng_ctx, int len);

#endif
