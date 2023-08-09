#include <string.h>
#include <ctype.h>

#include "common.h"
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

static app_err_t eip712_top_level(struct eip712_tokens* eip712, const jsmntok_t tokens[], int token_count, const char* json) {
  int found = 0;

  for (int i = 1; (i < (token_count - 1)) && (found != 0xf); i++) {
    if ((tokens[i].parent == 0) && (tokens[i].type == JSMN_STRING)) {
      switch(json[tokens[i].start]) {
      case 't': // types
        if (tokens[++i].type != JSMN_OBJECT) {
          return ERR_DATA;
        }
        eip712->types = i;
        found |= 1;
        break;
      case 'p': // primaryType
        if (tokens[++i].type != JSMN_STRING) {
          return ERR_DATA;
        }
        eip712->primary_type = i;
        found |= 2;
        break;
      case 'd': // domain
        if (tokens[++i].type != JSMN_OBJECT) {
          return ERR_DATA;
        }
        eip712->domain = i;
        found |= 4;
        break;
      case 'm': // message
        if (tokens[++i].type != JSMN_OBJECT) {
          return ERR_DATA;
        }
        eip712->domain = i;
        found |= 8;
        break;
      default:
        return ERR_DATA;
      }
    }
  }

  return found == 0xf ? ERR_OK : ERR_DATA;
}

static int eip712_parse_types(uint8_t* heap, size_t heap_size, int types_token, struct eip712_type types[], int types_count, jsmntok_t tokens[], int token_count, const char* json) {
  int current_type = 0;
  int fields_size = 0;

  int i = types_token + 1;
  while (current_type < types_count) {
    if (!(tokens[i].parent == types_token && tokens[i].type == JSMN_STRING)) {
      return -1;
    }

    types[current_type].name.len = (tokens[i].end - tokens[i].start);
    types[current_type].name.str = &json[tokens[i].start];

    if (!(tokens[++i].parent == types_token && tokens[i].type == JSMN_ARRAY)) {
      return -1;
    }

    types[current_type].field_count = tokens[i].size;
    size_t field_size = types[current_type].field_count * sizeof(struct eip712_field);
    fields_size += field_size;

    if (field_size > heap_size) {
      return -1;
    }

    types[current_type].fields = (struct eip712_field *) heap;
    heap += field_size;
    heap_size -= field_size;
    int fields_token = i;

    for (int j = 0; j < types[current_type].field_count; j++) {
      if (!(tokens[++i].parent == fields_token && tokens[i].type == JSMN_OBJECT)) {
        return -1;
      }

      int field_obj_token = i;

      for (int k = 0; k < 2; k++) {
        if (!(tokens[++i].parent == field_obj_token && tokens[i].type == JSMN_STRING)) {
          return -1;
        }

        char p = json[tokens[i].start];

        if (!(tokens[++i].parent == field_obj_token && tokens[i].type == JSMN_STRING)) {
          return -1;
        }

        switch(p) {
        case 'n':
          types[current_type].fields[j].name.len = tokens[i].end - tokens[i].start;
          types[current_type].fields[j].name.str = &json[tokens[i].start];
          break;
        case 't':
          types[current_type].fields[j].type.len = tokens[i].end - tokens[i].start;
          types[current_type].fields[j].type.str = &json[tokens[i].start];
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

  sha3_Update(sha3, (const uint8_t*) type->name.str, type->name.len);
  sep = '(';

  for(int j = 0; j < type->field_count; j++) {
    sha3_Update(sha3, &sep, 1);

    sha3_Update(sha3, (const uint8_t*) type->fields[j].type.str, type->fields[j].type.len);
    sep = ' ';
    sha3_Update(sha3, &sep, 1);
    sha3_Update(sha3, (const uint8_t*) type->fields[j].name.str, type->fields[j].name.len);

    sep = ',';
  }

  sep = ')';
  sha3_Update(sha3, &sep, 1);
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
    if (isupper((int) types[current_type].fields[j].type.str[0])) {
      int found = eip712_find_type(types, types_count, &types[current_type].fields[j].type);
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
  if (heap_size < sizeof(SHA3_CTX)) {
    return ERR_DATA;
  }

  SHA3_CTX* sha3 = (SHA3_CTX*) heap;

  for (int i = 0; i < types_count; i++) {
    sha3_256_Init(sha3);

    eip712_hash_type(sha3, &types[i]);

    int references[types_count];
    int reference_count = 0;

    if (eip712_collect_references(i, i, types, types_count, references, &reference_count) != ERR_OK) {
      return ERR_DATA;
    }

    for (int j = 0; j < reference_count; j++) {
      eip712_hash_type(sha3, &types[references[j]]);
    }

    sha3_Final(sha3, types[i].type_hash);
  }

  return ERR_OK;
}

app_err_t eip712_hash(SHA3_CTX *sha3, uint8_t* heap, size_t heap_size, const char* json, size_t json_len) {
  jsmntok_t* tokens = (jsmntok_t *) heap;
  jsmn_parser parser;
  jsmn_init(&parser);
  int token_count = jsmn_parse(&parser, json, json_len, tokens, heap_size/sizeof(jsmntok_t));

  if (token_count < 0) {
    return ERR_DATA;
  }

  size_t token_size = token_count * sizeof(jsmntok_t);
  heap += token_size;
  heap_size -= token_size;

  if (!((tokens[0].type == JSMN_OBJECT) && (tokens[0].size == 8))) {
    return ERR_DATA;
  }

  struct eip712_tokens eip712 = {0};
  if (eip712_top_level(&eip712, tokens, token_count, json) != ERR_OK) {
    return ERR_DATA;
  }

  size_t types_count = (tokens[eip712.types].size / 2);
  size_t types_len = types_count * sizeof(struct eip712_type);

  if (heap_size < types_len) {
    return ERR_DATA;
  }

  struct eip712_type* types = (struct eip712_type*) heap;
  heap += types_len;
  heap_size -= types_len;
  memset(types, 0, types_len);

  int fields_size = eip712_parse_types(heap, heap_size, eip712.types, types, types_count, tokens, token_count, json);

  if (fields_size < 0) {
    return ERR_DATA;
  }

  heap += fields_size;
  heap_size -= fields_size;

  if (eip712_hash_types(heap, heap_size, types, types_count) != ERR_OK) {
    return ERR_DATA;
  }

  return ERR_DATA;
}
