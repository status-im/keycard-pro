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
#define SIGNATURE_LEN 65
#define MAX_MSG_SIZE 512

typedef enum {
  CORE_EVT_USB_CMD,
  CORE_EVT_UI_CANCELLED,
  CORE_EVT_UI_OK
} core_evt_t;

typedef struct {
  txContext_t ctx;
  txContent_t content;
} core_tx_t;

typedef struct {
  uint32_t received;
  uint32_t len;
  uint8_t content[MAX_MSG_SIZE];
} core_msg_t;

typedef union {
  core_tx_t tx;
  core_msg_t msg;
} core_data_t;

typedef struct {
  Keycard keycard;
  Command usb_command;
  struct eth_sign_request qr_request;

  uint8_t digest[SHA3_256_DIGEST_LENGTH];
  uint8_t bip44_path[BIP44_MAX_PATH_LEN];
  uint8_t bip44_path_len;
  uint8_t signature[SIGNATURE_LEN];
  SHA3_CTX hash_ctx;
  core_data_t data;
} core_ctx_t;

extern core_ctx_t g_core;

core_evt_t core_wait_event(uint8_t accept_usb);
void core_usb_run();
void core_action_run(i18n_str_id_t menu);

#endif
