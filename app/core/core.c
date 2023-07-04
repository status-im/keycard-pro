#include "app_tasks.h"
#include "core.h"
#include "crypto/address.h"
#include "crypto/ripemd160.h"
#include "crypto/util.h"
#include "mem.h"
#include "log/log.h"
#include "keycard/secure_channel.h"
#include "keycard/keycard_cmdset.h"
#include "keycard/keycard.h"
#include "ui/ui_internal.h"
#include "ur/eip4527_encode.h"
#include "util/tlv.h"

#define APP_MAJOR 1
#define APP_MINOR 9
#define APP_PATCH 17

#define ETH_MSG_MAGIC_LEN 26
#define ETH_EIP712_MAGIC_LEN 2

const uint8_t* ETH_MSG_MAGIC = (uint8_t *) "\031Ethereum Signed Message:\n";
const uint8_t ETH_EIP712_MAGIC[] = { 0x19, 0x01 };

const uint32_t ETH_DEFAULT_BIP44[] = { 0x8000002c, 0x8000003c, 0x80000000 };
const uint32_t ETH_DEFAULT_BIP44_LEN = 3;

const uint8_t* EIP4527_NAME = (uint8_t*) "Keycard Pro";
const uint32_t EIP4527_NAME_LEN = 11;

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

static inline app_err_t core_init_sign() {
  keccak_256_Init(&g_core.hash_ctx);
  return ERR_OK;
}

static inline uint8_t core_get_tx_v_base() {
  uint8_t v_base;
  if (g_core.data.tx.ctx.txType == EIP1559 || g_core.data.tx.ctx.txType == EIP2930) {
    v_base = 0;
  } else {
    if (g_core.data.tx.content.vLength == 0) {
      v_base = 27;
    } else {
      uint32_t v = u32_from_BE(g_core.data.tx.content.v, APP_MIN(4, g_core.data.tx.content.vLength));
      v_base = (v * 2) + 35;
    }
  }

  return v_base;
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
  uint32_t chain_id = eth_tx_chain_id(&g_core.data.tx.ctx);
  return ui_display_tx(&g_core.data.tx.content, chain_id) == CORE_EVT_UI_OK ? ERR_OK : ERR_CANCEL;
}

static inline app_err_t core_wait_msg_confirmation(const uint8_t* msg) {
  return ui_display_msg(msg, g_core.data.msg.len) == CORE_EVT_UI_OK ? ERR_OK : ERR_CANCEL;
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

  parserStatus_e res = processTx(&g_core.data.tx.ctx, data, len, 0);
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
    return core_wait_msg_confirmation(data);
  } else {
    return ERR_NEED_MORE_DATA;
  }
}

static void core_usb_err_sw(apdu_t* cmd, uint8_t sw1, uint8_t sw2) {
  uint8_t* data = APDU_RESP(cmd);
  data[0] = sw1;
  data[1] = sw2;
  cmd->lr = 2;
}

static void core_usb_get_app_config(apdu_t* cmd) {
  uint8_t* data = APDU_RESP(cmd);
  data[0] = 0x03;
  data[1] = APP_MAJOR;
  data[2] = APP_MINOR;
  data[3] = APP_PATCH;
  data[4] = 0x90;
  data[5] = 0x00;
  cmd->lr = 6;
}

static void core_usb_get_address(keycard_t* kc, apdu_t* cmd) {
  uint8_t* data = APDU_DATA(cmd);
  uint16_t len = data[0] * 4;
  if (len > BIP44_MAX_PATH_LEN) {
    core_usb_err_sw(cmd, 0x6a, 0x80);
    return;
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
    return;
  default:
    core_usb_err_sw(cmd, 0x6f, 0x00);
    return;
  }

  out[0] = 65;
  out[66] = 40;

  ethereum_address(&out[1], path);
  ethereum_address_checksum(path, (char *)&out[67]);

  if (APDU_P1(cmd) == 1) {
    if (ui_confirm_eth_address((char *)&out[67]) != CORE_EVT_UI_OK) {
      core_usb_err_sw(cmd, 0x69, 0x82);
      return;
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
}

static app_err_t core_usb_init_sign(uint8_t* data) {
  g_core.bip44_path_len = data[0] * 4;

  if (g_core.bip44_path_len > BIP44_MAX_PATH_LEN) {
    g_core.bip44_path_len = 0;
    return ERR_DATA;
  }

  memcpy(g_core.bip44_path, &data[1], g_core.bip44_path_len);

  return core_init_sign();
}

static void core_usb_sign(keycard_t* kc, apdu_t* cmd, uint8_t v_base) {
  uint8_t* out = APDU_RESP(cmd);

  switch (core_sign(kc, &out[1])) {
  case ERR_OK:
    out[0] = v_base + out[65];
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

static void core_usb_sign_tx(keycard_t* kc, apdu_t* cmd) {
  cmd->has_lc = 1;
  uint8_t* data = APDU_DATA(cmd);
  uint32_t len = APDU_LC(cmd);
  uint8_t first = APDU_P1(cmd) == 0;

  if (first) {
    if (core_usb_init_sign(data) != ERR_OK) {
      core_usb_err_sw(cmd, 0x6a, 0x80);
      return;
    }

    data = &data[1+g_core.bip44_path_len];
    len -= g_core.bip44_path_len + 1;

    if (len < 1) {
      core_usb_err_sw(cmd, 0x6a, 0x80);
      return;
    }
  }

  switch(core_process_tx(data, len, first)) {
  case ERR_OK:
    core_usb_sign(kc, cmd, core_get_tx_v_base());
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
}

static void core_usb_sign_message(keycard_t* kc, apdu_t* cmd) {
  cmd->has_lc = 1;
  uint8_t* data = APDU_DATA(cmd);
  uint32_t len = APDU_LC(cmd);
  uint8_t first_segment = APDU_P1(cmd) == 0;
  g_core.data.msg.content = g_mem_heap;

  if (first_segment) {
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
    keccak_Update(&g_core.hash_ctx, ETH_MSG_MAGIC, ETH_MSG_MAGIC_LEN);
    len -= g_core.bip44_path_len + 5;
    data = &data[g_core.bip44_path_len + 5];
  }

  if ((g_core.data.msg.received + len) > MEM_HEAP_SIZE) {
    core_usb_err_sw(cmd, 0x6a, 0x80);
    return;
  }

  uint8_t* buf = &g_core.data.msg.content[g_core.data.msg.received];
  memcpy(buf, data, len);

  switch(core_process_msg(buf, len, first_segment)) {
    case ERR_OK:
      core_usb_sign(kc, cmd, 27);
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
}

static void core_usb_sign_eip712(keycard_t* kc, apdu_t* cmd) {
  uint8_t* data = APDU_DATA(cmd);

  if (core_usb_init_sign(data) != ERR_OK) {
    core_usb_err_sw(cmd, 0x6a, 0x80);
    return;
  }

  keccak_Update(&g_core.hash_ctx, ETH_EIP712_MAGIC, ETH_EIP712_MAGIC_LEN);
  keccak_Update(&g_core.hash_ctx, &data[1+g_core.bip44_path_len], (SHA3_256_DIGEST_LENGTH * 2));

  core_usb_sign(kc, cmd, 27);
}

static void core_usb_command(keycard_t* kc, command_t* cmd) {
  apdu_t* apdu = &cmd->apdu;

  if (APDU_CLA(apdu) == 0xe0) {
    switch(APDU_INS(apdu)) {
      case INS_GET_ETH_ADDR:
        core_usb_get_address(kc, apdu);
        break;
      case INS_SIGN_ETH_TX:
        core_usb_sign_tx(kc, apdu);
        break;
      case INS_SIGN_ETH_MSG:
        core_usb_sign_message(kc, apdu);
        break;
      case INS_GET_APP_CONF:
        core_usb_get_app_config(apdu);
        break;
      case INS_SIGN_EIP_712:
        core_usb_sign_eip712(kc, apdu);
        break;
      default:
        core_usb_err_sw(apdu, 0x6d, 0x00);
        break;
    }
  } else {
    core_usb_err_sw(apdu, 0x6e, 0x00);
  }

  command_init_send(cmd);
}

void core_usb_cancel() {
  //TODO: implement
}

void core_usb_run() {

}

app_err_t core_eip4527_init_sign(struct eth_sign_request *qr_request) {
  g_core.bip44_path_len = qr_request->_eth_sign_request_derivation_path._crypto_keypath_components__path_component_count * 4;

  if (g_core.bip44_path_len > BIP44_MAX_PATH_LEN) {
    g_core.bip44_path_len = 0;
    return ERR_DATA;
  }

  for (int i = 0; i < qr_request->_eth_sign_request_derivation_path._crypto_keypath_components__path_component_count; i++) {
    uint32_t idx = qr_request->_eth_sign_request_derivation_path._crypto_keypath_components__path_component[i]._path_component__child_index;
    if (qr_request->_eth_sign_request_derivation_path._crypto_keypath_components__path_component[i]._path_component__is_hardened) {
      idx |= 0x80000000;
    }

    g_core.bip44_path[(i * 4)] = idx >> 24;
    g_core.bip44_path[(i * 4) + 1] = (idx >> 16) & 0xff;
    g_core.bip44_path[(i * 4) + 2] = (idx >> 8) & 0xff;
    g_core.bip44_path[(i * 4) + 3] = idx & 0xff;
  }

  return core_init_sign();
}

void core_qr_run() {
  struct eth_sign_request qr_request;

  if (ui_qrscan(&qr_request) != CORE_EVT_UI_OK) {
    return;
  }

  if (core_eip4527_init_sign(&qr_request) != ERR_OK) {
    return;
  }

  app_err_t err;

  switch(qr_request._eth_sign_request_data_type._sign_data_type_choice) {
    case _sign_data_type__eth_transaction_data:
    case _sign_data_type__eth_typed_transaction:
      err = core_process_tx(qr_request._eth_sign_request_sign_data.value, qr_request._eth_sign_request_sign_data.len, 1);
      break;
    case _sign_data_type__eth_raw_bytes:
      g_core.data.msg.len = qr_request._eth_sign_request_sign_data.len;
      err = core_process_msg(qr_request._eth_sign_request_sign_data.value, qr_request._eth_sign_request_sign_data.len, 1);
      break;
    case _sign_data_type__eth_typed_data:
      err = ERR_UNSUPPORTED;
      break;
    default:
      err = ERR_UNSUPPORTED;
      break;
  }

  if(err != ERR_OK) {
    //TODO: handle this
    return;
  }

  if (core_sign(&g_core.keycard, g_core.data.sig.plain_sig) != ERR_OK) {
    return;
  }

  struct eth_signature sig = {0};
  sig._eth_signature_request_id_present = qr_request._eth_sign_request_request_id_present;
  if (sig._eth_signature_request_id_present) {
    sig._eth_signature_request_id._eth_signature_request_id.value = qr_request._eth_sign_request_request_id._eth_sign_request_request_id.value;
    sig._eth_signature_request_id._eth_signature_request_id.len = qr_request._eth_sign_request_request_id._eth_sign_request_request_id.len;
  }
  sig._eth_signature_signature.value = g_core.data.sig.plain_sig;
  sig._eth_signature_signature.len = SIGNATURE_LEN;
  cbor_encode_eth_signature(g_core.data.sig.cbor_sig, CBOR_SIG_MAX_LEN, &sig, &g_core.data.sig.cbor_len);
  ui_display_qr(g_core.data.sig.cbor_sig, g_core.data.sig.cbor_len, ETH_SIGNATURE);
}

void core_display_public() {
  struct hd_key key;
  key._hd_key_key_data.len = PUBKEY_COMPRESSED_LEN;
  key._hd_key_key_data.value = g_core.data.key.pub;
  key._hd_key_chain_code._hd_key_chain_code.len = CHAINCODE_LEN;
  key._hd_key_chain_code._hd_key_chain_code.value = g_core.data.key.chain;
  key._hd_key_chain_code_present = 1;
  key._hd_key_origin._hd_key_origin._crypto_keypath_depth_present = 1;
  key._hd_key_origin._hd_key_origin._crypto_keypath_depth._crypto_keypath_depth = ETH_DEFAULT_BIP44_LEN;
  key._hd_key_origin._hd_key_origin._crypto_keypath_source_fingerprint_present = 1;
  key._hd_key_origin._hd_key_origin._crypto_keypath_components__path_component_count = ETH_DEFAULT_BIP44_LEN;
  key._hd_key_origin_present = 1;
  key._hd_key_name_present = 1;
  key._hd_key_name._hd_key_name.len = EIP4527_NAME_LEN;
  key._hd_key_name._hd_key_name.value = EIP4527_NAME;
  key._hd_key_source_present = 0;

  uint16_t path_len = 0;
  for (int i = 0; i < ETH_DEFAULT_BIP44_LEN; i++) {
    uint32_t c = ETH_DEFAULT_BIP44[i];
    g_core.bip44_path[path_len++] = c >> 24;
    g_core.bip44_path[path_len++] = (c >> 16) & 0xff;
    g_core.bip44_path[path_len++] = (c >> 8) & 0xff;
    g_core.bip44_path[path_len++] = (c & 0xff);
    key._hd_key_origin._hd_key_origin._crypto_keypath_components__path_component[i]._path_component__child_index = c & 0x7fffffff;
    key._hd_key_origin._hd_key_origin._crypto_keypath_components__path_component[i]._path_component__is_hardened = c > 0x7fffffff;
  }

  SC_BUF(path, BIP44_MAX_PATH_LEN);
  memcpy(path, g_core.bip44_path, path_len);
  app_err_t err = core_export_key(&g_core.keycard, path, (path_len - 4), g_core.data.key.pub, NULL);

  if (err != ERR_OK) {
    //TODO: handle this
    return;
  }

  g_core.data.key.pub[0] = 0x02 | (g_core.data.key.pub[PUBKEY_LEN - 1] & 1);

  sha256_Raw(g_core.data.key.pub, PUBKEY_COMPRESSED_LEN, g_core.data.key.chain);
  ripemd160(g_core.data.key.chain, SHA256_DIGEST_LENGTH, g_core.data.key.pub);

  uint32_t fingerprint = (g_core.data.key.pub[0] << 24) | (g_core.data.key.pub[0] << 16) | (g_core.data.key.pub[0] << 8) | g_core.data.key.pub[0];
  key._hd_key_origin._hd_key_origin._crypto_keypath_source_fingerprint._crypto_keypath_source_fingerprint = fingerprint;

  memcpy(path, g_core.bip44_path, path_len);
  err = core_export_key(&g_core.keycard, path, path_len, g_core.data.key.pub, g_core.data.key.chain);

  if (err != ERR_OK) {
    //TODO: handle this
    return;
  }

  g_core.data.key.pub[0] = 0x02 | (g_core.data.key.pub[PUBKEY_LEN - 1] & 1);

  cbor_encode_hd_key(g_core.data.key.cbor_key, CBOR_KEY_MAX_LEN, &key, &g_core.data.key.cbor_len);
  ui_display_qr(g_core.data.key.cbor_key, g_core.data.key.cbor_len, CRYPTO_HDKEY);
}

void core_action_run(i18n_str_id_t menu) {
  switch(menu) {
  case MENU_QRCODE:
    core_qr_run();
    break;
  case MENU_DISPLAY_PUBLIC:
    core_display_public();
    break;
  default:
    //unhandled commands
    break;
  }
}

core_evt_t core_wait_event(uint8_t accept_usb) {
  uint32_t events;

  while(1) {
    BaseType_t res = pdFAIL;
    res = xTaskNotifyWaitIndexed(CORE_EVENT_IDX, 0, UINT32_MAX, &events, portMAX_DELAY);

    if (res != pdPASS) {
      continue;
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
  }
}
