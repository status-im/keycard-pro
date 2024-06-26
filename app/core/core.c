#include "app_tasks.h"
#include "core.h"
#include "crypto/address.h"
#include "crypto/ripemd160.h"
#include "crypto/util.h"
#include "crypto/secp256k1.h"
#include "crypto/segwit_addr.h"
#include "ethereum/eth_db.h"
#include "mem.h"
#include "keycard/secure_channel.h"
#include "keycard/keycard_cmdset.h"
#include "keycard/keycard.h"
#include "storage/keys.h"
#include "ui/ui_internal.h"
#include "ur/eip4527_encode.h"
#include "util/tlv.h"

#define ETH_MSG_MAGIC_LEN 26
#define ETH_EIP712_MAGIC_LEN 2

#define BTC_SEGWIT_VER 0

#define CRYPTO_MULTIACCOUNT_SN_LEN 40

#define USB_MORE_DATA_TIMEOUT 100

typedef void (*core_addr_encoder_t)(const uint8_t* key, char* addr);

const char *const BTC_BECH32_HRP = "bc";

const uint8_t *const ETH_MSG_MAGIC = (uint8_t *) "\031Ethereum Signed Message:\n";
const uint8_t ETH_EIP712_MAGIC[] = { 0x19, 0x01 };

const uint32_t ETH_DEFAULT_BIP44[] = { 0x8000002c, 0x8000003c, 0x80000000 };
const uint32_t ETH_DEFAULT_BIP44_LEN = 3;

const uint32_t ETH_LEDGER_LIVE_PATH[] = { 0x8000002c, 0x8000003c };
const uint32_t ETH_LEDGER_LIVE_PATH_LEN = 2;

const uint32_t BTC_LEGACY_PATH[] = { 0x8000002c, 0x80000000, 0x80000000 };
const uint32_t BTC_LEGACY_PATH_LEN = 3;

const uint32_t BTC_SEGWIT_PATH[] = { 0x80000031, 0x80000000, 0x80000000 };
const uint32_t BTC_SEGWIT_PATH_LEN = 3;

const uint32_t BTC_NATIVE_SEGWIT_PATH[] = { 0x80000054, 0x80000000, 0x80000000 };
const uint32_t BTC_NATIVE_SEGWIT_PATH_LEN = 3;

const uint32_t BTC_TAPROOT_PATH[] = { 0x80000056, 0x80000000, 0x80000000 };
const uint32_t BTC_TAPROOT_PATH_LEN = 3;

const uint32_t TRX_PATH[] = { 0x8000002c, 0x800000c3, 0x80000000 };
const uint32_t TRX_PATH_LEN = 3;

const uint32_t LTC_PATH[] = { 0x80000031, 0x80000002, 0x80000000 };
const uint32_t LTC_PATH_LEN = 3;

const uint32_t BCH_PATH[] = { 0x8000002c, 0x80000091, 0x80000000 };
const uint32_t BCH_PATH_LEN = 3;

const uint32_t DASH_PATH[] = { 0x8000002c, 0x80000005, 0x80000000 };
const uint32_t DASH_PATH_LEN = 3;

const uint8_t *const EIP4527_NAME = (uint8_t*) "Keycard Pro";
const uint32_t EIP4527_NAME_LEN = 11;

const uint8_t *const EIP4527_SOURCE = (uint8_t*) "account.standard";
const uint32_t EIP4527_SOURCE_LEN = 16;

core_ctx_t g_core;

static app_err_t core_export_key(keycard_t* kc, uint8_t* path, uint16_t len, uint8_t* out_pub, uint8_t* out_chain) {
  uint8_t export_type;

  if (out_chain) {
    export_type = 2;
  } else {
    export_type = 1;
  }

  if ((keycard_cmd_export_key(kc, export_type, path, len) != ERR_OK) || (APDU_SW(&kc->apdu) != 0x9000)) {
    return ERR_CRYPTO;
  }

  uint8_t* data = APDU_RESP(&kc->apdu);

  uint16_t tag;
  uint16_t off = tlv_read_tag(data, &tag);
  if (tag != 0xa1) {
    return ERR_DATA;
  }

  off += tlv_read_length(&data[off], &len);
  len = tlv_read_fixed_primitive(0x80, PUBKEY_LEN, &data[off], out_pub);
  if (len == TLV_INVALID) {
    return ERR_DATA;
  }
  off += len;

  if (out_chain) {
    if (tlv_read_fixed_primitive(0x82, CHAINCODE_LEN, &data[off], out_chain) == TLV_INVALID) {
      return ERR_DATA;
    }
  }

  return ERR_OK;
}

static app_err_t core_get_fingerprint(uint8_t* path, size_t len, uint32_t* fingerprint) {
  if (len == 0 && g_core.master_fingerprint != 0) {
    *fingerprint = g_core.master_fingerprint;
    return ERR_OK;
  }

  app_err_t err = core_export_key(&g_core.keycard, path, len, g_core.data.key.pub, NULL);

  if (err != ERR_OK) {
    return err;
  }

  g_core.data.key.pub[0] = 0x02 | (g_core.data.key.pub[PUBKEY_LEN - 1] & 1);

  sha256_Raw(g_core.data.key.pub, PUBKEY_COMPRESSED_LEN, g_core.data.key.chain);
  ripemd160(g_core.data.key.chain, SHA256_DIGEST_LENGTH, g_core.data.key.pub);

  *fingerprint = (g_core.data.key.pub[0] << 24) | (g_core.data.key.pub[1] << 16) | (g_core.data.key.pub[2] << 8) | g_core.data.key.pub[3];

  if (len == 0)  {
    g_core.master_fingerprint = *fingerprint;
  }

  return ERR_OK;
}

static app_err_t core_export_public(uint8_t* pub, uint8_t* chain, uint32_t* fingerprint, uint32_t* parent_fingerprint) {
  SC_BUF(path, BIP44_MAX_PATH_LEN);
  app_err_t err;

  if (fingerprint) {
    err = core_get_fingerprint(path, 0, fingerprint);
    if (err != ERR_OK) {
      return err;
    }
  }

  if (parent_fingerprint) {
    memcpy(path, g_core.bip44_path, (g_core.bip44_path_len - 4));
    err = core_get_fingerprint(path, (g_core.bip44_path_len - 4), parent_fingerprint);
    if (err != ERR_OK) {
      return err;
    }
  }

  memcpy(path, g_core.bip44_path, g_core.bip44_path_len);
  err = core_export_key(&g_core.keycard, path, g_core.bip44_path_len, pub, chain);

  if (err != ERR_OK) {
    return err;
  }

  pub[0] = 0x02 | (pub[PUBKEY_LEN - 1] & 1);

  return ERR_OK;
}

static inline app_err_t core_init_sign(uint32_t* fingerprint) {
  keccak_256_Init(&g_core.hash_ctx);

  if (core_export_public(g_core.data.key.pub, g_core.data.key.chain, fingerprint, NULL) != ERR_OK) {
    return ERR_HW;
  }

  ethereum_address(g_core.data.key.pub, g_core.address);

  return ERR_OK;
}

static inline uint32_t core_get_tx_v_base() {
  uint32_t v_base;
  if (g_core.data.tx.ctx.txType == EIP1559 || g_core.data.tx.ctx.txType == EIP2930) {
    v_base = 0;
  } else {
    if (g_core.data.tx.content.v == V_NONE) {
      v_base = 27;
    } else {
      v_base = (g_core.data.tx.content.v * 2) + 35;
    }
  }

  return v_base;
}

static inline void core_set_is_message() {
  g_core.data.tx.ctx.txType = LEGACY;
  g_core.data.tx.content.v = V_NONE;
}

static app_err_t core_sign(keycard_t* kc, uint8_t* out) {
  uint8_t digest[SHA3_256_DIGEST_LENGTH];
  keccak_Final(&g_core.hash_ctx, digest);

  if ((keycard_cmd_sign(kc, g_core.bip44_path, g_core.bip44_path_len, digest) != ERR_OK) || (APDU_SW(&kc->apdu) != 0x9000)) {
    return ERR_CRYPTO;
  }

  uint8_t* data = APDU_RESP(&kc->apdu);

  if (keycard_read_signature(data, digest, out) != ERR_OK) {
    return ERR_DATA;
  }

  return ERR_OK;
}

static inline app_err_t core_wait_tx_confirmation() {
  return ui_display_tx(g_core.address, &g_core.data.tx.content) == CORE_EVT_UI_OK ? ERR_OK : ERR_CANCEL;
}

static inline app_err_t core_wait_msg_confirmation(const uint8_t* msg, size_t msg_len) {
  return ui_display_msg(g_core.address, msg, msg_len) == CORE_EVT_UI_OK ? ERR_OK : ERR_CANCEL;
}

static app_err_t core_process_tx(const uint8_t* data, uint32_t len, uint8_t first_segment) {
  if (first_segment) {
    // EIP 2718: TransactionType might be present before the TransactionPayload.
    uint8_t txType = data[0];

    initTx(&g_core.data.tx.ctx, &g_core.hash_ctx, &g_core.data.tx.content);

    if (txType >= MIN_TX_TYPE && txType <= MAX_TX_TYPE) {
      // Enumerate through all supported txTypes here...
      if (txType == EIP2930 || txType == EIP1559) {
        keccak_Update(&g_core.hash_ctx, data, 1);
        g_core.data.tx.ctx.txType = txType;
        data++;
        len--;
      } else {
        return ERR_UNSUPPORTED;
      }
    } else {
      g_core.data.tx.ctx.txType = LEGACY;
    }
  }

  if (g_core.data.tx.ctx.currentField == RLP_NONE) {
    return ERR_DATA;
  }

  parserStatus_e res = processTx(&g_core.data.tx.ctx, data, len);
  switch (res) {
    case USTREAM_FINISHED:
      return core_wait_tx_confirmation();
    case USTREAM_PROCESSING:
      return ERR_NEED_MORE_DATA;
    case USTREAM_FAULT:
    default:
      return ERR_DATA;
  }
}

static app_err_t core_process_msg(const uint8_t* data, uint32_t len, uint8_t first_segment) {
  if (first_segment) {
    core_set_is_message();
    g_core.data.msg.received = 0;
    keccak_Update(&g_core.hash_ctx, ETH_MSG_MAGIC, ETH_MSG_MAGIC_LEN);
    uint8_t tmp[11];
    uint8_t* ascii_len = u32toa(g_core.data.msg.len, tmp, 11);
    keccak_Update(&g_core.hash_ctx, ascii_len, 10 - (size_t)(ascii_len - tmp));
  }

  if ((g_core.data.msg.received + len) > g_core.data.msg.len) {
    return ERR_DATA;
  }

  keccak_Update(&g_core.hash_ctx, data, len);
  g_core.data.msg.received += len;

  if (g_core.data.msg.received == g_core.data.msg.len) {
    return core_wait_msg_confirmation(g_core.data.msg.content, g_core.data.msg.len);
  } else {
    return ERR_NEED_MORE_DATA;
  }
}

static app_err_t core_process_eip712(const uint8_t* data, uint32_t len) {
  core_set_is_message();

  uint8_t* heap = (uint8_t*) &data[len];
  size_t heap_size = MEM_HEAP_SIZE - ((size_t) (heap - g_mem_heap));
  app_err_t err;

  keccak_Update(&g_core.hash_ctx, ETH_EIP712_MAGIC, ETH_EIP712_MAGIC_LEN);
  memset(&g_core.data.eip712, 0, sizeof(eip712_ctx_t));
  err = eip712_hash(&g_core.data.eip712, &g_core.hash_ctx, heap, heap_size, (const char*) data, len);

  if (err != ERR_OK) {
    return err;
  }

  return ui_display_eip712(g_core.address, &g_core.data.eip712) == CORE_EVT_UI_OK ? ERR_OK : ERR_CANCEL;
}

static app_err_t core_usb_get_app_config(apdu_t* cmd) {
  uint8_t* data = APDU_RESP(cmd);
  data[0] = FW_VERSION[0];
  data[1] = FW_VERSION[1];
  data[2] = FW_VERSION[2];

  uint32_t db_version = 0;
  eth_db_lookup_version(&db_version);

  data[3] = db_version >> 24;
  data[4] = (db_version >> 16) & 0xff;
  data[5] = (db_version >> 8) & 0xff;
  data[6] = db_version & 0xff;

  hal_device_uid(&data[7]);

  uint8_t key[32];
  key_read_private(DEV_AUTH_PRIV_KEY, key);
  ecdsa_get_public_key33(&secp256k1, key, &data[7 + HAL_DEVICE_UID_LEN]);
  memset(key, 0, 32);

  data[7 + HAL_DEVICE_UID_LEN + 33] = 0x90;
  data[8 + HAL_DEVICE_UID_LEN + 33] = 0x00;
  cmd->lr = 9 + HAL_DEVICE_UID_LEN + 33;

  return ERR_OK;
}

static app_err_t core_usb_get_address(keycard_t* kc, apdu_t* cmd) {
  uint8_t* data = APDU_DATA(cmd);
  uint16_t len = data[0] * 4;
  if (len > BIP44_MAX_PATH_LEN) {
    core_usb_err_sw(cmd, 0x6a, 0x80);
    return ERR_DATA;
  }

  uint8_t extended = APDU_P2(cmd) == 1;

  SC_BUF(path, BIP44_MAX_PATH_LEN);
  memcpy(path, &data[1], len);
  uint8_t* out = APDU_RESP(cmd);

  app_err_t err = core_export_key(kc, path, len, &out[1], (extended ? &out[107] : NULL));

  switch (err) {
  case ERR_OK:
    break;
  case ERR_CRYPTO:
    core_usb_err_sw(cmd, 0x69, 0x82);
    return ERR_DATA;
  default:
    core_usb_err_sw(cmd, 0x6f, 0x00);
    return ERR_DATA;
  }

  out[0] = 65;
  out[66] = 40;

  ethereum_address(&out[1], path);
  ethereum_address_checksum(path, (char *)&out[67]);

  if (APDU_P1(cmd) == 1) {
    if (ui_confirm_eth_address((char *)&out[67]) != CORE_EVT_UI_OK) {
      core_usb_err_sw(cmd, 0x69, 0x82);
      return ERR_CANCEL;
    }
  }

  if (extended) {
    len = 139;
  } else {
    len = 107;
  }

  out[len++] = 0x90;
  out[len++] = 0x00;

  cmd->lr = len;

  return ERR_OK;
}

static app_err_t core_usb_init_sign(uint8_t* data) {
  g_core.bip44_path_len = data[0] * 4;

  if (g_core.bip44_path_len > BIP44_MAX_PATH_LEN) {
    g_core.bip44_path_len = 0;
    return ERR_DATA;
  }

  memcpy(g_core.bip44_path, &data[1], g_core.bip44_path_len);

  return core_init_sign(NULL);
}

static void core_usb_sign(keycard_t* kc, apdu_t* cmd) {
  uint8_t* out = APDU_RESP(cmd);

  switch (core_sign(kc, &out[1])) {
  case ERR_OK:
    out[0] = core_get_tx_v_base() + out[65];
    out[65] = 0x90;
    out[66] = 0x00;
    cmd->lr = 67;
    break;
  case ERR_CRYPTO:
    core_usb_err_sw(cmd, 0x69, 0x82);
    break;
  default:
    core_usb_err_sw(cmd, 0x6f, 0x00);
    break;
  }
}

static app_err_t core_usb_sign_tx(keycard_t* kc, apdu_t* cmd) {
  cmd->has_lc = 1;
  uint8_t* data = APDU_DATA(cmd);
  uint32_t len = APDU_LC(cmd);
  uint8_t first = APDU_P1(cmd) == 0;

  if (first) {
    if (core_usb_init_sign(data) != ERR_OK) {
      core_usb_err_sw(cmd, 0x6a, 0x80);
      return ERR_DATA;
    }

    data = &data[1+g_core.bip44_path_len];
    len -= g_core.bip44_path_len + 1;

    if (len < 1) {
      core_usb_err_sw(cmd, 0x6a, 0x80);
      return ERR_DATA;
    }

    g_core.data.tx.content.data = g_mem_heap;
  }

  app_err_t err = core_process_tx(data, len, first);

  switch(err) {
  case ERR_OK:
    core_usb_sign(kc, cmd);
    break;
  case ERR_NEED_MORE_DATA:
    core_usb_err_sw(cmd, 0x90, 0x00);
    break;
  case ERR_DATA:
    core_usb_err_sw(cmd, 0x6a, 0x80);
    break;
  case ERR_CANCEL:
    core_usb_err_sw(cmd, 0x69, 0x82);
    break;
  case ERR_UNSUPPORTED:
    core_usb_err_sw(cmd, 0x65, 0x01);
    break;
  default:
    core_usb_err_sw(cmd, 0x6f, 0x00);
    break;
  }

  return err;
}

static void core_usb_message_reassemble(keycard_t* kc, apdu_t* cmd, uint8_t** segment, uint32_t* len, uint8_t* first_segment) {
  cmd->has_lc = 1;
  uint8_t* data = APDU_DATA(cmd);
  *len = APDU_LC(cmd);
  *first_segment = APDU_P1(cmd) == 0;
  if (*first_segment) {
    if (core_usb_init_sign(data) != ERR_OK) {
      core_usb_err_sw(cmd, 0x6a, 0x80);
      return;
    }

    g_core.data.msg.len = (data[1+g_core.bip44_path_len] << 24) | (data[2+g_core.bip44_path_len] << 16) | (data[3+g_core.bip44_path_len] << 8) | data[4+g_core.bip44_path_len];

    if (g_core.data.msg.len > MEM_HEAP_SIZE) {
      core_usb_err_sw(cmd, 0x6a, 0x80);
      return;
    }

    g_core.data.msg.received = 0;
    *len -= g_core.bip44_path_len + 5;
    data = &data[g_core.bip44_path_len + 5];
  }

  if ((g_core.data.msg.received + *len) > MEM_HEAP_SIZE) {
    core_usb_err_sw(cmd, 0x6a, 0x80);
    return;
  }

  g_core.data.msg.content = g_mem_heap;
  *segment = &g_core.data.msg.content[g_core.data.msg.received];
  memcpy(*segment, data, *len);
}

static app_err_t core_usb_sign_message(keycard_t* kc, apdu_t* cmd) {
  uint8_t* segment;
  uint32_t len;
  uint8_t first_segment;
  core_usb_message_reassemble(kc, cmd, &segment, &len, &first_segment);

  app_err_t err = core_process_msg(segment, len, first_segment);

  switch(err) {
    case ERR_OK:
      core_usb_sign(kc, cmd);
      break;
    case ERR_NEED_MORE_DATA:
      core_usb_err_sw(cmd, 0x90, 0x00);
      break;
    case ERR_DATA:
      core_usb_err_sw(cmd, 0x6a, 0x80);
      break;
    case ERR_CANCEL:
      core_usb_err_sw(cmd, 0x69, 0x82);
      break;
    default:
      core_usb_err_sw(cmd, 0x6f, 0x00);
      break;
  }

  return err;
}

static app_err_t core_usb_sign_eip712(keycard_t* kc, apdu_t* cmd) {
  if (APDU_P2(cmd) != 1) {
    core_usb_err_sw(cmd, 0x69, 0x82);
    return ERR_DATA;
  }

  uint8_t* segment;
  uint32_t len;
  uint8_t first_segment;
  core_usb_message_reassemble(kc, cmd, &segment, &len, &first_segment);

  if ((g_core.data.msg.received + len) > g_core.data.msg.len) {
    core_usb_err_sw(cmd, 0x6a, 0x80);
    return ERR_DATA;
  }

  g_core.data.msg.received += len;

  core_set_is_message();

  if (g_core.data.msg.received == g_core.data.msg.len) {
    if (core_process_eip712(g_core.data.msg.content, g_core.data.msg.len) == ERR_OK) {
      core_usb_sign(kc, cmd);
      return ERR_OK;
    } else {
      core_usb_err_sw(cmd, 0x69, 0x82);
      return ERR_CANCEL;
    }
  } else {
    core_usb_err_sw(cmd, 0x90, 0x00);
    return ERR_NEED_MORE_DATA;
  }

}

static app_err_t core_usb_command(keycard_t* kc, command_t* cmd) {
  apdu_t* apdu = &cmd->apdu;

  app_err_t err;

  if (APDU_CLA(apdu) == 0xe0) {
    switch(APDU_INS(apdu)) {
      case INS_GET_ETH_ADDR:
        err = core_usb_get_address(kc, apdu);
        break;
      case INS_SIGN_ETH_TX:
        err = core_usb_sign_tx(kc, apdu);
        break;
      case INS_SIGN_ETH_MSG:
        err = core_usb_sign_message(kc, apdu);
        break;
      case INS_GET_APP_CONF:
        err = core_usb_get_app_config(apdu);
        break;
      case INS_SIGN_EIP_712:
        err = core_usb_sign_eip712(kc, apdu);
        break;
      case INS_FW_UPGRADE:
        err = updater_usb_fw_upgrade(cmd, apdu);
        break;
      case INS_ERC20_UPGRADE:
        err = updater_usb_db_upgrade(apdu);
        break;
      default:
        err = ERR_CANCEL;
        core_usb_err_sw(apdu, 0x6d, 0x00);
        break;
    }
  } else {
    err = ERR_CANCEL;
    core_usb_err_sw(apdu, 0x6e, 0x00);
  }

  command_init_send(cmd);
  return err;
}

void core_usb_cancel() {
  core_usb_err_sw(&g_core.usb_command.apdu, 0x69, 0x82);
  command_init_send(&g_core.usb_command);
}

void core_usb_run() {
  while(1) {
    if (core_usb_command(&g_core.keycard, &g_core.usb_command) == ERR_NEED_MORE_DATA) {
      if (core_wait_event(USB_MORE_DATA_TIMEOUT, 1) != CORE_EVT_USB_CMD) {
        break;
      }
    } else {
      break;
    }
  }
}

static app_err_t core_eip4527_init_sign(struct eth_sign_request *qr_request) {
  g_core.bip44_path_len = qr_request->eth_sign_request_derivation_path.crypto_keypath_components_path_component_m_count * 4;

  if (g_core.bip44_path_len > BIP44_MAX_PATH_LEN) {
    g_core.bip44_path_len = 0;
    return ERR_DATA;
  }

  for (int i = 0; i < qr_request->eth_sign_request_derivation_path.crypto_keypath_components_path_component_m_count; i++) {
    uint32_t idx = qr_request->eth_sign_request_derivation_path.crypto_keypath_components_path_component_m[i].path_component_child_index_m;
    if (qr_request->eth_sign_request_derivation_path.crypto_keypath_components_path_component_m[i].path_component_is_hardened_m) {
      idx |= 0x80000000;
    }

    g_core.bip44_path[(i * 4)] = idx >> 24;
    g_core.bip44_path[(i * 4) + 1] = (idx >> 16) & 0xff;
    g_core.bip44_path[(i * 4) + 2] = (idx >> 8) & 0xff;
    g_core.bip44_path[(i * 4) + 3] = idx & 0xff;
  }

  uint32_t fingerprint;
  app_err_t err = core_init_sign(&fingerprint);

  if (err != ERR_OK) {
    return err;
  }

  if (!(qr_request->eth_sign_request_derivation_path.crypto_keypath_source_fingerprint_present &&
      (qr_request->eth_sign_request_derivation_path.crypto_keypath_source_fingerprint.crypto_keypath_source_fingerprint == fingerprint))) {
    return ERR_MISMATCH;
  }

  ethereum_address(g_core.data.key.pub, g_core.address);

  return ERR_OK;
}

void core_qr_run() {
  struct eth_sign_request qr_request;

  if (ui_qrscan(ETH_SIGN_REQUEST, &qr_request) != CORE_EVT_UI_OK) {
    return;
  }

  if (core_eip4527_init_sign(&qr_request) != ERR_OK) {
    ui_info(LSTR(INFO_WRONG_CARD), 1);
    return;
  }

  app_err_t err;

  switch(qr_request.eth_sign_request_data_type.sign_data_type_choice) {
    case sign_data_type_eth_transaction_data_m_c:
    case sign_data_type_eth_typed_transaction_m_c:
      g_core.data.tx.content.data = NULL;
      g_core.data.tx.content.chainID = qr_request.eth_sign_request_chain_id_present ? (uint32_t) qr_request.eth_sign_request_chain_id.eth_sign_request_chain_id : 1;
      err = core_process_tx(qr_request.eth_sign_request_sign_data.value, qr_request.eth_sign_request_sign_data.len, 1);
      break;
    case sign_data_type_eth_raw_bytes_m_c:
      g_core.data.msg.content = (uint8_t*) qr_request.eth_sign_request_sign_data.value;
      g_core.data.msg.len = qr_request.eth_sign_request_sign_data.len;
      err = core_process_msg(qr_request.eth_sign_request_sign_data.value, qr_request.eth_sign_request_sign_data.len, 1);
      break;
    case sign_data_type_eth_typed_data_m_c:
      err = core_process_eip712(qr_request.eth_sign_request_sign_data.value, qr_request.eth_sign_request_sign_data.len);
      break;
    default:
      err = ERR_UNSUPPORTED;
      break;
  }

  switch(err) {
  case ERR_OK:
    break;
  case ERR_CANCEL:
    return;
  default:
    ui_info(LSTR(INFO_MALFORMED_DATA), 1);
    return;
  }

  uint32_t v = core_get_tx_v_base();

  if (core_sign(&g_core.keycard, g_core.data.sig.plain_sig) != ERR_OK) {
    ui_card_transport_error();
    return;
  }

  struct eth_signature sig = {0};
  sig.eth_signature_request_id_present = qr_request.eth_sign_request_request_id_present;
  if (sig.eth_signature_request_id_present) {
    sig.eth_signature_request_id.eth_signature_request_id.value = qr_request.eth_sign_request_request_id.eth_sign_request_request_id.value;
    sig.eth_signature_request_id.eth_signature_request_id.len = qr_request.eth_sign_request_request_id.eth_sign_request_request_id.len;
  }
  sig.eth_signature_signature.value = g_core.data.sig.plain_sig;
  sig.eth_signature_signature.len = SIGNATURE_LEN;

  v += g_core.data.sig.plain_sig[SIGNATURE_LEN];

  if (v <= 0xff) {
    g_core.data.sig.plain_sig[SIGNATURE_LEN] = v;
    sig.eth_signature_signature.len += 1;
  } else if (v <= 0xffff) {
    g_core.data.sig.plain_sig[SIGNATURE_LEN] = (v >> 8) & 0xff;
    g_core.data.sig.plain_sig[SIGNATURE_LEN + 1] = v & 0xff;
    sig.eth_signature_signature.len += 2;
  } else if (v <= 0xffffff) {
    g_core.data.sig.plain_sig[SIGNATURE_LEN] = (v >> 16) & 0xff;
    g_core.data.sig.plain_sig[SIGNATURE_LEN + 1] = (v >> 8) & 0xff;
    g_core.data.sig.plain_sig[SIGNATURE_LEN + 2] = v & 0xff;
    sig.eth_signature_signature.len += 3;
  } else if (v <= 0xffffffff) {
    g_core.data.sig.plain_sig[SIGNATURE_LEN] = v >> 24;
    g_core.data.sig.plain_sig[SIGNATURE_LEN + 1] = (v >> 16) & 0xff;
    g_core.data.sig.plain_sig[SIGNATURE_LEN + 2] = (v >> 8) & 0xff;
    g_core.data.sig.plain_sig[SIGNATURE_LEN + 3] = v & 0xff;
    sig.eth_signature_signature.len += 4;
  }

  cbor_encode_eth_signature(g_core.data.sig.cbor_sig, CBOR_SIG_MAX_LEN, &sig, &g_core.data.sig.cbor_len);
  ui_display_ur_qr(LSTR(QR_SIGNATURE_TITLE), g_core.data.sig.cbor_sig, g_core.data.sig.cbor_len, ETH_SIGNATURE);
}

static app_err_t encode_hd_key(struct hd_key* key, uint8_t* pub, uint8_t* chain, const uint32_t bip32_path[], size_t bip32_len, bool add_source) {
  key->hd_key_is_private = 0;
  key->hd_key_key_data.len = PUBKEY_COMPRESSED_LEN;
  key->hd_key_key_data.value = pub;
  key->hd_key_chain_code.len = CHAINCODE_LEN;
  key->hd_key_chain_code.value = chain;
  key->hd_key_use_info_present = 0;
  key->hd_key_origin.crypto_keypath_depth_present = 1;
  key->hd_key_origin.crypto_keypath_depth.crypto_keypath_depth = bip32_len;
  key->hd_key_origin.crypto_keypath_source_fingerprint_present = 1;
  key->hd_key_origin.crypto_keypath_components_path_component_m_count = bip32_len;
  key->hd_key_name.len = EIP4527_NAME_LEN;
  key->hd_key_name.value = EIP4527_NAME;

  if (add_source) {
    key->hd_key_source_present = 1;
    key->hd_key_source.hd_key_source.len = EIP4527_SOURCE_LEN;
    key->hd_key_source.hd_key_source.value = EIP4527_SOURCE;
  } else {
    key->hd_key_source_present = 0;
  }

  for (int i = 0; i < bip32_len; i++) {
    uint32_t c = bip32_path[i];
    g_core.bip44_path[(i * 4)] = c >> 24;
    g_core.bip44_path[(i * 4) + 1] = (c >> 16) & 0xff;
    g_core.bip44_path[(i * 4) + 2] = (c >> 8) & 0xff;
    g_core.bip44_path[(i * 4) + 3] = (c & 0xff);
    key->hd_key_origin.crypto_keypath_components_path_component_m[i].path_component_child_index_m = c & 0x7fffffff;
    key->hd_key_origin.crypto_keypath_components_path_component_m[i].path_component_is_hardened_m = c > 0x7fffffff;
  }

  g_core.bip44_path_len = bip32_len * 4;

  if (core_export_public(pub, chain, &key->hd_key_origin.crypto_keypath_source_fingerprint.crypto_keypath_source_fingerprint, &key->hd_key_parent_fingerprint) != ERR_OK) {
    return ERR_HW;
  }

  return ERR_OK;
}

void core_display_public_eip4527() {
  struct hd_key key;

  if (encode_hd_key(&key, g_core.data.key.pub, g_core.data.key.chain, ETH_DEFAULT_BIP44, ETH_DEFAULT_BIP44_LEN, true) != ERR_OK) {
    ui_card_transport_error();
    return;
  }

  cbor_encode_hd_key(g_core.data.key.cbor_key, CBOR_KEY_MAX_LEN, &key, &g_core.data.key.cbor_len);
  ui_display_ur_qr(LSTR(QR_CONNECT_EIP4527_TITLE), g_core.data.key.cbor_key, g_core.data.key.cbor_len, CRYPTO_HDKEY);
}

// this macro can only be used in core_display_public_multicoin()
#define CORE_MULTICOIN_EXPORT(__NUM__, __PATH__, __PATH_LEN__, __SOURCE___) \
  if (encode_hd_key(&accounts.crypto_multi_accounts_keys_hd_key_m[__NUM__], &g_mem_heap[keys_off], &g_mem_heap[keys_off + PUBKEY_LEN], __PATH__, __PATH_LEN__, __SOURCE___) != ERR_OK) { \
    ui_card_transport_error(); \
    return; \
  } \
  keys_off += PUBKEY_LEN + CHAINCODE_LEN

void core_display_public_multicoin() {
  struct crypto_multi_accounts accounts;

  accounts.crypto_multi_accounts_device.crypto_multi_accounts_device.len = EIP4527_NAME_LEN;
  accounts.crypto_multi_accounts_device.crypto_multi_accounts_device.value = EIP4527_NAME;
  accounts.crypto_multi_accounts_device_present = 1;

  uint8_t uid[CRYPTO_MULTIACCOUNT_SN_LEN/2];
  memset(uid, 0, CRYPTO_MULTIACCOUNT_SN_LEN/20);
  hal_device_uid(uid);
  uint8_t sn[CRYPTO_MULTIACCOUNT_SN_LEN];
  base16_encode(uid, (char*) sn, (CRYPTO_MULTIACCOUNT_SN_LEN/2));

  accounts.crypto_multi_accounts_device_id.crypto_multi_accounts_device_id.len = CRYPTO_MULTIACCOUNT_SN_LEN;
  accounts.crypto_multi_accounts_device_id.crypto_multi_accounts_device_id.value = sn;
  accounts.crypto_multi_accounts_device_id_present = 1;

  accounts.crypto_multi_accounts_version_present = 0;

  size_t keys_off = 0;

  CORE_MULTICOIN_EXPORT(0, BTC_LEGACY_PATH, BTC_LEGACY_PATH_LEN, false);
  CORE_MULTICOIN_EXPORT(1, BTC_SEGWIT_PATH, BTC_SEGWIT_PATH_LEN, false);
  CORE_MULTICOIN_EXPORT(2, BTC_NATIVE_SEGWIT_PATH, BTC_NATIVE_SEGWIT_PATH_LEN, false);
  CORE_MULTICOIN_EXPORT(3, ETH_DEFAULT_BIP44, ETH_DEFAULT_BIP44_LEN, true);

  accounts.crypto_multi_accounts_keys_hd_key_m_count = 4;
  accounts.crypto_multi_accounts_master_fingerprint = g_core.master_fingerprint;

  cbor_encode_crypto_multi_accounts(&g_mem_heap[keys_off], MEM_HEAP_SIZE, &accounts, &g_core.data.key.cbor_len);
  ui_display_ur_qr(LSTR(QR_CONNECT_MULTIACCOUNT_TITLE), &g_mem_heap[keys_off], g_core.data.key.cbor_len, CRYPTO_MULTI_ACCOUNTS);
}

static void core_addresses(const uint32_t* base_path, size_t base_len, core_addr_encoder_t encoder) {
  uint32_t index = 0;

  for (int i = 0; i < base_len; i++) {
    uint32_t c = base_path[i];
    g_core.bip44_path[(i * 4)] = c >> 24;
    g_core.bip44_path[(i * 4) + 1] = (c >> 16) & 0xff;
    g_core.bip44_path[(i * 4) + 2] = (c >> 8) & 0xff;
    g_core.bip44_path[(i * 4) + 3] = (c & 0xff);
  }

  g_core.bip44_path[(base_len * 4)] = index >> 24;
  g_core.bip44_path[(base_len * 4) + 1] = (index >> 16) & 0xff;
  g_core.bip44_path[(base_len * 4) + 2] = (index >> 8) & 0xff;
  g_core.bip44_path[(base_len * 4) + 3] = (index & 0xff);

  base_len++;

  g_core.bip44_path_len = (base_len + 1) * 4;

  do {
    g_core.bip44_path[(base_len * 4)] = index >> 24;
    g_core.bip44_path[(base_len * 4) + 1] = (index >> 16) & 0xff;
    g_core.bip44_path[(base_len * 4) + 2] = (index >> 8) & 0xff;
    g_core.bip44_path[(base_len * 4) + 3] = (index & 0xff);

    if (core_export_public(g_core.data.key.pub, NULL, NULL, NULL) != ERR_OK) {
      ui_info(LSTR(INFO_CARD_ERROR_MSG), 0);
    }

    encoder(g_core.data.key.pub, (char*) g_mem_heap);
    ui_display_address_qr((char*) g_mem_heap, &index);
  } while(index != UINT32_MAX);
}

static void core_eth_addr_encoder(const uint8_t* key, char* addr) {
  addr[0] = '0';
  addr[1] = 'x';
  ethereum_address(key, g_core.address);
  ethereum_address_checksum(g_core.address, &addr[2]);
}

static void core_btc_addr_encoder(const uint8_t* key, char* addr) {
  sha256_Raw(key, PUBKEY_COMPRESSED_LEN, g_core.data.key.chain);
  ripemd160(g_core.data.key.chain, SHA256_DIGEST_LENGTH, g_core.address);

  segwit_addr_encode(addr, BTC_BECH32_HRP, BTC_SEGWIT_VER, g_core.address, RIPEMD160_DIGEST_LENGTH);
}

void core_addresses_ethereum() {
  core_addresses(ETH_DEFAULT_BIP44, ETH_DEFAULT_BIP44_LEN, core_eth_addr_encoder);
}

void core_addresses_bitcoin() {
  core_addresses(BTC_NATIVE_SEGWIT_PATH, BTC_NATIVE_SEGWIT_PATH_LEN, core_btc_addr_encoder);
}

core_evt_t core_wait_event(uint32_t timeout, uint8_t accept_usb) {
  uint32_t events;

  BaseType_t res = pdFAIL;
  res = xTaskNotifyWaitIndexed(CORE_EVENT_IDX, 0, UINT32_MAX, &events, timeout);

  if (res != pdPASS) {
    return CORE_EVT_NONE;
  }

  if (events & CORE_USB_EVT) {
    if (accept_usb) {
      return CORE_EVT_USB_CMD;
    } else {
      core_usb_cancel();
    }
  }

  if (events & CORE_UI_EVT) {
    return g_ui_cmd.result == ERR_OK ? CORE_EVT_UI_OK : CORE_EVT_UI_CANCELLED;
  }

  return CORE_EVT_NONE;
}
