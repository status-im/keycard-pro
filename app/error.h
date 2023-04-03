#ifndef __ERROR
#define __ERROR

typedef enum {
  ERR_OK = 0,
  ERR_TXRX,
  ERR_CRYPTO,
  ERR_RETRY,
  ERR_DATA,
  ERR_CANCEL
} app_err_t;

#endif
