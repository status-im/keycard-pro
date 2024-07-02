#include "core.h"
#include "bitcoin/psbt.h"

void core_btc_tx_elem_handler(psbt_elem_t* rec) {
  if (rec->type == PSBT_ELEM_RECORD) {

  } else if (rec->type == PSBT_ELEM_TXELEM) {

  }
}

void core_btc_psbt_run(struct zcbor_string* qr_request) {
  psbt_t psbt;
  psbt_init(&psbt, (uint8_t*) qr_request->value, qr_request->len);
  psbt_read(qr_request->value, qr_request->len, &psbt, core_btc_tx_elem_handler, NULL);
}
