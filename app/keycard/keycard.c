#include <string.h>

#include "keycard.h"
#include "keycard_cmdset.h"
#include "application_info.h"
#include "pairing.h"
#include "error.h"
#include "ui/ui.h"
#include "util/tlv.h"
#include "common.h"
#include "crypto/rand.h"
#include "crypto/sha2.h"
#include "crypto/pbkdf2.h"
#include "crypto/bip39.h"
#include "crypto/secp256k1.h"

#define KEYCARD_AID_LEN 9

const uint8_t KEYCARD_AID[] = {0xa0, 0x00, 0x00, 0x08, 0x04, 0x00, 0x01, 0x01, 0x01};
const uint8_t KEYCARD_DEFAULT_PSK[] = {0x67, 0x5d, 0xea, 0xbb, 0x0d, 0x7c, 0x72, 0x4b, 0x4a, 0x36, 0xca, 0xad, 0x0e, 0x28, 0x08, 0x26, 0x15, 0x9e, 0x89, 0x88, 0x6f, 0x70, 0x82, 0x53, 0x5d, 0x43, 0x1e, 0x92, 0x48, 0x48, 0xbc, 0xf1};

static void keycard_random_puk(uint8_t puk[KEYCARD_PUK_LEN]) {
  for (int i = 0; i < KEYCARD_PUK_LEN; i++) {
    puk[i] = '0' + random_uniform(9);
  }
}

static app_err_t keycard_init_card(keycard_t* kc, uint8_t* sc_key) {
  uint8_t pin[KEYCARD_PIN_LEN];
  uint8_t puk[KEYCARD_PUK_LEN];
  if (ui_read_pin(pin, PIN_NEW_CODE) != CORE_EVT_UI_OK) {
    return ERR_CANCEL;
  }

  keycard_random_puk(puk);

  if (keycard_cmd_init(kc, sc_key, pin, puk, (uint8_t*)KEYCARD_DEFAULT_PSK) != ERR_OK) {
    ui_keycard_init_failed();
    return ERR_CRYPTO;
  }

  return ERR_OK;
}

static app_err_t keycard_pair(keycard_t* kc, pairing_t* pairing, uint8_t* instance_uid) {
  memcpy(pairing->instance_uid, instance_uid, APP_INFO_INSTANCE_UID_LEN);
  
  if (pairing_read(pairing) == ERR_OK) {
    ui_keycard_already_paired();
    return ERR_OK;
  }

  uint8_t* psk = (uint8_t*) KEYCARD_DEFAULT_PSK;
  
  while(1) {
    if (keycard_cmd_autopair(kc, psk, pairing) == ERR_OK) {
      if (pairing_write(pairing) != ERR_OK) {
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

    if (ui_read_pairing(pairing, &len) != CORE_EVT_UI_OK) {
      return ERR_CANCEL;
    }

    pbkdf2_hmac_sha256(password, len, (uint8_t*)"Keycard Pairing Password Salt", 30, 50000, pairing, 32);
  }
}

static app_err_t keycard_factoryreset(keycard_t* kc) {
  //TODO: implement global platform
  return ERR_CANCEL;
}

static app_err_t keycard_unblock(keycard_t* kc, uint8_t pukRetries) {
  uint8_t pin[KEYCARD_PIN_LEN];

  if (pukRetries) {
    if (ui_prompt_try_puk() != CORE_EVT_UI_OK) {
      pukRetries = 0;
    } else if (ui_read_pin(pin, PIN_NEW_CODE) != CORE_EVT_UI_OK) {
      return ERR_CANCEL;
    }
  }

  while(pukRetries) {
    uint8_t puk[KEYCARD_PUK_LEN];
    if (ui_read_puk(puk, pukRetries) != CORE_EVT_UI_OK) {
      return ERR_CANCEL;
    }

    if (keycard_cmd_unblock_pin(kc, pin, puk) != ERR_OK) {
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

  return keycard_factoryreset(kc);
}

static app_err_t keycard_authenticate(keycard_t* kc) {
  if (keycard_cmd_get_status(kc) != ERR_OK) {
    return ERR_TXRX;
  }

  APDU_ASSERT_OK(&kc->apdu);
  app_status_t pinStatus;
  application_status_parse(APDU_RESP(&kc->apdu), &pinStatus);

  while(pinStatus.pin_retries) {
    SC_BUF(pin, KEYCARD_PIN_LEN);
    if (ui_read_pin(pin, pinStatus.pin_retries) != CORE_EVT_UI_OK) {
      return ERR_CANCEL;
    }

    if (keycard_cmd_verify_pin(kc, pin) != ERR_OK) {
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

  return keycard_unblock(kc, pinStatus.puk_retries);
}

static app_err_t keycard_init_keys(keycard_t* kc) {
  uint16_t indexes[24];
  uint32_t len;

  core_evt_t err = ui_read_mnemonic(indexes, &len);

  if (err == CORE_EVT_UI_CANCELLED) {
    if (keycard_cmd_generate_mnemonic(kc, len) != ERR_OK) {
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

  if (err == CORE_EVT_UI_CANCELLED) {
    if (ui_backup_mnemonic(mnemonic) != CORE_EVT_UI_OK) {
      mnemonic_clear();
      return ERR_CANCEL;
    }
  }

  SC_BUF(seed, 64);
  mnemonic_to_seed(mnemonic, "\0", seed, NULL);
  mnemonic_clear();

  if(keycard_cmd_load_seed(kc, seed) != ERR_OK) {
    return ERR_TXRX;
  }

  APDU_ASSERT_OK(&kc->apdu);

  ui_seed_loaded();
  return ERR_OK;
}

static app_err_t keycard_setup(keycard_t* kc) {
  if (keycard_cmd_select(kc, KEYCARD_AID, KEYCARD_AID_LEN) != ERR_OK) {
    return ERR_TXRX;
  }  

  if (APDU_SW(&kc->apdu) != SW_OK) {
    ui_keycard_wrong_card();
    return APDU_SW(&kc->apdu);
  }

  app_info_t info;
  if (application_info_parse(APDU_RESP(&kc->apdu), &info) != ERR_OK) {
    ui_keycard_wrong_card();
    return ERR_DATA;
  }

  uint8_t initKeys;
  uint16_t err;

  switch (info.status) {
    case NOT_INITIALIZED:
      ui_keycard_not_initialized();
      err = keycard_init_card(kc, info.sc_key);
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
      ui_keycard_ready();
      break;
    default:
      return ERR_DATA;
  }

  pairing_t pairing;
  err = keycard_pair(kc, &pairing, info.instance_uid);
  if (err != ERR_OK) {
    return err;
  }

  if (securechannel_open(&kc->ch, &kc->sc, &kc->apdu, &pairing, info.sc_key) != ERR_OK) {
    pairing_erase(&pairing);
    ui_keycard_secure_channel_failed();
    return ERR_RETRY;
  }

  ui_keycard_secure_channel_ok();

  err = keycard_authenticate(kc);
  if (err != ERR_OK) {
    return err;
  }

  if (initKeys) {
    return keycard_init_keys(kc);
  } else {
    return ERR_OK;
  }
}

void keycard_init(keycard_t* kc) {
  smartcard_init(&kc->sc);
  kc->ch.open = 0;
}

void keycard_activate(keycard_t* kc) {
  smartcard_activate(&kc->sc);
  
  ui_card_accepted();

  if (kc->sc.state != SC_READY) {
    ui_card_transport_error();
    return;
  }

  uint16_t res;
  do {
    res = keycard_setup(kc);
  } while(res == ERR_RETRY);

  if (res != ERR_OK) {
    ui_card_transport_error();
    smartcard_deactivate(&kc->sc);
  }
}

app_err_t keycard_read_signature(uint8_t* data, uint8_t* digest, uint8_t* out_sig) {
  if (tlv_read_fixed_primitive(0x80, 65, data, out_sig) != TLV_INVALID) {
    return ERR_OK;
  }

  uint16_t len;
  uint16_t tag;
  uint16_t off = tlv_read_tag(data, &tag);

  if (tag != 0xa0) {
    return ERR_DATA;
  }

  off += tlv_read_length(&data[off], &len);

  off += tlv_read_tag(&data[off], &tag);
  if (tag != 0x80) {
    return ERR_DATA;
  }
  off += tlv_read_length(&data[off], &len);

  uint8_t* pub = &data[off];

  if (ecdsa_sig_from_der(&pub[len], 72, out_sig)) {
    return ERR_DATA;
  }

  uint8_t* pub_tmp = &pub[len];

  for (int i = 0; i < 4; i++) {
    if (!ecdsa_recover_pub_from_sig(&secp256k1, pub_tmp, out_sig, digest, i)) {
      if (!memcmp(pub_tmp, pub, 65)) {
        out_sig[64] = i;
        return ERR_OK;
      }
    }
  }

  return ERR_DATA;
}

void keycard_in(keycard_t* kc) {
  ui_card_inserted();
  smartcard_in(&kc->sc);
}

void keycard_out(keycard_t* kc) {
  ui_card_removed();
  smartcard_out(&kc->sc);
  kc->ch.open = 0;
}
