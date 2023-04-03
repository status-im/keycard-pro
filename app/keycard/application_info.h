#ifndef __APPLICATION_INFO_H
#define __APPLICATION_INFO_H

#include <stdint.h>
#include "error.h"

#define APP_INFO_INSTANCE_UID_LEN 16
#define APP_INFO_KEY_UID_LEN 32
#define APP_INFO_PUBKEY_LEN 65

typedef enum {
  NOT_INITIALIZED,
  INIT_NO_KEYS,
  INIT_WITH_KEYS
} ApplicationInfoStatus;

typedef struct {
  ApplicationInfoStatus status;
  uint8_t instance_uid[APP_INFO_INSTANCE_UID_LEN];
  uint8_t sc_key[APP_INFO_PUBKEY_LEN];
  uint16_t version;
  uint8_t free_pairing;
  uint8_t key_uid[APP_INFO_KEY_UID_LEN];
} ApplicationInfo;

typedef struct {
  uint8_t pin_retries;
  uint8_t puk_retries;
  uint8_t has_key;
} ApplicationStatus;

app_err_t ApplicationInfo_Parse(uint8_t* buf, ApplicationInfo* info);
app_err_t ApplicationStatus_Parse(uint8_t* buf, ApplicationStatus* status);
#endif
