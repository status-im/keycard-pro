#include "eip712.h"
#include "json/jsmn.h"

struct eip712_tokens {
  int types;
  int primary_type;
  int domain;
  int message;
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

app_err_t eip712_hash(SHA3_CTX *sha3, uint8_t* heap, size_t heap_size, const char* json, size_t json_len) {
  jsmntok_t* tokens = (jsmntok_t *) heap;
  jsmn_parser parser;
  jsmn_init(&parser);
  int token_count = jsmn_parse(&parser, json, json_len, tokens, heap_size/sizeof(jsmntok_t));

  if (token_count < 0) {
    return ERR_DATA;
  }

  size_t token_size = token_count * sizeof(jsmntok_t);
  heap = (uint8_t*) &heap[token_size];
  heap_size -= token_size;

  if (!((tokens[0].type == JSMN_OBJECT) && (tokens[0].size == 8))) {
    return ERR_DATA;
  }

  struct eip712_tokens eip712;
  if (eip712_top_level(&eip712, tokens, token_count, json) != ERR_OK) {
    return ERR_DATA;
  }

  return ERR_DATA;
}
