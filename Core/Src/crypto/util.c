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