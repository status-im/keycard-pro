#ifndef _LOG_H_
#define _LOG_H_

#include <stdint.h>
#include <string.h>

typedef enum {
  LOG_MSG = 0,
  LOG_IMG,
  LOG_TXN
} log_type_t;

#ifdef DEBUG
#define LOG(t, d, l) _log(t, d, l)
#define LOG_MSG(m) _log_msg(m)
#else
#define LOG(t, m, l)
#define LOG_MSG(m)
#endif

void _log(log_type_t type, const uint8_t* data, size_t len);

static inline void _log_msg(const char* msg) {
  _log(LOG_MSG, (const uint8_t*) msg, strlen(msg));
}

#endif
