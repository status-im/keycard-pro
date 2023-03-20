#include <ctype.h>
#include "ur.h"
#include "bytewords.h"

hal_err_t ur_process_part(ur_t* ur, uint8_t* in, size_t in_len) {
  if (in_len < 10) {
    return HAL_ERROR;
  }

  if (!(tolower(in[0]) == 'u' && tolower(in[1]) == 'r' && in[2] == ':')) {
    return HAL_ERROR;
  }

  size_t offset;
  uint32_t tmp = 0;

  for (offset = 3; offset < in_len; offset++) {
    if (in[offset] == '/') {
      break;
    }

    tmp += tolower(in[offset]);
  }

  if (offset == in_len) {
    return HAL_ERROR;
  }

  // we assume we are dealing with a supported type and moving the
  // case where we are not to actual payload validation
  ur->type = (ur_type_t)(((tmp * 5961553) >> 30) & 0x3);

  if (isdigit(in[++offset])) {
    ur->is_multipart = 1;
    while((offset < in_len) && in[offset++] != '/') { /*we don't need this*/}
    if (offset == in_len) {
      return HAL_ERROR;
    }

    //TODO: remove this when multipart support is implemented
    return HAL_ERROR;
  } else {
    ur->is_multipart = 0;
    ur->is_complete = 1;
  }

  ur->data_len = bytewords_decode(&in[offset], (in_len - offset), ur->data, UR_MAX_DATA_LEN);

  return ur->data_len == 0 ? HAL_ERROR : HAL_OK;
}
