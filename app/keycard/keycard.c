#include <string.h>

#include "keycard.h"
#include "keycard_cmdset.h"
#include "application_info.h"
#include "pairing.h"
#include "error.h"
#include "ui/ui.h"
#include "util/tlv.h"
#include "common.h"
#include "crypto/address.h"
#include "crypto/rand.h"
#include "crypto/sha2.h"
#include "crypto/sha3.h"
#include "crypto/util.h"
#include "crypto/pbkdf2.h"
#include "crypto/bip39.h"
#include "crypto/ecdsa.h"
#include "crypto/secp256k1.h"
#include "ethereum/ethUstream.h"
#include "ethereum/ethUtils.h"

#define INS_GET_ETH_ADDR 0x02
#define INS_SIGN_ETH_TX 0x04
#define INS_GET_APP_CONF 0x06
#define INS_SIGN_ETH_MSG 0x08
#define INS_SIGN_ETH_MSG 0x08
#define INS_PROVIDE_ERC20 0x0A
#define INS_SIGN_EIP_712 0x0C
#define INS_SIGN_ETH_MSG 0x08
#define INS_PROVIDE_NFT 0x14

#define APP_MAJOR 1
#define APP_MINOR 9
#define APP_PATCH 17

#define BIP44_MAX_PATH_LEN 40
#define KEYCARD_AID_LEN 9
#define ETH_MSG_MAGIC_LEN 26
#define ETH_EIP712_MAGIC_LEN 2

typedef struct {
  uint8_t digest[SHA3_256_DIGEST_LENGTH];
  uint8_t bip44_path[BIP44_MAX_PATH_LEN];
  uint8_t bip44_path_len;
  uint32_t remaining;
  SHA3_CTX hash_ctx;
  txContent_t tx_data;
  txContext_t tx_ctx;
} Keycard_Sign_Ctx;

const uint8_t KEYCARD_AID[] = {0xa0, 0x00, 0x00, 0x08, 0x04, 0x00, 0x01, 0x01, 0x01};
const uint8_t KEYCARD_DEFAULT_PSK[] = {0x67, 0x5d, 0xea, 0xbb, 0x0d, 0x7c, 0x72, 0x4b, 0x4a, 0x36, 0xca, 0xad, 0x0e, 0x28, 0x08, 0x26, 0x15, 0x9e, 0x89, 0x88, 0x6f, 0x70, 0x82, 0x53, 0x5d, 0x43, 0x1e, 0x92, 0x48, 0x48, 0xbc, 0xf1};

const uint8_t* ETH_MSG_MAGIC = (uint8_t *) "\031Ethereum Signed Message:\n";
const uint8_t ETH_EIP712_MAGIC[] = { 0x19, 0x01 };

static Keycard_Sign_Ctx signing_ctx;

void Keycard_Init(Keycard* kc) {
  SmartCard_Init(&kc->sc);
  kc->ch.open = 0;
}

uint16_t Keycard_Init_Card(Keycard* kc, uint8_t* sc_key) {
  uint8_t pin[6];
  uint8_t puk[12];
  if (!ui_read_pin(pin, -1)) {
    return ERR_CANCEL;
  }

  if (!ui_read_puk(puk, -1)) {
    return ERR_CANCEL;
  }

  if (Keycard_CMD_Init(kc, sc_key, pin, puk, (uint8_t*)KEYCARD_DEFAULT_PSK) != ERR_OK) {
    ui_keycard_init_failed();
    return ERR_CRYPTO;
  }

  return ERR_OK;
}

uint16_t Keycard_Pair(Keycard* kc, Pairing* pairing, uint8_t* instance_uid) {
  memcpy(pairing->instance_uid, instance_uid, APP_INFO_INSTANCE_UID_LEN);
  
  if (Pairing_Read(pairing)) {
    ui_keycard_already_paired();
    return ERR_OK;
  }

  uint8_t* psk = (uint8_t*) KEYCARD_DEFAULT_PSK;
  
  while(1) {
    if (Keycard_CMD_AutoPair(kc, psk, pairing) == ERR_OK) {
      if (!Pairing_Write(pairing)) {
        ui_keycard_flash_failed();
        return ERR_DATA;
      }

      ui_keyard_paired();
      return ERR_OK;
    }

    uint8_t password[16];
    uint8_t pairing[32];
    uint32_t len = 16;
    psk = pairing;

    ui_keycard_pairing_failed();

    if (!ui_read_pairing(pairing, &len)) {
      return ERR_CANCEL;
    }

    pbkdf2_hmac_sha256(password, len, (uint8_t*)"Keycard Pairing Password Salt", 30, 50000, pairing, 32);
  }
}

uint16_t Keycard_FactoryReset(Keycard* kc) {
  //TODO: implement global platform
  return ERR_CANCEL;
}

uint16_t Keycard_Unblock(Keycard* kc, uint8_t pukRetries) {
  uint8_t pin[KEYCARD_PIN_LEN];

  if (pukRetries) {
    if (!ui_prompt_try_puk()) {
      pukRetries = 0;
    } else if (!ui_read_pin(pin, -1)) {
      return ERR_CANCEL;
    }
  }

  while(pukRetries) {
    uint8_t puk[KEYCARD_PUK_LEN];
    if (!ui_read_puk(puk, pukRetries)) {
      return ERR_CANCEL;
    }

    if (!Keycard_CMD_UnblockPIN(kc, pin, puk)) {
      return ERR_TXRX;
    }

    uint16_t sw = APDU_SW(&kc->apdu);

    if (sw == SW_OK) {
      ui_keycard_puk_ok();
      return ERR_OK;
    } else if ((sw & 0x63c0) == 0x63c0) {
      ui_keycard_wrong_puk();
      pukRetries = (sw & 0xf);
    } else {
      return sw;
    }    
  }

  return Keycard_FactoryReset(kc);
}

uint16_t Keycard_Authenticate(Keycard* kc) {
  if (!Keycard_CMD_GetStatus(kc)) {
    return ERR_TXRX;
  }

  APDU_ASSERT_OK(&kc->apdu);
  ApplicationStatus pinStatus;
  ApplicationStatus_Parse(APDU_RESP(&kc->apdu), &pinStatus);

  while(pinStatus.pin_retries) {
    SC_BUF(pin, KEYCARD_PIN_LEN);
    if (!ui_read_pin(pin, pinStatus.pin_retries)) {
      return ERR_CANCEL;
    }

    if (!Keycard_CMD_VerifyPIN(kc, pin)) {
      return ERR_TXRX;
    }

    uint16_t sw = APDU_SW(&kc->apdu);

    if (sw == SW_OK) {
      ui_keycard_pin_ok();
      return ERR_OK;
    } else if ((sw & 0x63c0) == 0x63c0) {
      ui_keycard_wrong_pin();
      pinStatus.pin_retries = (sw & 0xf);
    } else {
      return sw;
    }
  } 

  return Keycard_Unblock(kc, pinStatus.puk_retries);
}

uint16_t Keycard_Init_Keys(Keycard* kc) {
  uint16_t indexes[24];
  uint32_t len;

  uint16_t err = ui_read_mnemonic(indexes, &len);

  if (err == ERR_CANCEL) {
    return err;
  } else if (err == ERR_DATA) {
    if (!Keycard_CMD_GenerateMnemonic(kc, len)) {
      return ERR_TXRX;
    }

    APDU_ASSERT_OK(&kc->apdu);
    uint8_t* data = APDU_RESP(&kc->apdu);

    for (int i = 0; i < (len << 1); i += 2) {
      indexes[(i >> 1)] = ((data[i] << 8) | data[i+1]);
    }

    memset(data, 0, (len << 1));
  }

  const char* mnemonic = mnemonic_from_indexes(indexes, len);

  if (err == ERR_DATA) {
    if (!ui_backup_mnemonic(mnemonic)) {
      mnemonic_clear();
      return ERR_CANCEL;
    }
  }

  SC_BUF(seed, 64);
  mnemonic_to_seed(mnemonic, "\0", seed, NULL);
  mnemonic_clear();

  if(!Keycard_CMD_LoadSeed(kc, seed)) {
    return ERR_TXRX;
  }

  APDU_ASSERT_OK(&kc->apdu);

  ui_seed_loaded();
  return ERR_OK;
}

uint16_t Keycard_Setup(Keycard* kc) {
  if (!Keycard_CMD_Select(kc, KEYCARD_AID, KEYCARD_AID_LEN)) {
    return ERR_TXRX;
  }  

  if (APDU_SW(&kc->apdu) != SW_OK) {
    ui_keycard_wrong_card();
    return APDU_SW(&kc->apdu);
  }

  ApplicationInfo info;
  if (!ApplicationInfo_Parse(APDU_RESP(&kc->apdu), &info)) {
    ui_keycard_wrong_card();
    return ERR_DATA;
  }

  uint8_t initKeys;
  uint16_t err;

  switch (info.status) {
    case NOT_INITIALIZED:
      ui_keycard_not_initialized();
      err = Keycard_Init_Card(kc, info.sc_key);
      if (err != ERR_OK) {
        return err;
      }
      return ERR_RETRY;
    case INIT_NO_KEYS:
      initKeys = 1;
      ui_keycard_no_keys();
      break;
    case INIT_WITH_KEYS:
      initKeys = 0;
      ui_keycarrd_ready();
      break;
    default:
      return ERR_DATA;
  }

  Pairing pairing;
  err = Keycard_Pair(kc, &pairing, info.instance_uid);
  if (err != ERR_OK) {
    return err;
  }

  if (SecureChannel_Open(&kc->ch, &kc->sc, &kc->apdu, &pairing, info.sc_key) != ERR_OK) {
    Pairing_Erase(&pairing);
    ui_keycard_secure_channel_failed();
    return ERR_RETRY;
  }

  ui_keycard_secure_channel_ok();

  err = Keycard_Authenticate(kc);
  if (err != ERR_OK) {
    return err;
  }

  if (initKeys) {
    return Keycard_Init_Keys(kc);
  } else {
    return ERR_OK;
  }
}

void Keycard_Activate(Keycard* kc) {
  SmartCard_Activate(&kc->sc);
  
  ui_card_accepted();

  if (kc->sc.state != SC_READY) {
    ui_card_transport_error();
    return;
  }

  ui_clear();

  uint16_t res;
  do {
    res = Keycard_Setup(kc);
  } while(res == ERR_RETRY);

  if (res != ERR_OK) {
    ui_card_transport_error();
    SmartCard_Deactivate(&kc->sc);
  }
}

void Keycard_Error_SW(APDU* cmd, uint8_t sw1, uint8_t sw2) {
  uint8_t* data = APDU_RESP(cmd);
  data[0] = sw1;
  data[1] = sw2;
  cmd->lr = 2;
}

void Keycard_Get_App_Config(APDU* cmd) {
  uint8_t* data = APDU_RESP(cmd);
  data[0] = 0x03;
  data[1] = APP_MAJOR;
  data[2] = APP_MINOR;
  data[3] = APP_PATCH;
  data[4] = 0x90;
  data[5] = 0x00;
  cmd->lr = 6;
}

void Keycard_Get_Address(Keycard* kc, APDU* cmd) {
  uint8_t* data = APDU_DATA(cmd);
  uint16_t len = data[0] * 4;
  if (len > BIP44_MAX_PATH_LEN) {
    Keycard_Error_SW(cmd, 0x6a, 0x80);
    return;    
  }

  uint8_t extended = APDU_P2(cmd) == 1;
  uint8_t export_type = 1 + extended;

  SC_BUF(path, BIP44_MAX_PATH_LEN);
  memcpy(path, &data[1], len);

  if (!Keycard_CMD_ExportKey(kc, export_type, path, len) || (APDU_SW(&kc->apdu) != 0x9000)) {
    Keycard_Error_SW(cmd, 0x69, 0x82);
    return;
  }

  data = APDU_RESP(&kc->apdu);
  uint8_t* out = APDU_RESP(cmd);

  uint16_t tag;
  uint16_t off = tlv_read_tag(data, &tag);
  if (tag != 0xa1) {
    Keycard_Error_SW(cmd, 0x6f, 0x00);
    return;    
  }
  off += tlv_read_length(&data[off], &len);
  len = tlv_read_fixed_primitive(0x80, 65, &data[off], &out[1]);
  if (len == TLV_INVALID) {
    Keycard_Error_SW(cmd, 0x6f, 0x00);
    return;    
  }
  off += len;
  out[0] = 65;
  out[66] = 40;

  ethereum_address(&out[1], path);
  ethereum_address_checksum(path, (char *)&out[67]);

  len = 107;

  if (extended) {
    if (tlv_read_fixed_primitive(0x82, 32, &data[off], &out[len]) == TLV_INVALID) {
      Keycard_Error_SW(cmd, 0x6f, 0x00);
      return;    
    }
    len += 32;
  }
  
  if (APDU_P1(cmd) == 1) {
    if (!ui_confirm_eth_address((char *)&out[67])) {
      Keycard_Error_SW(cmd, 0x69, 0x82);
      return;
    }
  }

  out[len++] = 0x90;
  out[len++] = 0x00;

  cmd->lr = len;
}

uint8_t Keycard_Init_Sign(Keycard* kc, uint8_t* data) {
  signing_ctx.bip44_path_len = data[0] * 4;

  if (signing_ctx.bip44_path_len > BIP44_MAX_PATH_LEN) {
    signing_ctx.bip44_path_len = 0;
    return 0;    
  }

  memcpy(signing_ctx.bip44_path, &data[1], signing_ctx.bip44_path_len);

  keccak_256_Init(&signing_ctx.hash_ctx);
  return 1;
}

void Keycard_SignLegacy(APDU* cmd, uint8_t v_base, uint8_t* data, uint8_t* out) {
  uint16_t len;
  uint16_t tag;
  uint16_t off = tlv_read_tag(data, &tag);

  if (tag != 0xa0) {
    Keycard_Error_SW(cmd, 0x6f, 0x00);
    return; 
  }

  off += tlv_read_length(&data[off], &len);
  
  off += tlv_read_tag(&data[off], &tag);
  if (tag != 0x80) {
    Keycard_Error_SW(cmd, 0x6f, 0x00);
    return;    
  }
  off += tlv_read_length(&data[off], &len);

  uint8_t* pub = &data[off];

  uint8_t* sig = &out[1];
  if (ecdsa_sig_from_der(&pub[len], 72, sig)) {
    Keycard_Error_SW(cmd, 0x6f, 0x00);
    return;    
  }

  uint8_t* pub_tmp = &pub[len];

  for (int v = 0; v < 4; v++) {
    if (!ecdsa_recover_pub_from_sig(&secp256k1, pub_tmp, sig, signing_ctx.digest, v)) {
      if (!memcmp(pub_tmp, pub, 65)) {
        out[0] = v_base + v;
        out[65] = 0x90;
        out[66] = 0x00;
        cmd->lr = 67;
        return;
      }
    }
  }

  Keycard_Error_SW(cmd, 0x6f, 0x00);  
}

void Keycard_Sign(Keycard* kc, APDU* cmd, uint8_t v_base) {
  uint8_t* out = APDU_RESP(cmd);
  keccak_Final(&signing_ctx.hash_ctx, signing_ctx.digest);
  
  if (!Keycard_CMD_Sign(kc, signing_ctx.bip44_path, signing_ctx.bip44_path_len, signing_ctx.digest) || (APDU_SW(&kc->apdu) != 0x9000)) {
    Keycard_Error_SW(cmd, 0x69, 0x82);
    return;    
  }

  uint8_t* data = APDU_RESP(&kc->apdu);

  if (tlv_read_fixed_primitive(0x80, 65, data, &out[1]) == TLV_INVALID) {
    return Keycard_SignLegacy(cmd, v_base, data, out);
  }

  out[0] = v_base + out[65];
  out[65] = 0x90;
  out[66] = 0x00;
  cmd->lr = 67;
}

void Keycard_SignTX(Keycard* kc, APDU* cmd) {
  cmd->has_lc = 1;
  uint8_t* data = APDU_DATA(cmd);
  uint32_t len = APDU_LC(cmd);

  if (APDU_P1(cmd) == 0) {
    if (!Keycard_Init_Sign(kc, data)) {
      Keycard_Error_SW(cmd, 0x6a, 0x80);
      return;
    }

    data = &data[1+signing_ctx.bip44_path_len];
    len -= signing_ctx.bip44_path_len + 1;

    initTx(&signing_ctx.tx_ctx, &signing_ctx.hash_ctx, &signing_ctx.tx_data);
    
    if (len < 1) {
      Keycard_Error_SW(cmd, 0x6a, 0x80);
      return;  
    }

    // EIP 2718: TransactionType might be present before the TransactionPayload.
    uint8_t txType = data[0];
    if (txType >= MIN_TX_TYPE && txType <= MAX_TX_TYPE) {
      // Enumerate through all supported txTypes here...
      if (txType == EIP2930 || txType == EIP1559) {
        keccak_Update(&signing_ctx.hash_ctx, data, 1);
        signing_ctx.tx_ctx.txType = txType;
        data++;
        len--;
      } else {
        Keycard_Error_SW(cmd, 0x65, 0x01);
        return;  
      }
    } else {
      signing_ctx.tx_ctx.txType = LEGACY;
    }
  }

  if (signing_ctx.tx_ctx.currentField == RLP_NONE) {
    Keycard_Error_SW(cmd, 0x69, 0x85);
    return;
  }

  parserStatus_e res = processTx(&signing_ctx.tx_ctx, data, len, 0);
  switch (res) {
    case USTREAM_FINISHED: {
      uint8_t v_base;
      if (signing_ctx.tx_ctx.txType == EIP1559 || signing_ctx.tx_ctx.txType == EIP2930) {
        v_base = 0;
      } else {
        if (signing_ctx.tx_data.vLength == 0) {
          v_base = 27;
        } else {
          uint32_t v = (uint32_t) u64_from_BE(signing_ctx.tx_data.v, APP_MIN(4, signing_ctx.tx_data.vLength));
          v_base = (v * 2) + 35;
        }
      }
      Keycard_Sign(kc, cmd, v_base);
      break;
    }    
    case USTREAM_PROCESSING:
      Keycard_Error_SW(cmd, 0x90, 0x00);
      break;
    case USTREAM_FAULT:
    default:
      Keycard_Error_SW(cmd, 0x6a, 0x80);
      break;
  }
}

void Keycard_SignMessage(Keycard* kc, APDU* cmd) {
  cmd->has_lc = 1;
  uint8_t* data = APDU_DATA(cmd);
  uint32_t len = APDU_LC(cmd);

  if (APDU_P1(cmd) == 0) {
    if (!Keycard_Init_Sign(kc, data)) {
      Keycard_Error_SW(cmd, 0x6a, 0x80);
      return;
    }
    
    signing_ctx.remaining = (data[1+signing_ctx.bip44_path_len] << 24) | (data[2+signing_ctx.bip44_path_len] << 16) | (data[3+signing_ctx.bip44_path_len] << 8) | data[4+signing_ctx.bip44_path_len];
    keccak_Update(&signing_ctx.hash_ctx, ETH_MSG_MAGIC, ETH_MSG_MAGIC_LEN);
    uint8_t tmp[11];
    uint8_t* ascii_len = u32toa(signing_ctx.remaining, tmp, 11);
    keccak_Update(&signing_ctx.hash_ctx, ascii_len, 10 - (size_t)(ascii_len - tmp));
    len -= signing_ctx.bip44_path_len + 5;
    data = &data[signing_ctx.bip44_path_len + 5];    
  }

  if (signing_ctx.remaining < len) {
    Keycard_Error_SW(cmd, 0x6a, 0x80);
    return;
  }
  
  keccak_Update(&signing_ctx.hash_ctx, data, len);
  signing_ctx.remaining -= len;

  if (signing_ctx.remaining == 0) {
    Keycard_Sign(kc, cmd, 27);
  } else {
    Keycard_Error_SW(cmd, 0x90, 0x00);
  }
}

void Keycard_SignEIP712(Keycard* kc, APDU* cmd) {
  uint8_t* data = APDU_DATA(cmd);
  
  if (!Keycard_Init_Sign(kc, data)) {
    Keycard_Error_SW(cmd, 0x6a, 0x80);
    return;
  }

  keccak_Update(&signing_ctx.hash_ctx, ETH_EIP712_MAGIC, ETH_EIP712_MAGIC_LEN);
  keccak_Update(&signing_ctx.hash_ctx, &data[1+signing_ctx.bip44_path_len], (SHA3_256_DIGEST_LENGTH * 2));

  Keycard_Sign(kc, cmd, 27);
}

void Keycard_Command(Keycard* kc, Command* cmd) {
  APDU* apdu = &cmd->apdu;

  if (APDU_CLA(apdu) == 0xe0) {
    switch(APDU_INS(apdu)) {
      case INS_GET_ETH_ADDR:
        Keycard_Get_Address(kc, apdu);
        break;
      case INS_SIGN_ETH_TX:
        Keycard_SignTX(kc, apdu);
        break;
      case INS_SIGN_ETH_MSG:
        Keycard_SignMessage(kc, apdu);
        break;        
      case INS_GET_APP_CONF:
        Keycard_Get_App_Config(apdu);
        break;
      case INS_SIGN_EIP_712:
        Keycard_SignEIP712(kc, apdu);
        break;
      default:
        Keycard_Error_SW(apdu, 0x6d, 0x00);
        break;
    }
  } else {
    Keycard_Error_SW(apdu, 0x6e, 0x00);
  }

  Command_Init_Send(cmd);
}

void Keycard_Run(Keycard* kc, Command* cmd) {
  switch (kc->sc.state) {
    case SC_NOT_PRESENT:
    case SC_DEACTIVATED:
      break; // sleep unil interrupt!
    case SC_OFF:
      Keycard_Activate(kc);
      break;
    case SC_READY:
      if(cmd->status == COMMAND_COMPLETE) {
        Keycard_Command(kc, cmd);
      }
      break; // process commands
  }
}

void Keycard_In(Keycard* kc) {
  ui_card_inserted();
  SmartCard_In(&kc->sc);
}

void Keycard_Out(Keycard* kc) {
  ui_card_removed();
  SmartCard_Out(&kc->sc);
  kc->ch.open = 0;
}
