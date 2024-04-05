#include <string.h>
#include <ctype.h>

#include "common.h"
#include "crypto/util.h"
#include "eip712.h"
#include "json/jsmn.h"

struct eip712_tokens {
  int types;
  int primary_type;
  int domain;
  int message;
};

struct eip712_string {
  const char *str;
  size_t len;
};

struct eip712_field {
  struct eip712_string name;
  struct eip712_string type;
};

struct eip712_type {
  struct eip712_string name;
  int field_count;
  struct eip712_field* fields;
  uint8_t type_hash[SHA3_256_DIGEST_LENGTH];
};

struct eip712_ctx {
  struct eip712_tokens index;
  int token_count;
  const jsmntok_t* tokens;
  const char* json;
};

// ugly stuff
#define ALIGN_HEAP(__HEAP__, __HEAP_SIZE__) __HEAP__ = (uint8_t*) (((uint32_t)(__HEAP__ + 3)) & ~0x3); __HEAP_SIZE__ &= ~0x3

static app_err_t eip712_hash_struct(uint8_t out[32], uint8_t* heap, size_t heap_size, int type, const struct eip712_type types[], int types_count, int data, const struct eip712_ctx* ctx);

static app_err_t eip712_top_level(struct eip712_ctx* ctx) {
  int found = 0;

  for (int i = 1; (i < (ctx->token_count - 1)) && (found != 0xf); i++) {
    if ((ctx->tokens[i].parent == 0) && (ctx->tokens[i].type == JSMN_STRING)) {
      switch(ctx->json[ctx->tokens[i].start]) {
      case 't': // types
        if (ctx->tokens[++i].type != JSMN_OBJECT) {
          return ERR_DATA;
        }
        ctx->index.types = i;
        found |= 1;
        break;
      case 'p': // primaryType
        if (ctx->tokens[++i].type != JSMN_STRING) {
          return ERR_DATA;
        }
        ctx->index.primary_type = i;
        found |= 2;
        break;
      case 'd': // domain
        if (ctx->tokens[++i].type != JSMN_OBJECT) {
          return ERR_DATA;
        }
        ctx->index.domain = i;
        found |= 4;
        break;
      case 'm': // message
        if (ctx->tokens[++i].type != JSMN_OBJECT) {
          return ERR_DATA;
        }
        ctx->index.message = i;
        found |= 8;
        break;
      default:
        return ERR_DATA;
      }
    }
  }

  return found == 0xf ? ERR_OK : ERR_DATA;
}

static int eip712_parse_types(uint8_t* heap, size_t heap_size, int types_token, struct eip712_type types[], int types_count, const struct eip712_ctx* ctx) {
  int current_type = 0;
  int fields_size = 0;

  int i = types_token;
  while (current_type < types_count) {
    if (!(ctx->tokens[++i].parent == types_token && ctx->tokens[i].type == JSMN_STRING)) {
      return -1;
    }

    types[current_type].name.len = (ctx->tokens[i].end - ctx->tokens[i].start);
    types[current_type].name.str = &ctx->json[ctx->tokens[i].start];

    if (ctx->tokens[++i].type != JSMN_ARRAY) {
      return -1;
    }

    types[current_type].field_count = ctx->tokens[i].size;
    size_t field_size = types[current_type].field_count * sizeof(struct eip712_field);
    fields_size += field_size;

    ALIGN_HEAP(heap, heap_size);

    if (field_size > heap_size) {
      return -1;
    }

    types[current_type].fields = (struct eip712_field *) heap;
    heap += field_size;
    heap_size -= field_size;
    int fields_token = i;

    for (int j = 0; j < types[current_type].field_count; j++) {
      if (!(ctx->tokens[++i].parent == fields_token && ctx->tokens[i].type == JSMN_OBJECT)) {
        return -1;
      }

      int field_obj_token = i;

      for (int k = 0; k < 2; k++) {
        if (!(ctx->tokens[++i].parent == field_obj_token && ctx->tokens[i].type == JSMN_STRING)) {
          return -1;
        }

        char p = ctx->json[ctx->tokens[i].start];

        if (ctx->tokens[++i].type != JSMN_STRING) {
          return -1;
        }

        switch(p) {
        case 'n':
          types[current_type].fields[j].name.len = ctx->tokens[i].end - ctx->tokens[i].start;
          types[current_type].fields[j].name.str = &ctx->json[ctx->tokens[i].start];
          break;
        case 't':
          types[current_type].fields[j].type.len = ctx->tokens[i].end - ctx->tokens[i].start;
          types[current_type].fields[j].type.str = &ctx->json[ctx->tokens[i].start];
          break;
        default:
          return -1;
        }
      }
    }

    current_type++;
  }

  return fields_size;
}

static inline int eip712_strcmp(const struct eip712_string* a, const struct eip712_string* b) {
  int res = memcmp(a->str, b->str, APP_MIN(a->len, b->len));
  return res != 0 ? res : a->len - b->len;
}

static inline int eip712_streq(const struct eip712_string* a, const struct eip712_string* b) {
  if (a->len == b->len) {
    return memcmp(a->str, b->str, a->len) == 0;
  }

  return 0;
}

static inline int eip712_is_struct(const struct eip712_string* type) {
  return isupper((int) type->str[0]) || type->str[0] == '_';
}

static inline int eip712_is_array(const struct eip712_string* type) {
  return type->str[type->len - 1] == ']';
}

static app_err_t eip712_inner_type(const struct eip712_string* array_type, struct eip712_string* inner_type) {
  inner_type->str = array_type->str;
  inner_type->len = array_type->len;

  while(inner_type->str[--inner_type->len] != '[') {
    if (inner_type->len == 0) {
      return ERR_DATA;
    }
  }

  return ERR_OK;
}

static int eip712_find_type(const struct eip712_type types[], int types_count, const struct eip712_string* type) {
  for (int i = 0; i < types_count; i++) {
    if (eip712_streq(type, &types[i].name)) {
      return i;
    }
  }

  return -1;
}

static void eip712_hash_type(SHA3_CTX* sha3, const struct eip712_type* type) {
  uint8_t sep;

  keccak_Update(sha3, (const uint8_t*) type->name.str, type->name.len);
  sep = '(';

  for(int j = 0; j < type->field_count; j++) {
    keccak_Update(sha3, &sep, 1);

    keccak_Update(sha3, (const uint8_t*) type->fields[j].type.str, type->fields[j].type.len);
    sep = ' ';
    keccak_Update(sha3, &sep, 1);
    keccak_Update(sha3, (const uint8_t*) type->fields[j].name.str, type->fields[j].name.len);

    sep = ',';
  }

  sep = ')';
  keccak_Update(sha3, &sep, 1);
}

static int eip712_insert(int insert, const struct eip712_type types[], int types_count, int references[], int *references_count) {
  for(int i = 0; i < *references_count; i++) {
    if (insert == references[i]) {
      return 0;
    }

    int cmp = eip712_strcmp(&types[insert].name, &types[references[i]].name);

    if (cmp < 0) {
      for (int j = (*references_count)++; j > i; j--) {
        references[j] = references[j-1];
      }

      references[i] = insert;

      return 1;
    }
  }

  references[(*references_count)++] = insert;

  return 1;
}

static app_err_t eip712_collect_references(int main_type, int current_type, const struct eip712_type types[], int types_count, int references[], int *references_count) {
  for (int j = 0; j < types[current_type].field_count; j++) {
    const struct eip712_string* field_type = &types[current_type].fields[j].type;

    if (eip712_is_struct(field_type)) {
      struct eip712_string inner_type;

      if (eip712_is_array(field_type)) {
        if (eip712_inner_type(field_type, &inner_type) != ERR_OK) {
          return ERR_DATA;
        }

        field_type = &inner_type;
      }

      int found = eip712_find_type(types, types_count, field_type);
      if (found == -1 || (main_type == found)) {
        return ERR_DATA;
      }

      if (eip712_insert(found, types, types_count, references, references_count)) {
        eip712_collect_references(main_type, found, types, types_count, references, references_count);
      }
    }
  }

  return ERR_OK;
}

static app_err_t eip712_hash_types(uint8_t* heap, size_t heap_size, struct eip712_type types[], int types_count) {
  ALIGN_HEAP(heap, heap_size);

  if (heap_size < sizeof(SHA3_CTX)) {
    return ERR_DATA;
  }

  SHA3_CTX* sha3 = (SHA3_CTX*) heap;

  for (int i = 0; i < types_count; i++) {
    keccak_256_Init(sha3);

    eip712_hash_type(sha3, &types[i]);

    int references[types_count];
    int reference_count = 0;

    if (eip712_collect_references(i, i, types, types_count, references, &reference_count) != ERR_OK) {
      return ERR_DATA;
    }

    for (int j = 0; j < reference_count; j++) {
      eip712_hash_type(sha3, &types[references[j]]);
    }

    keccak_Final(sha3, types[i].type_hash);
  }

  return ERR_OK;
}

static int eip712_hash_find_data(const struct eip712_string* name, int start, const struct eip712_ctx* ctx) {

  for (int i = start + 1; i < ctx->token_count; i++) {
    if (ctx->tokens[i].parent == start) {
      if (ctx->tokens[i].type != JSMN_STRING) {
        return -1;
      }

      struct eip712_string key_name;
      key_name.str = &ctx->json[ctx->tokens[i].start];
      key_name.len = ctx->tokens[i].end - ctx->tokens[i].start;

      if (eip712_streq(name, &key_name)) {
        return i + 1;
      }
    }
  }

  return -1;
}

// This macro must only be used in the below functions
#define __DECL_SHA3_CTX() \
  ALIGN_HEAP(heap, heap_size); \
  if (heap_size < sizeof(SHA3_CTX)) { \
    return ERR_DATA; \
  } \
  SHA3_CTX* sha3 = (SHA3_CTX*) heap; \
  heap += sizeof(SHA3_CTX); \
  heap_size -= sizeof(SHA3_CTX); \
  keccak_256_Init(sha3)

static app_err_t eip712_encode_field(uint8_t out[32], uint8_t* heap, size_t heap_size, const struct eip712_string *field_type, int field_val, const struct eip712_type types[], int types_count, const struct eip712_ctx* ctx) {
  if (eip712_is_array(field_type)) {
    if (ctx->tokens[field_val].type != JSMN_ARRAY) {
      return ERR_DATA;
    }

    uint8_t tmp[32];

    __DECL_SHA3_CTX();

    struct eip712_string inner_type;
    if (eip712_inner_type(field_type, &inner_type) != ERR_OK) {
      return ERR_DATA;
    }

    int inner_field = field_val + 1;

    for (int i = 0; i < ctx->tokens[field_val].size; i++) {
      while(ctx->tokens[inner_field].parent != field_val) {
        if (++inner_field == ctx->token_count) {
          return ERR_DATA;
        }
      }

      if (eip712_encode_field(tmp, heap, heap_size, &inner_type, inner_field++, types, types_count, ctx) != ERR_OK) {
        return ERR_DATA;
      }

      keccak_Update(sha3, tmp, 32);
    }

    keccak_Final(sha3, out);
  } else if (eip712_is_struct(field_type)) {
    int type = eip712_find_type(types, types_count, field_type);

    if (type == -1) {
      return ERR_DATA;
    }

    if (eip712_hash_struct(out, heap, heap_size, type, types, types_count, field_val, ctx) != ERR_OK) {
      return ERR_DATA;
    }
  } else if (field_type->len == 6 && !strncmp(field_type->str, "string", 6)) {
    if (ctx->tokens[field_val].type != JSMN_STRING) {
      return ERR_DATA;
    }

    __DECL_SHA3_CTX();

    keccak_Update(sha3, (uint8_t*) &ctx->json[ctx->tokens[field_val].start], ctx->tokens[field_val].end - ctx->tokens[field_val].start);
    keccak_Final(sha3, out);
  } else if (field_type->len == 5 && !strncmp(field_type->str, "bytes", 5)) {
    if (ctx->tokens[field_val].type != JSMN_STRING) {
      return ERR_DATA;
    }

    struct eip712_string tmpstr;
    tmpstr.str = &ctx->json[ctx->tokens[field_val].start];
    tmpstr.len = ctx->tokens[field_val].end - ctx->tokens[field_val].start;

    __DECL_SHA3_CTX();

    if ((tmpstr.len > 2) && (tmpstr.str[0] == '0') && (tmpstr.str[1] == 'x')) {
      tmpstr.str += 2;
      tmpstr.len -= 2;

      while(tmpstr.len) {
        int len = APP_MIN(tmpstr.len, (64 - (tmpstr.len & 1)));
        if (!base16_decode(tmpstr.str, out, len)) {
          return ERR_DATA;
        }
        tmpstr.len -= len;
        tmpstr.str += len;

        keccak_Update(sha3, out, (len >> 1));
      }

      keccak_Final(sha3, out);
    } else {
      __DECL_SHA3_CTX();
      keccak_Update(sha3, (uint8_t*) tmpstr.str, tmpstr.len);
      keccak_Final(sha3, out);
    }
  } else if (field_type->len == 4 && !strncmp(field_type->str, "bool", 4)) {
    if (ctx->tokens[field_val].type != JSMN_PRIMITIVE) {
      return ERR_DATA;
    }

    memset(out, 0, 32);
    out[31] = ctx->json[ctx->tokens[field_val].start] == 't';
  } else {
    struct eip712_string tmpstr;
    tmpstr.str = &ctx->json[ctx->tokens[field_val].start];
    tmpstr.len = ctx->tokens[field_val].end - ctx->tokens[field_val].start;

    if ((tmpstr.len > 2) && (tmpstr.str[0] == '0') && (tmpstr.str[1] == 'x')) {
      int out_len = ((tmpstr.len - 1) >> 1);
      int padding = 32 - out_len;
      int offset;

      // bytesX are right padded, others are left padded
      if (field_type->str[0] == 'b') {
        offset = 0;
        memset(&out[offset], 0, padding);
      } else {
        offset = padding;
        memset(out, 0, padding);
      }

      if (!base16_decode(&tmpstr.str[2], &out[offset], (tmpstr.len - 2))) {
        return ERR_DATA;
      }
    } else {
      if (!atoi256BE(tmpstr.str, tmpstr.len, out)) {
        return ERR_DATA;
      }
    }
  }

  return ERR_OK;
}

static app_err_t eip712_hash_struct(uint8_t out[32], uint8_t* heap, size_t heap_size, int type, const struct eip712_type types[], int types_count, int data, const struct eip712_ctx* ctx) {
  const struct eip712_type *t = &types[type];

  __DECL_SHA3_CTX();

  keccak_Update(sha3, t->type_hash, SHA3_256_DIGEST_LENGTH);

  for (int i = 0; i < t->field_count; i++) {
    int field_val = eip712_hash_find_data(&t->fields[i].name, data, ctx);
    if (field_val == -1) {
      return ERR_DATA;
    }

    APP_ALIGNED(uint8_t field[32], 4);
    if (eip712_encode_field(field, heap, heap_size, &t->fields[i].type, field_val, types, types_count, ctx) != ERR_OK) {
      return ERR_DATA;
    }

    keccak_Update(sha3, field, 32);
  }

  keccak_Final(sha3, out);

  return ERR_OK;
}

app_err_t eip712_hash(SHA3_CTX *sha3, uint8_t* heap, size_t heap_size, const char* json, size_t json_len) {
  ALIGN_HEAP(heap, heap_size);
  struct eip712_ctx *ctx = (struct eip712_ctx *) heap;
  memset(ctx, 0, sizeof(struct eip712_ctx));
  heap += sizeof(struct eip712_ctx);
  heap_size -= sizeof(struct eip712_ctx);
  ctx->json = json;
  ALIGN_HEAP(heap, heap_size);
  ctx->tokens = (jsmntok_t *) heap;
  jsmn_parser parser;
  jsmn_init(&parser);
  ctx->token_count = jsmn_parse(&parser, json, json_len, (jsmntok_t*) ctx->tokens, heap_size/sizeof(jsmntok_t));

  if (ctx->token_count < 0) {
    return ERR_DATA;
  }

  size_t token_size = ctx->token_count * sizeof(jsmntok_t);
  heap += token_size;
  heap_size -= token_size;

  if (!((ctx->tokens[0].type == JSMN_OBJECT) && (ctx->tokens[0].size == 4))) {
    return ERR_DATA;
  }

  if (eip712_top_level(ctx) != ERR_OK) {
    return ERR_DATA;
  }

  int types_count = ctx->tokens[ctx->index.types].size;
  size_t types_len = types_count * sizeof(struct eip712_type);
  ALIGN_HEAP(heap, heap_size);

  if (heap_size < types_len) {
    return ERR_DATA;
  }

  struct eip712_type* types = (struct eip712_type*) heap;
  heap += types_len;
  heap_size -= types_len;
  memset(types, 0, types_len);

  int fields_size = eip712_parse_types(heap, heap_size, ctx->index.types, types, types_count, ctx);

  if (fields_size < 0) {
    return ERR_DATA;
  }

  heap += fields_size;
  heap_size -= fields_size;

  if (eip712_hash_types(heap, heap_size, types, types_count) != ERR_OK) {
    return ERR_DATA;
  }

  struct eip712_string tmpstr;
  tmpstr.str = "EIP712Domain";
  tmpstr.len = 12;

  int struct_idx = eip712_find_type(types, types_count, &tmpstr);

  if (struct_idx == -1) {
    return ERR_DATA;
  }

  uint8_t tmp[32];

  eip712_hash_struct(tmp, heap, heap_size, struct_idx, types, types_count, ctx->index.domain, ctx);
  keccak_Update(sha3, tmp, 32);

  tmpstr.str = &json[ctx->tokens[ctx->index.primary_type].start];
  tmpstr.len = (ctx->tokens[ctx->index.primary_type].end - ctx->tokens[ctx->index.primary_type].start);
  struct_idx = eip712_find_type(types, types_count, &tmpstr);

  if (struct_idx == -1) {
    return ERR_DATA;
  }

  app_err_t err = eip712_hash_struct(tmp, heap, heap_size, struct_idx, types, types_count, ctx->index.message, ctx);
  keccak_Update(sha3, tmp, 32);

  return err;
}
