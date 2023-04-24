#include "sampler.h"

const double* RANDOM_SAMPLER_PROBS[RANDOM_SAMPLER_MAX_LEN] = {
    //TODO: put values
};

const int* RANDOM_SAMPLER_ALIASES[RANDOM_SAMPLER_MAX_LEN] = {
    //TODO: put values
};

void random_sampler_gen_lookup(double in[], int len) {
  double sum = 0;

  for (int i = 0; i < len; i++) {
    sum += in[i];
  }

  double P[len];
  int S[len];
  int L[len];

  double probs[len];
  int aliases[len];

  int s_len = 0;
  int l_len = 0;

  for (int i = 0; i < len; i++) {
    P[i] = (in[i] * (double)(len)) / sum;
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
    probs[a] = P[a];
    aliases[a] = g;
    P[g] += P[a] - 1;

    if(P[g] < 1.0) {
      S[s_len++] = g;
    } else {
      L[l_len++] = g;
    }
  }

  while(l_len--) {
    probs[L[l_len]] = 1.0;
  }

  while(s_len--) {
    probs[S[s_len]] = 1.0;
  }
}

void random_sampler_gen_all_lookups() {
  double probs[RANDOM_SAMPLER_MAX_LEN];

  for (int i = 0; i < RANDOM_SAMPLER_MAX_LEN; i++) {
    probs[i] = 1.0/((double)(i + 1));
  }

  for (int i = 2; i <= RANDOM_SAMPLER_MAX_LEN; i++) {
    random_sampler_gen_lookup(probs, i);
  }
}

int random_sampler_next(xoshiro_ctx_t* rng_ctx, int len) {
  if ((len < 2) || (len > RANDOM_SAMPLER_MAX_LEN)) {
    return -1;
  }

  double r1 = xoshiro256_next_double(rng_ctx);
  double r2 = xoshiro256_next_double(rng_ctx);
  int i = (int) (r1 * ((double) len));
  return r2 < RANDOM_SAMPLER_PROBS[len][i] ? i : RANDOM_SAMPLER_ALIASES[len][i];
}
