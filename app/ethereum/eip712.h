#ifndef __EIP_712__
#define __EIP_712__

#include <stddef.h>
#include "crypto/sha3.h"
#include "error.h"
#include "json/jsmn.h"

struct eip712_tokens {
  int types;
  int primary_type;
  int domain;
  int message;
};

typedef struct {
  struct eip712_tokens index;
  int token_count;
  const jsmntok_t* tokens;
  const char* json;
} eip712_ctx_t;

app_err_t eip712_hash(eip712_ctx_t *ctx, SHA3_CTX *sha3, uint8_t* heap, size_t heap_size, const char* json, size_t json_len);

#endif
