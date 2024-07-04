#include "core.h"
#include "mem.h"
#include "bitcoin/psbt.h"

#define BTC_MAX_INPUTS 10
#define BTC_MAX_OUTPUTS 10

enum btc_input_type {
  BTC_INPUT_TYPE_LEGACY,
  BTC_INPUT_TYPE_SEGWIT
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
  app_err_t error;
};

static void core_btc_psbt_rec_handler(struct btc_tx_ctx* tx_data, size_t index, psbt_record_t* rec) {
  if (rec->scope != PSBT_SCOPE_INPUTS) {
    return;
  } else if (index >= tx_data->input_count) {
    tx_data->error = ERR_DATA;
    return;
  }

  switch (rec->type) {
  case PSBT_IN_NON_WITNESS_UTXO:
  case PSBT_IN_REDEEM_SCRIPT:
  case PSBT_IN_WITNESS_SCRIPT:
    //TODO: implement
    tx_data->error = ERR_DECODE;
    break;
  case PSBT_IN_WITNESS_UTXO:
    if ((rec->val[8] != 22) || (rec->val[9] != 0x00) || (rec->val[10] != 20)) {
      tx_data->error = ERR_DECODE;
      return;
    }

    tx_data->input_data[index].input_type = BTC_INPUT_TYPE_SEGWIT;
    tx_data->input_data[index].amount = rec->val;
    tx_data->input_data[index].pubkey_hash = &rec->val[11];
    break;
  case PSBT_IN_BIP32_DERIVATION:
    memcpy(&tx_data->input_data[index].master_fingerprint, rec->val, sizeof(uint32_t));
    tx_data->input_data[index].bip32_path = &rec->val[4];
    tx_data->input_data[index].bip32_path_len = rec->val_size - 4;
    break;
  case PSBT_IN_SIGHASH_TYPE:
    memcpy(&tx_data->input_data[index].sighash_flag, rec->val, sizeof(uint32_t));
    break;
  }
}

static void core_btc_txelem_handler(struct btc_tx_ctx* tx_data, psbt_txelem_t* elem) {
  switch (elem->elem_type) {
  case PSBT_TXELEM_TX:
    memcpy(&tx_data->tx, elem->elem.tx, sizeof(psbt_tx_t));
    break;
  case PSBT_TXELEM_TXIN:
    if (tx_data->input_count < BTC_MAX_INPUTS) {
      memcpy(&tx_data->inputs[tx_data->input_count++], elem->elem.txin, sizeof(psbt_txin_t));
    } else {
      tx_data->error = ERR_FULL;
    }
    break;
  case PSBT_TXELEM_TXOUT:
    if (tx_data->output_count < BTC_MAX_OUTPUTS) {
      memcpy(&tx_data->outputs[tx_data->output_count++], elem->elem.txout, sizeof(psbt_txout_t));
    } else {
      tx_data->error = ERR_FULL;
    }
    break;
  default:
    break;
  }
}

static void core_btc_parser_cb(psbt_elem_t* rec) {
  struct btc_tx_ctx* tx_data = (struct btc_tx_ctx*) rec->user_data;

  if (rec->type == PSBT_ELEM_RECORD) {
    core_btc_psbt_rec_handler(tx_data, rec->index, rec->elem.rec);
  } else if (rec->type == PSBT_ELEM_TXELEM) {
    core_btc_txelem_handler(tx_data, rec->elem.txelem);
  }
}

void core_btc_psbt_run(struct zcbor_string* qr_request) {
  struct btc_tx_ctx* tx_ctx = (struct btc_tx_ctx*) &qr_request->value[(qr_request->len + 3) & ~0x3];
  memset(tx_ctx, 0, sizeof(struct btc_tx_ctx));

  psbt_t psbt;
  psbt_init(&psbt, (uint8_t*) qr_request->value, qr_request->len);
  psbt_read(qr_request->value, qr_request->len, &psbt, core_btc_parser_cb, tx_ctx);

  if (!tx_ctx->error != ERR_OK) {
    //TODO: add error message
    return;
  }


}
