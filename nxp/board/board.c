/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_lpuart.h"
#include "board.h"
#include "fsl_lpi2c.h"
#include "fsl_iomuxc.h"
#include "fsl_trng.h"
#include "fsl_gpt.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Initialize debug console. */
void BOARD_InitDebugConsole(void) {
  lpuart_config_t cfg;
  LPUART_GetDefaultConfig(&cfg);
  cfg.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
  cfg.enableTx = 1;
  LPUART_Init(BOARD_DEBUG_UART_BASEADDR, &cfg, BOARD_BOOTCLOCKRUN_UART_CLK_ROOT);
}

void BOARD_IO_Init() {
  lpi2c_master_config_t lpi2cConfig = {0};
  LPI2C_MasterGetDefaultConfig(&lpi2cConfig);
  LPI2C_MasterInit(BOARD_CAMERA_I2C_BASEADDR, &lpi2cConfig, BOARD_BOOTCLOCKRUN_LPI2C_CLK_ROOT);

  lpspi_master_config_t lpspiConfig = {0};
  LPSPI_MasterGetDefaultConfig(&lpspiConfig);
  lpspiConfig.baudRate = BOARD_LCD_BAUD_RATE;
  lpspiConfig.pinCfg = kLPSPI_SdoInSdoOut;
  lpspiConfig.cpol = kLPSPI_ClockPolarityActiveLow;
  LPSPI_MasterInit(BOARD_LCD_SPI_BASEADDR, &lpspiConfig, BOARD_BOOTCLOCKRUN_LPSPI_CLK_ROOT);
}

void BOARD_Crypto_Init(dcp_handle_t* sha256_handle) {
  trng_config_t trngConfig;
  TRNG_GetDefaultConfig(&trngConfig);
  TRNG_Init(TRNG, &trngConfig);

  dcp_config_t dcpConfig;
  DCP_GetDefaultConfig(&dcpConfig);
  DCP_Init(DCP, &dcpConfig);
  sha256_handle->channel = kDCP_Channel0;
}

void BOARD_Timer_Init() {
  gpt_config_t gptCfg;
  GPT_GetDefaultConfig(&gptCfg);
  gptCfg.clockSource = kGPT_ClockSource_Osc;
  gptCfg.enableFreeRun = true;
  gptCfg.enableMode = true;
  gptCfg.divider = 24;
  GPT_Init(GPT1, &gptCfg);
}

/* MPU configuration. */
void BOARD_ConfigMPU(void) {
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
  extern uint32_t Image$$RW_m_ncache$$Base[];
  /* RW_m_ncache_unused is a auxiliary region which is used to get the whole size of noncache section */
  extern uint32_t Image$$RW_m_ncache_unused$$Base[];
  extern uint32_t Image$$RW_m_ncache_unused$$ZI$$Limit[];
  uint32_t nonCacheStart = (uint32_t)Image$$RW_m_ncache$$Base;
  uint32_t size          = ((uint32_t)Image$$RW_m_ncache_unused$$Base == nonCacheStart) ?
                      0 :
                      ((uint32_t)Image$$RW_m_ncache_unused$$ZI$$Limit - nonCacheStart);
#elif defined(__MCUXPRESSO)
  extern uint32_t __base_NCACHE_REGION;
  extern uint32_t __top_NCACHE_REGION;
  uint32_t nonCacheStart = (uint32_t)(&__base_NCACHE_REGION);
  __attribute__((unused)) uint32_t size = (uint32_t)(&__top_NCACHE_REGION) - nonCacheStart;
#elif defined(__ICCARM__) || defined(__GNUC__)
  extern uint32_t __NCACHE_REGION_START[];
  extern uint32_t __NCACHE_REGION_SIZE[];
  uint32_t nonCacheStart = (uint32_t)__NCACHE_REGION_START;
  uint32_t size          = (uint32_t)__NCACHE_REGION_SIZE;
#endif
  /* Disable I cache and D cache */
  if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR)) {
      SCB_DisableICache();
  }

  if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR)) {
      SCB_DisableDCache();
  }

  /* Disable MPU */
  ARM_MPU_Disable();

  /* MPU configure:
   * Use ARM_MPU_RASR(DisableExec, AccessPermission, TypeExtField, IsShareable, IsCacheable, IsBufferable,
   * SubRegionDisable, Size)
   * API in mpu_armv7.h.
   * param DisableExec       Instruction access (XN) disable bit,0=instruction fetches enabled, 1=instruction fetches
   * disabled.
   * param AccessPermission  Data access permissions, allows you to configure read/write access for User and
   * Privileged mode.
   *      Use MACROS defined in mpu_armv7.h:
   * ARM_MPU_AP_NONE/ARM_MPU_AP_PRIV/ARM_MPU_AP_URO/ARM_MPU_AP_FULL/ARM_MPU_AP_PRO/ARM_MPU_AP_RO
   * Combine TypeExtField/IsShareable/IsCacheable/IsBufferable to configure MPU memory access attributes.
   *  TypeExtField  IsShareable  IsCacheable  IsBufferable   Memory Attribute    Shareability        Cache
   *     0             x           0           0             Strongly Ordered    shareable
   *     0             x           0           1              Device             shareable
   *     0             0           1           0              Normal             not shareable   Outer and inner write
   * through no write allocate
   *     0             0           1           1              Normal             not shareable   Outer and inner write
   * back no write allocate
   *     0             1           1           0              Normal             shareable       Outer and inner write
   * through no write allocate
   *     0             1           1           1              Normal             shareable       Outer and inner write
   * back no write allocate
   *     1             0           0           0              Normal             not shareable   outer and inner
   * noncache
   *     1             1           0           0              Normal             shareable       outer and inner
   * noncache
   *     1             0           1           1              Normal             not shareable   outer and inner write
   * back write/read acllocate
   *     1             1           1           1              Normal             shareable       outer and inner write
   * back write/read acllocate
   *     2             x           0           0              Device              not shareable
   *  Above are normal use settings, if your want to see more details or want to config different inner/outter cache
   * policy.
   *  please refer to Table 4-55 /4-56 in arm cortex-M7 generic user guide <dui0646b_cortex_m7_dgug.pdf>
   * param SubRegionDisable  Sub-region disable field. 0=sub-region is enabled, 1=sub-region is disabled.
   * param Size              Region size of the region to be configured. use ARM_MPU_REGION_SIZE_xxx MACRO in
   * mpu_armv7.h.
   */
  /*
   * Add default region to deny access to whole address space to workaround speculative prefetch.
   * Refer to Arm errata 1013783-B for more details.
   *
   */
  /* Region 0 setting: Instruction access disabled, No data access permission. */
  MPU->RBAR = ARM_MPU_RBAR(0, 0x00000000U);
  MPU->RASR = ARM_MPU_RASR(1, ARM_MPU_AP_NONE, 0, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_4GB);

  /* Region 1 setting: Memory with Device type, not shareable, non-cacheable. */
  MPU->RBAR = ARM_MPU_RBAR(1, 0x80000000U);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_512MB);

  /* Region 2 setting: Memory with Device type, not shareable,  non-cacheable. */
  MPU->RBAR = ARM_MPU_RBAR(2, 0x60000000U);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_512MB);

#if defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1)
  /* Region 3 setting: Memory with Normal type, not shareable, outer/inner write back. */
  MPU->RBAR = ARM_MPU_RBAR(3, 0x70000000U);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_RO, 0, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_4MB);
#endif

  /* Region 4 setting: Memory with Device type, not shareable, non-cacheable. */
  MPU->RBAR = ARM_MPU_RBAR(4, 0x00000000U);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_1GB);

  /* Region 5 setting: Memory with Normal type, not shareable, outer/inner write back */
  MPU->RBAR = ARM_MPU_RBAR(5, 0x00000000U);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_128KB);

  /* Region 6 setting: Memory with Normal type, not shareable, outer/inner write back */
  MPU->RBAR = ARM_MPU_RBAR(6, 0x20000000U);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_128KB);

  /* Region 7 setting: Memory with Normal type, not shareable, outer/inner write back */
  MPU->RBAR = ARM_MPU_RBAR(7, 0x20200000U);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 0, 1, 0, ARM_MPU_REGION_SIZE_512KB);

  /* Region 8 setting: Memory with Normal type, not shareable, outer/inner write back */
  MPU->RBAR = ARM_MPU_RBAR(8, 0x20280000U);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 0, 1, 0, ARM_MPU_REGION_SIZE_256KB);

  /* Region 9 setting: Memory with Normal type, not shareable, outer/inner write back */
  MPU->RBAR = ARM_MPU_RBAR(9, 0x80000000U);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_32MB);

  /* Region 10 setting: Memory with Device type, not shareable, non-cacheable */
  MPU->RBAR = ARM_MPU_RBAR(10, 0x40000000);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_4MB);

  /* Region 11 setting: Memory with Device type, not shareable, non-cacheable */
  MPU->RBAR = ARM_MPU_RBAR(11, 0x42000000);
  MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_1MB);

  /* Enable MPU */
  ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

  /* Enable I cache and D cache */
  SCB_EnableDCache();
  SCB_EnableICache();
}
