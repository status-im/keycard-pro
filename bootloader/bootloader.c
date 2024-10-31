#ifdef BOOTLOADER
#include "hal.h"
#include "mem.h"
#include "crypto/secp256k1.h"

struct boot_vectable {
  uint32_t initial_sp;
  void (*reset_handler)(void);
};

#define BOOTVTAB ((struct boot_vectable *)HAL_FLASH_FW_START_ADDR)

__attribute__((section(".fw_verification_key"))) __attribute__((__used__)) const uint8_t FW_PUB[] = {
	0x95, 0xbf, 0x0a, 0xc8, 0x60, 0xea, 0xf0, 0x57, 0xdb, 0x73, 0xf0, 0x9b, 0x3e, 0xb8, 0x0c, 0x08,
	0xb7, 0xac, 0xe5, 0xd5, 0xb2, 0x78, 0x97, 0x08, 0x08, 0x11, 0x2e, 0xaf, 0x45, 0xdd, 0x06, 0xb0,
	0x62, 0x5d, 0x66, 0x7f, 0x10, 0x30, 0x41, 0x1b, 0xca, 0x91, 0x24, 0x2a, 0xb4, 0x08, 0x86, 0x89,
	0x06, 0x60, 0x47, 0xf0, 0xd3, 0x89, 0x1a, 0x68, 0xd6, 0x5a, 0x7e, 0xff, 0x0c, 0x59, 0x77, 0x20,
};

bool verify_firmware() {
  uint8_t* const fw_area = (uint8_t*) HAL_FLASH_FW_START_ADDR;

  uint8_t digest[SHA256_DIGEST_LENGTH];
  hal_sha256_ctx_t sha256;
  hal_sha256_init(&sha256);
  hal_sha256_update(&sha256, fw_area, HAL_FW_HEADER_OFFSET);
  hal_sha256_update(&sha256, &fw_area[HAL_FW_HEADER_OFFSET + 64], ((HAL_FLASH_FW_BLOCK_COUNT * HAL_FLASH_BLOCK_SIZE) - (HAL_FW_HEADER_OFFSET + 64)));

  hal_sha256_finish(&sha256, digest);

  return hal_ecdsa_verify(&secp256k1, FW_PUB, &fw_area[HAL_FW_HEADER_OFFSET], digest) == HAL_SUCCESS;
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
