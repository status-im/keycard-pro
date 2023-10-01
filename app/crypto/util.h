#ifndef __PAD_H__
#define __PAD_H__

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

extern const char* const HEX_DIGITS;

uint32_t pad_iso9797_m1(uint8_t* data, uint8_t plen, uint32_t size);
uint32_t unpad_iso9797_m1(uint8_t* data, uint32_t size);
uint8_t* u32toa(uint32_t value, uint8_t* buf, uint32_t len);

size_t strnlen(const char *s, size_t maxlen);
bool base16_decode(const char* s, uint8_t* out, size_t s_len);
void base16_encode(const uint8_t* data, char* out, size_t len);
bool atoi256BE(const char* str, size_t len, uint8_t out[32]);

static inline int memcmp_ct(const uint8_t* a, const uint8_t* b, size_t length) {
  int compareSum = 0;

  for (int i = 0; i < length; i++) {
    compareSum |= a[i] ^ b[i];
  }

  return compareSum;
}

#endif
