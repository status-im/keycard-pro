#ifndef __UI_H
#define __UI_H

#include <stdint.h>
#include "ethereum/ethUstream.h"
#include "menu.h"
#include "ur/eip4527_types.h"
#include "ur/ur.h"

#define PIN_NEW_CODE -1
#define PUK_NEW_CODE PIN_NEW_CODE

typedef enum {
  CORE_EVT_USB_CMD,
  CORE_EVT_UI_CANCELLED,
  CORE_EVT_UI_OK,
  CORE_EVT_NONE,
} core_evt_t;

core_evt_t ui_qrscan(ur_type_t type, void* out);
core_evt_t ui_menu(const char* title, const menu_t* menu, i18n_str_id_t* selected, uint8_t allow_usb);
core_evt_t ui_display_tx(const uint8_t* address, const txContent_t* tx);
core_evt_t ui_display_msg(const uint8_t* address, const uint8_t* msg, uint32_t len);
core_evt_t ui_display_qr(const uint8_t* data, uint32_t len, ur_type_t type);
core_evt_t ui_info(const char* title, const char* msg, uint8_t dismissable);

void ui_card_inserted();
void ui_card_removed();
void ui_card_transport_error();
void ui_card_accepted();
void ui_keycard_wrong_card();
void ui_keycard_old_card();
void ui_keycard_not_initialized();
void ui_keycard_init_failed();
void ui_keycard_no_keys();
void ui_keycard_ready();
void ui_keycard_paired();
void ui_keycard_already_paired();
void ui_keycard_pairing_failed();
void ui_keycard_flash_failed();
void ui_keycard_secure_channel_failed();
void ui_keycard_secure_channel_ok();
void ui_keycard_wrong_pin();
void ui_keycard_pin_ok();
void ui_keycard_puk_ok();
void ui_keycard_wrong_puk();
void ui_seed_loaded();

core_evt_t ui_prompt_try_puk();
core_evt_t ui_confirm_factory_reset();
core_evt_t ui_read_pin(uint8_t* out, int8_t retries, uint8_t dismissable);
core_evt_t ui_read_puk(uint8_t* out, int8_t retries, uint8_t dismissable);
core_evt_t ui_read_pairing(uint8_t* pairing, uint8_t* len);
core_evt_t ui_read_string(const char* title, char* out, uint8_t* len);

core_evt_t ui_read_mnemonic_len(uint32_t* len);
core_evt_t ui_backup_mnemonic(uint16_t* indexes, uint32_t len);
core_evt_t ui_read_mnemonic(uint16_t* indexes, uint32_t len);

core_evt_t ui_confirm_eth_address(const char* address);

core_evt_t ui_device_auth(uint32_t first_auth, uint32_t auth_time, uint32_t auth_count);
core_evt_t ui_settings_brightness(uint8_t* brightness);

core_evt_t ui_keycard_not_genuine();

void ui_update_progress(const char* title, uint8_t progress);

#endif
