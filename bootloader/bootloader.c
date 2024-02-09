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
    0x88, 0x2f, 0xb0, 0x7a, 0x60, 0xeb, 0x97, 0x4f, 0x07, 0x46, 0x20, 0x9c, 0x02, 0x82, 0x0a, 0xba,
    0x3c, 0xd6, 0x39, 0xf9, 0xfe, 0xaf, 0xb9, 0x6f, 0x0d, 0x3c, 0x30, 0x94, 0xad, 0xbf, 0x11, 0xd5,
    0x9f, 0xb3, 0x9d, 0x8e, 0x79, 0x8a, 0x64, 0xdc, 0x97, 0xec, 0x86, 0xf9, 0x11, 0xc4, 0x51, 0x95,
    0xd2, 0x49, 0x94, 0xe6, 0xd8, 0x76, 0xcd, 0x6b, 0x01, 0x4e, 0x96, 0xb5, 0xfa, 0x3d, 0xda, 0x2a,
};

bool verify_firmware() {
  uint8_t* const fw_area = (uint8_t*) HAL_FLASH_FW_START_ADDR;

  uint8_t digest[SHA256_DIGEST_LENGTH];
  hal_sha256_ctx_t sha256;
  hal_sha256_init(&sha256);
  hal_sha256_update(&sha256, fw_area, HAL_FW_HEADER_OFFSET);
  hal_sha256_update(&sha256, &fw_area[HAL_FW_HEADER_OFFSET + 64], ((HAL_FLASH_FW_BLOCK_COUNT * HAL_FLASH_BLOCK_SIZE) - (HAL_FW_HEADER_OFFSET + 64)));

  hal_sha256_finish(&sha256, digest);

  return hal_ecdsa_verify(&secp256k1, FW_PUB, &fw_area[HAL_FW_HEADER_OFFSET], digest);
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
