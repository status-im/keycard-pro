#include "util.h"

uint32_t pad_iso9797_m1(uint8_t* data, uint8_t plen, uint32_t size) {
  uint32_t padding = plen - (size % plen);
  data[size] = 0x80;
  memset(&data[size+1], 0, (padding - 1));
  return padding + size;
}

uint32_t unpad_iso9797_m1(uint8_t* data, uint32_t size) {
  while(data[--size] != 0x80) {}
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
__attribute__((weak)) size_t strnlen(const char *s, size_t maxlen) {
  size_t res;

  for (res = 0; res < maxlen; res++) {
    if (s[res] == '\0') {
      break;
    }
  }

  return res;
}
