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

typedef struct {
  const char* address;
  const char* name;
  uint32_t chainID;
} eip712_domain_t;

app_err_t eip712_hash(eip712_ctx_t *ctx, SHA3_CTX *sha3, uint8_t* heap, size_t heap_size, const char* json, size_t json_len);
size_t eip712_to_string(const eip712_ctx_t* ctx, uint8_t* out);
app_err_t eip712_extract_domain(const eip712_ctx_t* ctx, eip712_domain_t* out);

#endif
