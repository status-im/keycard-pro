#ifndef __PAD_H__
#define __PAD_H__

#include <string.h>
#include <stdint.h>

uint32_t pad_iso9797_m1(uint8_t* data, uint8_t plen, uint32_t size);
uint32_t unpad_iso9797_m1(uint8_t* data, uint32_t size);
uint8_t* u32toa(uint32_t value, uint8_t* buf, uint32_t len);

static inline int memcmp_ct(const uint8_t* a, const uint8_t* b, size_t length) {
  int compareSum = 0;

  for (int i = 0; i < length; i++) {
    compareSum |= a[i] ^ b[i];
  }

  return compareSum;
}

static inline uint32_t rev32(uint32_t value) {
  __asm__ volatile (
      "REV %0, %0  \n"
      : "+r" (value)
      :
  );
  return value;
}

static inline void rev32_all(uint32_t* out, uint32_t* in, size_t len) {
  len >>= 2;
  for (int i = 0; i < len; i++) {
    out[i] = rev32(in[i]);
  }
}

#endif