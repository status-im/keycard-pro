#ifdef BOOTLOADER
#include "hal.h"
#include "mem.h"

struct boot_vectable {
  uint32_t initial_sp;
  void (*reset_handler)(void);
};

#define BOOTVTAB ((struct boot_vectable *)HAL_FLASH_FW_START_ADDR)

bool verify_firmware() {
  uint8_t digest[SHA256_DIGEST_LENGTH];
  hal_sha256_ctx_t sha256;
  hal_sha256_init(&sha256);
  hal_sha256_update(&sha256, (uint8_t *) HAL_FLASH_FW_START_ADDR, (HAL_FLASH_FW_BLOCK_COUNT * HAL_FLASH_BLOCK_SIZE));
  hal_sha256_finish(&sha256, digest);

  return true;
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
    //TODO: add error?
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
