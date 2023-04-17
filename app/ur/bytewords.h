#ifndef _BYTEWORDS_
#define _BYTEWORDS_

#include <stdint.h>

size_t bytewords_decode(const uint8_t* in, size_t in_len, uint8_t* out, size_t max_out);
size_t bytewords_encode(const uint8_t* in, size_t in_len, uint8_t* out, size_t max_out);

#endif
