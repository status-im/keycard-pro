#include "common.h"
#include "core.h"
#include "crypto/ecdsa.h"
#include "crypto/sha2.h"
#include "crypto/secp256k1.h"
#include "ethereum/eth_db.h"
#include "mem.h"
#include "iso7816/smartcard.h"
#include "storage/keys.h"
#include "pwr.h"
#include "ui/ui.h"
#include "ur/ur.h"

#define SIG_LEN 64
#define UPDATE_SEGMENT_LEN 240
#define FW_UPGRADE_REBOOT_DELAY 150

static app_err_t updater_verify_db(uint8_t* data, size_t data_len) {
  uint8_t digest[SHA256_DIGEST_LENGTH];
  size_t len = data_len - SIG_LEN;

  sha256_Raw(data, len, digest);

  const uint8_t* key;
  key_read(DB_VERIFICATION_KEY, &key);
  return ecdsa_verify_digest(&secp256k1, key, &data[len], digest) ? ERR_DATA : ERR_OK;
}

static app_err_t updater_database_update(uint8_t* data, size_t len) {
  if (updater_verify_db(data, len) != ERR_OK) {
    ui_info(INFO_ERROR_TITLE, LSTR(INFO_DB_UPDATE_INVALID), 1);
    return ERR_DATA;
  }

  if (eth_db_update((fs_entry_t *) data, len - SIG_LEN) != ERR_OK) {
    ui_info(INFO_ERROR_TITLE, LSTR(INFO_DB_UPDATE_ERROR), 1);
    return ERR_DATA;
  } else {
    ui_info(INFO_SUCCESS_TITLE, LSTR(INFO_DB_UPDATE_OK), 1);
  }

  return ERR_OK;
}

void updater_database_run() {
  data_t data;

  if (ui_qrscan(FS_DATA, &data) != CORE_EVT_UI_OK) {
    return;
  }

  updater_database_update(data.data, data.len);
}

static void updater_clear_flash_area() {
  const int fw_block = HAL_FLASH_ADDR_TO_BLOCK(HAL_FLASH_FW_UPGRADE_AREA);
  hal_flash_begin_program();

  for (int i = fw_block; i < (fw_block + HAL_FLASH_FW_BLOCK_COUNT); i++) {
    hal_flash_erase(i);
  }

  hal_flash_end_program();
}

static app_err_t updater_verify_firmware() {
  uint8_t* const fw_upgrade_area = (uint8_t*) HAL_FLASH_FW_UPGRADE_AREA;

  uint8_t digest[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha2;
  sha256_Init(&sha2);
  sha256_Update(&sha2, fw_upgrade_area, HAL_FW_HEADER_OFFSET);
  sha256_Update(&sha2, &fw_upgrade_area[HAL_FW_HEADER_OFFSET + SIG_LEN], ((HAL_FLASH_FW_BLOCK_COUNT * HAL_FLASH_BLOCK_SIZE) - (HAL_FW_HEADER_OFFSET + SIG_LEN)));
  sha256_Final(&sha2, digest);

  const uint8_t* key;
  key_read(FW_VERIFICATION_KEY, &key);
  return ecdsa_verify_digest(&secp256k1, key, &fw_upgrade_area[HAL_FW_HEADER_OFFSET], digest) ? ERR_DATA : ERR_OK;
}

static inline void updater_fw_switch() {
  g_bootcmd = BOOTCMD_SWITCH_FW;
  pwr_reboot();
}

void updater_usb_fw_upgrade(command_t *cmd, apdu_t* apdu) {
  apdu->has_lc = 1;
  uint8_t* data = APDU_DATA(apdu);
  size_t len = APDU_LC(apdu);
  uint8_t first_segment = APDU_P1(apdu) == 0;

  //TODO: show confirmation here, then show progress bar
  if (first_segment) {
    g_core.data.msg.len = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    g_core.data.msg.received = 0;
    data += 4;
    len -= 4;

    updater_clear_flash_area();
  }

  if ((len % HAL_FLASH_WORD_SIZE) || g_core.data.msg.received >= (HAL_FLASH_FW_BLOCK_COUNT * HAL_FLASH_BLOCK_SIZE)) {
    core_usb_err_sw(apdu, 0x69, 0x82);
    return;
  }

  uint8_t* const fw_upgrade_area = (uint8_t*) HAL_FLASH_FW_UPGRADE_AREA;
  hal_flash_begin_program();
  hal_flash_program(data, &fw_upgrade_area[g_core.data.msg.received], len);
  hal_flash_end_program();

  g_core.data.msg.received += len;

  if (g_core.data.msg.received > g_core.data.msg.len) {
    core_usb_err_sw(apdu, 0x69, 0x82);
    return;
  } else if (g_core.data.msg.received == g_core.data.msg.len) {
    if (updater_verify_firmware() != ERR_OK) {
      updater_clear_flash_area();
      core_usb_err_sw(apdu, 0x69, 0x82);
      ui_info(INFO_ERROR_TITLE, LSTR(INFO_FW_UPGRADE_INVALID), 1);
      return;
    }

    if (ui_info(INFO_SUCCESS_TITLE, LSTR(INFO_FW_UPGRADE_CONFIRM), 1) == CORE_EVT_UI_OK) {
      core_usb_err_sw(apdu, 0x90, 0x00);
      command_init_send(cmd);
      vTaskDelay(pdMS_TO_TICKS(FW_UPGRADE_REBOOT_DELAY));
      updater_fw_switch();
    }
  }

  core_usb_err_sw(apdu, 0x90, 0x00);
}

void updater_usb_db_upgrade(apdu_t* apdu) {
  apdu->has_lc = 1;
  uint8_t* data = APDU_DATA(apdu);
  size_t len = APDU_LC(apdu);
  uint8_t first_segment = APDU_P1(apdu) == 0;

  //TODO: show confirmation here, then show progress bar
  if (first_segment) {
    g_core.data.msg.len = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    g_core.data.msg.received = 0;
    data += 4;
    len -= 4;
  }

  if ((g_core.data.msg.received + len) > MEM_HEAP_SIZE) {
    core_usb_err_sw(apdu, 0x69, 0x82);
    return;
  }

  memcpy(&g_mem_heap[g_core.data.msg.received], data, len);

  g_core.data.msg.received += len;

  if (g_core.data.msg.received > g_core.data.msg.len) {
    core_usb_err_sw(apdu, 0x69, 0x82);
    return;
  } else if (g_core.data.msg.received == g_core.data.msg.len) {
    if (updater_database_update(g_mem_heap, g_core.data.msg.len) != ERR_OK) {
      core_usb_err_sw(apdu, 0x69, 0x82);
      return;
    }
  }

  core_usb_err_sw(apdu, 0x90, 0x00);
}
