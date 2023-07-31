#include "keys.h"

const uint8_t _DEV_AUTH_PUB[] = {

};

//TODO: this must be outside the fw
const uint8_t _FW_PUB[] = {

};

const uint8_t _DB_PUB[] = {

};

app_err_t key_read(key_type_t type, const uint8_t** out) {
  switch(type) {
  case DEV_AUTH_PRIV_KEY:
    break;
  case DEV_AUTH_SERVER_KEY:
    *out = _DEV_AUTH_PUB;
    break;
  case FW_VERIFICATION_KEY:
    *out = _FW_PUB;
    break;
  case DB_VERIFICATION_KEY:
    *out = _DB_PUB;
    break;
  default:
    return ERR_DATA;
  }

  return ERR_OK;
}
