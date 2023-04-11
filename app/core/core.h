#ifndef _CORE_H_
#define _CORE_H_

#include "FreeRTOS.h"
#include "task.h"

#include "app_tasks.h"
#include "crypto/sha3.h"
#include "ethereum/ethUstream.h"
#include "ethereum/ethUtils.h"
#include "keycard/keycard.h"
#include "ui/ui.h"
#include "ur/eip4527_types.h"

#define BIP44_MAX_PATH_LEN 40

typedef enum {
  CORE_EVT_USB_CMD,
  CORE_EVT_UI_CANCELLED,
  CORE_EVT_UI_OK
} core_evt_t;

typedef struct {
  Keycard keycard;
  Command usb_command;
  struct eth_sign_request qr_request;

  uint8_t digest[SHA3_256_DIGEST_LENGTH];
  uint8_t bip44_path[BIP44_MAX_PATH_LEN];
  uint8_t bip44_path_len;
  uint32_t remaining;
  SHA3_CTX hash_ctx;
  txContent_t tx_data;
  txContext_t tx_ctx;
} core_ctx_t;

extern core_ctx_t g_core;

core_evt_t core_wait_event(uint8_t accept_usb);
void core_usb_run();
void core_action_run(i18n_str_id_t menu);

#endif
