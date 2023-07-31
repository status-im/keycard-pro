#include "common.h"
#include "crypto/ecdsa.h"
#include "crypto/sha2.h"
#include "crypto/secp256k1.h"
#include "ethereum/eth_db.h"
#include "storage/keys.h"
#include "ui/ui.h"
#include "ur/ur.h"

#define DB_SIG_LEN 64

app_err_t updater_verify_data(data_t* data) {
  uint8_t digest[SHA256_DIGEST_LENGTH];
  size_t len = data->len - DB_SIG_LEN;

  sha256_Raw(data->data, len, digest);

  const uint8_t* key;
  key_read(DB_VERIFICATION_KEY, &key);
  return ecdsa_verify_digest(&secp256k1, key, &data->data[len], digest) ? ERR_DATA : ERR_OK;
}

void updater_database_run() {
  data_t data;

  if (ui_qrscan(FS_DATA, &data) != CORE_EVT_UI_OK) {
    return;
  }

  if (updater_verify_data(&data) != ERR_OK) {
    ui_info(INFO_ERROR_TITLE, LSTR(INFO_DB_UPDATE_INVALID), 1);
    return;
  }

  if (eth_db_update((fs_entry_t *) data.data, data.len) != ERR_OK) {
    ui_info(INFO_ERROR_TITLE, LSTR(INFO_DB_UPDATE_ERROR), 1);
  } else {
    ui_info(INFO_SUCCESS_TITLE, LSTR(INFO_DB_UPDATE_OK), 1);
  }
}
