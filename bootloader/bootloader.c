#ifdef BOOTLOADER
#include "hal.h"
#include "mem.h"
#include "crypto/secp256k1.h"

struct boot_vectable {
  uint32_t initial_sp;
  void (*reset_handler)(void);
};

#define BOOTVTAB ((struct boot_vectable *)HAL_FLASH_FW_START_ADDR)

const uint8_t _FW__PUB[] = {
  0x02,
  0x01, 0x55, 0x78, 0x92, 0xf2, 0x8f, 0x2d, 0xa7, 0x04, 0xe5, 0x52, 0x27, 0x05, 0x3a, 0x2b, 0x7f,
  0x47, 0x76, 0x4c, 0x79, 0x39, 0xb6, 0xc9, 0x87, 0x61, 0x1f, 0xde, 0xfe, 0xcb, 0x50, 0xfe, 0x10,
};

bool verify_firmware() {
  uint8_t* const fw_area = (uint8_t*) HAL_FLASH_FW_START_ADDR;

  uint8_t digest[SHA256_DIGEST_LENGTH];
  hal_sha256_ctx_t sha256;
  hal_sha256_init(&sha256);
  hal_sha256_update(&sha256, fw_area, HAL_FW_HEADER_OFFSET);
  hal_sha256_update(&sha256, &fw_area[HAL_FW_HEADER_OFFSET + 64], ((HAL_FLASH_FW_BLOCK_COUNT * HAL_FLASH_BLOCK_SIZE) - (HAL_FW_HEADER_OFFSET + 64)));

  hal_sha256_finish(&sha256, digest);

  return !ecdsa_verify(&secp256k1, _FW__PUB, &fw_area[HAL_FW_HEADER_OFFSET], digest);
}

int main(void) {
  if (hal_boot_type() != BOOT_HOT) {
    g_bootcmd = 0;
  }

  hal_init_bootloader();

  if (g_bootcmd == BOOTCMD_SWITCH_FW) {
    g_bootcmd = 0;
    hal_flash_switch_firmware();
  }

  if (!verify_firmware()) {
    g_bootcmd = BOOTCMD_SWITCH_FW;
    hal_reboot();
  }

  __disable_irq();
  hal_teardown_bootloader();

  SCB->VTOR = HAL_FLASH_FW_START_ADDR;

  __set_MSP(BOOTVTAB->initial_sp);
  BOOTVTAB->reset_handler();
}
#endif
