#ifdef BOOTLOADER
#include "hal.h"
#include "mem.h"

struct boot_vectable {
  uint32_t initial_sp;
  void (*reset_handler)(void);
};

#define BOOTVTAB ((struct boot_vectable *)HAL_FLASH_FW_START_ADDR)

void bootloader_init() {

}

bool verify_firmware() {
  return true;
}

int main(void) {
  if (hal_boot_type() == BOOT_HOT) {
    g_bootcmd = 0;
  }

  bootloader_init();

  if (g_bootcmd == BOOTCMD_SWITCH_FW) {
    g_bootcmd = 0;
    hal_flash_switch_firmware();
  }

  if (!verify_firmware()) {
    //TODO: add error?
    g_bootcmd = BOOTCMD_SWITCH_FW;
    hal_reboot();
  }

  SCB->VTOR = HAL_FLASH_FW_START_ADDR;

  __set_MSP(BOOTVTAB->initial_sp);
  BOOTVTAB->reset_handler();
}
#endif
