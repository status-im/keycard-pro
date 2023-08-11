#include "util.h"
#include "common.h"

uint32_t pad_iso9797_m1(uint8_t* data, uint8_t plen, uint32_t size) {
  uint32_t padding = plen - (size % plen);
  data[size] = 0x80;
  memset(&data[size+1], 0, (padding - 1));
  return padding + size;
}

uint32_t unpad_iso9797_m1(uint8_t* data, uint32_t size) {
  while((size > 0) && data[--size] != 0x80) {}
  return size;
}

uint8_t* u32toa(uint32_t value, uint8_t* buf, uint32_t len) {
  uint8_t *p = &buf[len - 1];
  *p-- = '\0';

  do {
    *p-- = (value % 10) + '0';
    value /= 10;
  } while (value > 0);

  return (p + 1);
}

// if the used C library has it, prefer its version instead
APP_WEAK size_t strnlen(const char *s, size_t maxlen) {
  size_t res;

  for (res = 0; res < maxlen; res++) {
    if (s[res] == '\0') {
      break;
    }
  }

  return res;
}

static inline uint8_t base16_hexlet_decode(char c) {
  if ((c >= '0') && (c <= '9')) {
    return c - '0';
  } else if ((c >= 'a') && (c <= 'f')) {
    return 10 + (c - 'a');
  } else if ((c >= 'A') && (c <= 'F')) {
    return 10 + (c - 'A');
  }

  return 0xff;
}

bool base16_decode(const char* s, uint8_t* out, size_t s_len) {
  if (s_len & 1) {
    out[0] = base16_hexlet_decode(s[0]);

    if (out[0] == 0xff) {
      return false;
    }

    return base16_decode(&s[1], &out[1], s_len - 1);
  }

  const char *s_end = &s[s_len];

  while(s < s_end) {
    uint8_t nh = base16_hexlet_decode(*(s++));
    uint8_t nl = base16_hexlet_decode(*(s++));
    if (nl == 0xff || nh == 0xff) {
      return false;
    } else {
      *(out++) = (nh << 4) | nl;
    }
  }

  return true;
}

bool atoi64(const char* str, size_t len, int64_t* res) {
  int i;
  int sign;

  if (str[0] == '-') {
    sign = -1;
    i = 1;
  } else if (str[0] == '+') {
    sign = 1;
    i = 1;
  } else {
    sign = 1;
    i = 0;
  }

  *res = 0;

  while(i < len) {
    if (!(str[i] >= '0' && str[i] <= '9')) {
      return false;
    }

    *res = (10 * (*res)) + (str[i++] - '0');
  }

  *res = (*res) * sign;

  return true;
}
