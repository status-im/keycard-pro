#include <ctype.h>
#include "ur.h"
#include "bytewords.h"

app_err_t ur_process_part(ur_t* ur, const uint8_t* in, size_t in_len) {
  if (in_len < 10) {
    return ERR_DATA;
  }

  if (!(toupper(in[0]) == 'U' && toupper(in[1]) == 'R' && in[2] == ':')) {
    return ERR_DATA;
  }

  size_t offset;
  uint32_t tmp = 0;

  for (offset = 3; offset < in_len; offset++) {
    if (in[offset] == '/') {
      break;
    }

    tmp += toupper(in[offset]);
  }

  if (offset == in_len) {
    return ERR_DATA;
  }

  // we assume we are dealing with a supported type and moving the
  // case where we are not to actual payload validation
  ur->type = (ur_type_t)(((tmp * 8238429) >> 30) & 0x3);

  if (isdigit(in[++offset])) {
    ur->is_multipart = 1;
    while((offset < in_len) && in[offset++] != '/') { /*we don't need this*/}
    if (offset == in_len) {
      return ERR_DATA;
    }

    //TODO: remove this when multipart support is implemented
    return ERR_CANCEL;
  } else {
    ur->is_multipart = 0;
    ur->is_complete = 1;
  }

  ur->data_len = bytewords_decode(&in[offset], (in_len - offset), ur->data, UR_MAX_DATA_LEN);

  return ur->data_len == 0 ? ERR_DATA : ERR_OK;
}
