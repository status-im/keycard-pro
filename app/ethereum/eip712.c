#include <string.h>

#include "eip712.h"
#include "json/jsmn.h"

static const uint8_t EIP712_MAGIC[] = { 0x19, 0x01 };

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

static app_err_t eip712_top_level(struct eip712_tokens* eip712, jsmntok_t tokens[], int token_count, const char* json) {
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

app_err_t eip712_hash(SHA3_CTX *sha3, uint8_t* heap, size_t heap_size, const char* json, size_t json_len) {
  sha3_Update(sha3, EIP712_MAGIC, sizeof(EIP712_MAGIC));

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

  return ERR_DATA;
}
