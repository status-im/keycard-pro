#include "sampler.h"
#include "crypto/sha2.h"

double RANDOM_SAMPLER_PROBS[32] = {
    1., 1./2., 1./3., 1./4., 1./5., 1./6., 1./7., 1./8.,
    1./9., 1./10., 1./11., 1./12., 1./13., 1./14., 1./15., 1./16,
    1./17., 1./18., 1./19., 1./20., 1./21., 1./22., 1./23., 1./24.,
    1./25., 1./26., 1./27., 1./28., 1./29., 1./30., 1./31., 1./32.
};

void random_sampler_init(int len, double* out_probs, int* out_aliases) {
  double sum = 0;

  for (int i = 0; i < len; i++) {
    sum += RANDOM_SAMPLER_PROBS[i];
  }

  double P[len];
  int S[len];
  int L[len];

  int s_len = 0;
  int l_len = 0;

  for (int i = 0; i < len; i++) {
    P[i] = (RANDOM_SAMPLER_PROBS[i] * (double)(len)) / sum;
  }

  for (int i = (len - 1); i >= 0; i--) {
    if (P[i] < 1.0) {
      S[s_len++] = i;
    } else {
      L[l_len++] = i;
    }
  }

  while(s_len && l_len) {
    int a = S[--s_len];
    int g = L[--l_len];
    out_probs[a] = P[a];
    out_aliases[a] = g;
    P[g] += P[a] - 1;

    if(P[g] < 1.0) {
      S[s_len++] = g;
    } else {
      L[l_len++] = g;
    }
  }

  while(l_len--) {
    out_probs[L[l_len]] = 1.0;
  }

  while(s_len--) {
    out_probs[S[s_len]] = 1.0;
  }
}

int random_sampler_next(xoshiro_ctx_t* rng_ctx, int len, double* probs, int* aliases) {
  double r1 = xoshiro256_next_double(rng_ctx);
  double r2 = xoshiro256_next_double(rng_ctx);
  int i = (int) (r1 * ((double) len));
  return r2 < probs[i] ? i : aliases[i];
}

uint32_t fountain_part_indexes(uint32_t seq, uint32_t crc, int len, double* probs, int* aliases) {
  uint32_t tmp[2];
  tmp[0] = seq;
  tmp[1] = crc;

  uint8_t seed[XOSHIRO256_SEED_LEN];
  sha256_Raw((uint8_t*) tmp, sizeof(tmp), seed);

  xoshiro_ctx_t xsr;
  xoshiro256_seed(&xsr, seed);

  int degree = random_sampler_next(&xsr, len, probs, aliases);
  uint32_t indexes = 0;

  while(degree--) {
    int count = xoshiro256_next_int(&xsr, 1, len--);
    int i = -1;

    while(count) {
      if (!((indexes >> i) & 1)) {
        count--;
      }

      i++;
    }

    indexes |= (1 << i);
  }

  return indexes;
}
