#ifndef __EIP_712__
#define __EIP_712__

#include <stddef.h>
#include "crypto/sha3.h"
#include "error.h"

app_err_t eip712_hash(SHA3_CTX *sha3, uint8_t* heap, size_t heap_size, const char* json, size_t json_len);

#endif
