#include "core.h"
#include "mem.h"
#include "bitcoin/psbt.h"
#include "crypto/sha2_soft.h"
#include "keycard/keycard_cmdset.h"
#include "util/tlv.h"

#define BTC_MAX_INPUTS 20
#define BTC_MAX_OUTPUTS 20

#define BTC_TXID_LEN 32
#define BTC_PUBKEY_HASH_LEN 20

#define SIGHASH_MASK 0x1f
#define SIGHASH_ANYONECANPAY 0x80

static const uint8_t P2WPKH_SCRIPT_PRE[4] = { 0x19, 0x76, 0xa9, 0x14 };
static const uint8_t P2WPKH_SCRIPT_POST[4] = { 0x88, 0xac };

enum btc_input_type {
  BTC_INPUT_TYPE_P2PKH,
  BTC_INPUT_TYPE_P2SH,
  BTC_INPUT_TYPE_P2WPKH,
  BTC_INPUT_TYPE_P2WSH,
};

enum btc_sighash_flag {
  SIGHASH_DEFAULT = 0x00,
  SIGHASH_ALL = 0x01,
  SIGHASH_NONE = 0x02,
  SIGHASH_SINGLE = 0x03,
};

typedef struct {
  uint8_t* pubkey_hash;
  uint8_t* amount;
  uint8_t* bip32_path;
  uint32_t bip32_path_len;
  uint32_t master_fingerprint;
  uint32_t sighash_flag;
  enum btc_input_type input_type;
} psbt_input_data_t;

struct btc_tx_ctx {
  psbt_tx_t tx;
  psbt_txin_t inputs[BTC_MAX_INPUTS];
  psbt_txout_t outputs[BTC_MAX_OUTPUTS];
  psbt_input_data_t input_data[BTC_MAX_INPUTS];
  size_t input_count;
  size_t output_count;

  psbt_t psbt_out;
  size_t index_in;
  size_t index_out;
  uint8_t hash_prevouts[SHA256_DIGEST_LENGTH];
  uint8_t hash_sequence[SHA256_DIGEST_LENGTH];
  uint8_t hash_outputs[SHA256_DIGEST_LENGTH];

  app_err_t error;
};

static void core_btc_psbt_rec_handler(struct btc_tx_ctx* tx_ctx, size_t index, psbt_record_t* rec) {
  if (rec->scope != PSBT_SCOPE_INPUTS) {
    return;
  } else if (index >= tx_ctx->input_count) {
    tx_ctx->error = ERR_DATA;
    return;
  }

  switch (rec->type) {
  case PSBT_IN_NON_WITNESS_UTXO:
  case PSBT_IN_REDEEM_SCRIPT:
  case PSBT_IN_WITNESS_SCRIPT:
    //TODO: implement
    tx_ctx->error = ERR_DECODE;
    break;
  case PSBT_IN_WITNESS_UTXO:
    if ((rec->val[8] != 22) || (rec->val[9] != 0x00) || (rec->val[10] != 20)) {
      tx_ctx->error = ERR_DECODE;
      return;
    }

    tx_ctx->input_data[index].input_type = BTC_INPUT_TYPE_P2WPKH;
    tx_ctx->input_data[index].amount = rec->val;
    tx_ctx->input_data[index].pubkey_hash = &rec->val[11];
    break;
  case PSBT_IN_BIP32_DERIVATION:
    memcpy(&tx_ctx->input_data[index].master_fingerprint, rec->val, sizeof(uint32_t));
    tx_ctx->input_data[index].bip32_path = &rec->val[4];
    tx_ctx->input_data[index].bip32_path_len = rec->val_size - 4;
    break;
  case PSBT_IN_SIGHASH_TYPE:
    memcpy(&tx_ctx->input_data[index].sighash_flag, rec->val, sizeof(uint32_t));
    break;
  }
}

static void core_btc_txelem_handler(struct btc_tx_ctx* tx_ctx, psbt_txelem_t* elem) {
  switch (elem->elem_type) {
  case PSBT_TXELEM_TX:
    memcpy(&tx_ctx->tx, elem->elem.tx, sizeof(psbt_tx_t));
    break;
  case PSBT_TXELEM_TXIN:
    if (tx_ctx->input_count < BTC_MAX_INPUTS) {
      memcpy(&tx_ctx->inputs[tx_ctx->input_count++], elem->elem.txin, sizeof(psbt_txin_t));
    } else {
      tx_ctx->error = ERR_FULL;
    }
    break;
  case PSBT_TXELEM_TXOUT:
    if (tx_ctx->output_count < BTC_MAX_OUTPUTS) {
      memcpy(&tx_ctx->outputs[tx_ctx->output_count++], elem->elem.txout, sizeof(psbt_txout_t));
    } else {
      tx_ctx->error = ERR_FULL;
    }
    break;
  default:
    break;
  }
}

static void core_btc_parser_cb(psbt_elem_t* rec) {
  struct btc_tx_ctx* tx_ctx = (struct btc_tx_ctx*) rec->user_data;

  if (rec->type == PSBT_ELEM_RECORD) {
    core_btc_psbt_rec_handler(tx_ctx, rec->index, rec->elem.rec);
  } else if (rec->type == PSBT_ELEM_TXELEM) {
    core_btc_txelem_handler(tx_ctx, rec->elem.txelem);
  }
}

static app_err_t core_btc_hash_legacy(struct btc_tx_ctx* tx_ctx, size_t index, uint8_t digest[SHA256_DIGEST_LENGTH]) {
  return ERR_UNSUPPORTED;
}

static app_err_t core_btc_hash_sigwit(struct btc_tx_ctx* tx_ctx, size_t index, uint8_t digest[SHA256_DIGEST_LENGTH]) {
  SHA256_CTX sha256;
  sha256_Init(&sha256);

  uint8_t sighash = tx_ctx->input_data[index].sighash_flag & SIGHASH_MASK;
  uint8_t anyonecanpay = tx_ctx->input_data[index].sighash_flag & SIGHASH_ANYONECANPAY;

  sha256_Update(&sha256, (uint8_t*) &tx_ctx->tx.version, sizeof(uint32_t));

  if (anyonecanpay) {
    sha256_Update(&sha256, ZERO32, SHA256_DIGEST_LENGTH);
  } else {
    sha256_Update(&sha256, tx_ctx->hash_prevouts, SHA256_DIGEST_LENGTH);
  }

  if (anyonecanpay || (sighash != SIGHASH_ALL)) {
    sha256_Update(&sha256, ZERO32, SHA256_DIGEST_LENGTH);
  } else {
    sha256_Update(&sha256, tx_ctx->hash_sequence, SHA256_DIGEST_LENGTH);
  }

  sha256_Update(&sha256, tx_ctx->inputs[index].txid, BTC_TXID_LEN);
  sha256_Update(&sha256, (uint8_t*) &tx_ctx->inputs[index].index, sizeof(uint32_t));

  if (tx_ctx->input_data[index].input_type == BTC_INPUT_TYPE_P2WPKH) {
    sha256_Update(&sha256, P2WPKH_SCRIPT_PRE, sizeof(P2WPKH_SCRIPT_PRE));
    sha256_Update(&sha256, tx_ctx->input_data[index].pubkey_hash, BTC_PUBKEY_HASH_LEN);
    sha256_Update(&sha256, P2WPKH_SCRIPT_POST, sizeof(P2WPKH_SCRIPT_POST));
  } else {
    //TODO: implement
    return ERR_UNSUPPORTED;
  }

  sha256_Update(&sha256, tx_ctx->input_data[index].amount, sizeof(uint64_t));
  sha256_Update(&sha256, (uint8_t*) &tx_ctx->inputs[index].sequence_number, sizeof(uint32_t));

  if (sighash == SIGHASH_ALL) {
    sha256_Update(&sha256, tx_ctx->hash_outputs, SHA256_DIGEST_LENGTH);
  } else if ((sighash == SIGHASH_SINGLE) && (index < tx_ctx->output_count)) {
    SOFT_SHA256_CTX inner_sha256;
    uint8_t inner_digest[SHA256_DIGEST_LENGTH];
    soft_sha256_Init(&inner_sha256);
    size_t output_len = ((uint32_t) tx_ctx->outputs[index].script - (uint32_t) tx_ctx->outputs[index].amount) + tx_ctx->outputs[index].script_len;
    soft_sha256_Update(&inner_sha256, tx_ctx->outputs[index].amount, output_len);
    soft_sha256_Final(&inner_sha256, inner_digest);

    soft_sha256_Init(&inner_sha256);
    soft_sha256_Update(&inner_sha256, inner_digest, SHA256_DIGEST_LENGTH);
    soft_sha256_Final(&inner_sha256, inner_digest);

    sha256_Update(&sha256, inner_digest, SHA256_DIGEST_LENGTH);
  } else {
    sha256_Update(&sha256, ZERO32, SHA256_DIGEST_LENGTH);
  }

  sha256_Update(&sha256, (uint8_t*) &tx_ctx->tx.lock_time, sizeof(uint32_t));
  sha256_Update(&sha256, (uint8_t*) &tx_ctx->input_data[index].sighash_flag, sizeof(uint32_t));

  sha256_Final(&sha256, digest);
  return ERR_OK;
}

static app_err_t core_btc_read_signature(uint8_t* data, uint8_t sighash, psbt_record_t* rec) {
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

  rec->scope = PSBT_SCOPE_INPUTS;
  rec->type = PSBT_IN_PARTIAL_SIG;

  rec->key = &data[off];
  rec->key_size = PUBKEY_COMPRESSED_LEN;
  rec->key[0] = 0x02 | (rec->key[64] & 1);

  rec->val = &rec->key[len];

  if (rec->val[0] != 0x30) {
    return ERR_DATA;
  }

  rec->val_size = 3 + rec->val[1];
  rec->val[rec->val_size - 1] = sighash;

  return ERR_OK;
}

static app_err_t core_btc_sign_input(struct btc_tx_ctx* tx_ctx, size_t index) {

  for (int i = 0; i < tx_ctx->input_data[i].bip32_path_len; i += 4) {
    g_core.bip44_path[i + 3] = tx_ctx->input_data[index].bip32_path[i];
    g_core.bip44_path[i + 2] = tx_ctx->input_data[index].bip32_path[i + 1];
    g_core.bip44_path[i + 1] = tx_ctx->input_data[index].bip32_path[i + 2];
    g_core.bip44_path[i] = tx_ctx->input_data[index].bip32_path[i + 3];
  }

  g_core.bip44_path_len = tx_ctx->input_data[index].bip32_path_len;

  uint32_t mfp;

  if (core_get_fingerprint(g_core.bip44_path, 0, &mfp) != ERR_OK) {
    return ERR_HW;
  }

  if (mfp != tx_ctx->input_data[index].master_fingerprint) {
    return ERR_OK;
  }

  app_err_t err;
  uint8_t digest[SHA256_DIGEST_LENGTH];

  switch(tx_ctx->input_data[index].input_type) {
  case BTC_INPUT_TYPE_P2PKH:
  case BTC_INPUT_TYPE_P2SH:
    err = core_btc_hash_legacy(tx_ctx, index, digest);
    break;
  case BTC_INPUT_TYPE_P2WPKH:
  case BTC_INPUT_TYPE_P2WSH:
    err = core_btc_hash_sigwit(tx_ctx, index, digest);
    break;
  default:
    err = ERR_DATA;
    break;
  }

  if (err != ERR_OK) {
    return err == ERR_UNSUPPORTED ? ERR_OK : err;
  }

  keycard_t *kc = &g_core.keycard;
  if ((keycard_cmd_sign(kc, g_core.bip44_path, g_core.bip44_path_len, digest, 0) != ERR_OK) || (APDU_SW(&kc->apdu) != 0x9000)) {
    return ERR_CRYPTO;
  }

  uint8_t* data = APDU_RESP(&kc->apdu);
  psbt_record_t signature;

  if (core_btc_read_signature(data, tx_ctx->input_data[index].sighash_flag, &signature) != ERR_OK) {
    return ERR_DATA;
  }

  psbt_write_input_record(&tx_ctx->psbt_out, &signature);

  return ERR_OK;
}

static void core_btc_sign_handler(psbt_elem_t* rec) {
  struct btc_tx_ctx* tx_ctx = (struct btc_tx_ctx*) rec->user_data;

  if ((rec->type == PSBT_ELEM_TXELEM) || tx_ctx->error != ERR_OK) {
    return;
  }

  switch(rec->elem.rec->scope) {
  case PSBT_SCOPE_GLOBAL:
    psbt_write_global_record(&tx_ctx->psbt_out, rec->elem.rec);
    break;
  case PSBT_SCOPE_INPUTS:
    if (rec->index != tx_ctx->index_in) {
      psbt_new_input_record_set(&tx_ctx->psbt_out);
      tx_ctx->index_in = rec->index;
      tx_ctx->error = core_btc_sign_input(tx_ctx, rec->index);
    }

    psbt_write_input_record(&tx_ctx->psbt_out, rec->elem.rec);
    break;
  case PSBT_SCOPE_OUTPUTS:
    if (rec->index != tx_ctx->index_out) {
      psbt_new_output_record_set(&tx_ctx->psbt_out);
      tx_ctx->index_out = rec->index;
    }

    psbt_write_output_record(&tx_ctx->psbt_out, rec->elem.rec);
    break;
  }
}

static app_err_t core_btc_validate(struct btc_tx_ctx* tx_ctx) {
  //TODO: implement validation
  for (int i = 0; i < tx_ctx->input_count; i++) {
    if (tx_ctx->input_data[i].sighash_flag == SIGHASH_DEFAULT) {
      tx_ctx->input_data[i].sighash_flag = SIGHASH_ALL;
    }
  }

  return ERR_OK;
}

static app_err_t core_btc_confirm(struct btc_tx_ctx* tx_ctx) {
  //TODO: implement
  return ERR_OK;
}

static void core_btc_common_hashes(struct btc_tx_ctx* tx_ctx) {
  uint8_t tmp_digest[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  sha256_Init(&sha256);

  for(int i = 0; i < tx_ctx->input_count; i++) {
    sha256_Update(&sha256, tx_ctx->inputs[i].txid, BTC_TXID_LEN);
    sha256_Update(&sha256, (uint8_t*) &tx_ctx->inputs[i].index, sizeof(uint32_t));
  }

  sha256_Final(&sha256, tmp_digest);
  sha256_Raw(tmp_digest, SHA256_DIGEST_LENGTH, tx_ctx->hash_prevouts);

  sha256_Init(&sha256);

  for(int i = 0; i < tx_ctx->input_count; i++) {
    sha256_Update(&sha256, (uint8_t*) &tx_ctx->inputs[i].sequence_number, sizeof(uint32_t));
  }

  sha256_Final(&sha256, tmp_digest);
  sha256_Raw(tmp_digest, SHA256_DIGEST_LENGTH, tx_ctx->hash_sequence);

  sha256_Init(&sha256);

  for(int i = 0; i < tx_ctx->output_count; i++) {
    size_t len = ((uint32_t) tx_ctx->outputs[i].script - (uint32_t) tx_ctx->outputs[i].amount) + tx_ctx->outputs[i].script_len;
    sha256_Update(&sha256, tx_ctx->outputs[i].amount, len);
  }

  sha256_Final(&sha256, tmp_digest);
  sha256_Raw(tmp_digest, SHA256_DIGEST_LENGTH, tx_ctx->hash_outputs);
}

void core_btc_psbt_run(struct zcbor_string* qr_request) {
  struct btc_tx_ctx* tx_ctx = (struct btc_tx_ctx*) g_camera_fb[1];
  memset(tx_ctx, 0, sizeof(struct btc_tx_ctx));

  psbt_t psbt;
  psbt_init(&psbt, (uint8_t*) qr_request->value, qr_request->len);
  psbt_read(qr_request->value, qr_request->len, &psbt, core_btc_parser_cb, tx_ctx);

  if (!tx_ctx->error != ERR_OK) {
    //TODO: add error message
    return;
  }

  if (core_btc_validate(tx_ctx) != ERR_OK || core_btc_confirm(tx_ctx) != ERR_OK) {
    return;
  }

  core_btc_common_hashes(tx_ctx);

  psbt_init(&psbt, (uint8_t*) qr_request->value, qr_request->len);
  psbt_init(&tx_ctx->psbt_out, &g_camera_fb[1][(sizeof(struct btc_tx_ctx) + 3) & ~0x3], CAMERA_FB_SIZE - ((sizeof(struct btc_tx_ctx) + 3) & ~0x3));

  tx_ctx->index_in = UINT32_MAX;
  tx_ctx->index_out = UINT32_MAX;
  psbt_read(qr_request->value, qr_request->len, &psbt, core_btc_sign_handler, tx_ctx);

  if (tx_ctx->error != ERR_OK) {
    //TODO: add error message
    return;
  }

  if (tx_ctx->psbt_out.state != PSBT_ST_OUTPUTS) {
    psbt_new_output_record_set(&tx_ctx->psbt_out);
  }

  psbt_finalize(&tx_ctx->psbt_out);

  //TODO: encode & output
}
