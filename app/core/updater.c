#include "common.h"
#include "ethereum/eth_db.h"
#include "ui/ui.h"
#include "ur/ur.h"

app_err_t updater_verify_data(data_t* data) {
  //TODO: verify signature + sanity check
  return ERR_OK;
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
