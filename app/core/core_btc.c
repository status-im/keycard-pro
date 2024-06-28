#include "core.h"
#include "bitcoin/psbt.h"

void core_btc_psbt_run(struct zcbor_string* qr_request) {
  psbt_t psbt;
  psbt_init(&psbt, (uint8_t*) qr_request->value, qr_request->len);
  psbt_read(qr_request->value, qr_request->len, &psbt, NULL, NULL);
}
