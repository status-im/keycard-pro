#ifndef __ERROR
#define __ERROR

typedef enum {
  ERR_OK = 0,
  ERR_TXRX,
  ERR_CRYPTO,
  ERR_RETRY,
  ERR_DATA,
  ERR_CANCEL,
  ERR_HW,
  ERR_UNSUPPORTED,
  ERR_NEED_MORE_DATA,
  ERR_FULL,
  ERR_VERSION
} app_err_t;

#endif
