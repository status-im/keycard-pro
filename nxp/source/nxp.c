/*
 * Copyright 2016-2023 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    nxp.c
 * @brief   Application entry point.
 */
#include <nxp_camera.h>
#include <stdio.h>
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_csi.h"
#include "MIMXRT1064.h"
#include "fsl_trng.h"
#include "fsl_lpuart.h"
#include "fsl_dcp.h"
#include "fsl_gpt.h"
#include "fsl_lpi2c.h"
#include "fsl_lpspi_edma.h"
#include "fsl_gpio.h"
#include "hal.h"

struct gpio_pin_spec {
    GPIO_Type* base;
    uint32_t pin;
};

struct gpio_pin_spec NXP_PIN_MAP[] = {
    {BOARD_CAMERA_PWDN_GPIO, BOARD_CAMERA_PWDN_PIN},
    {BOARD_CAMERA_RST_GPIO, BOARD_CAMERA_RST_PIN},
    {BOARD_LCD_CD_GPIO, BOARD_LCD_CD_PIN},
    {BOARD_LCD_RST_GPIO, BOARD_LCD_RST_PIN},
};

static dcp_handle_t sha256_handle;

hal_err_t hal_init(void) {
   /* Init board hardware. */
  BOARD_ConfigMPU();
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
#ifdef DEBUG
  BOARD_InitDEBUG_UART();
  BOARD_InitDebugConsole();
#endif

  BOARD_IO_Init();

  BOARD_Crypto_Init(&sha256_handle);
  BOARD_Timer_Init();

  return HAL_OK;
}

hal_err_t hal_i2c_send(hal_i2c_port_t port, uint8_t addr, const uint8_t* data, size_t len) {
  assert(port == I2C_CAMERA);

  lpi2c_master_transfer_t xfer;

  xfer.flags          = kLPI2C_TransferDefaultFlag;
  xfer.slaveAddress   = addr;
  xfer.direction      = kLPI2C_Write;
  xfer.subaddress     = 0;
  xfer.subaddressSize = 0;
  xfer.data           = (uint8_t*) data;
  xfer.dataSize       = len;

  return LPI2C_MasterTransferBlocking(BOARD_CAMERA_I2C_BASEADDR, &xfer) == kStatus_Success ? HAL_OK : HAL_ERROR;
}

hal_err_t hal_spi_send(hal_spi_port_t port, const uint8_t* data, size_t len) {
  assert(port == SPI_LCD);

  lpspi_transfer_t xfer = { 0 };
  xfer.txData = (uint8_t*) data;
  xfer.dataSize = len;

  return LPSPI_MasterTransferBlocking(BOARD_LCD_SPI_BASEADDR, &xfer) == kStatus_Success ? HAL_OK : HAL_ERROR;
}

hal_err_t hal_spi_send_dma(hal_spi_port_t port, const uint8_t* data, size_t len) {
  assert(port == SPI_LCD);

  return HAL_ERROR;
}

hal_err_t hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state) {
  if (NXP_PIN_MAP[pin].base == NULL) {
    return HAL_OK; // unconnected PIN
  }

  GPIO_WritePinOutput(NXP_PIN_MAP[pin].base, NXP_PIN_MAP[pin].pin, state);
  return HAL_OK;
}

hal_err_t hal_rng_next(uint8_t *buf, size_t len) {
  return TRNG_GetRandomData(TRNG, buf, len) == kStatus_Success ? HAL_OK : HAL_ERROR;
}

hal_err_t hal_sha256_init(hal_sha256_ctx_t* ctx) {
  return DCP_HASH_Init(DCP, &sha256_handle, ctx, kDCP_Sha256) == kStatus_Success ? HAL_OK : HAL_ERROR;
}

hal_err_t hal_sha256_update(hal_sha256_ctx_t* ctx, const uint8_t* data, size_t len) {
  return DCP_HASH_Update(DCP, ctx, data, len) == kStatus_Success ? HAL_OK : HAL_ERROR;
}

hal_err_t hal_sha256_finish(hal_sha256_ctx_t* ctx, uint8_t out[SHA256_DIGEST_LENGTH]) {
  return DCP_HASH_Finish(DCP, ctx, out, NULL) == kStatus_Success ? HAL_OK : HAL_ERROR;
}

hal_err_t hal_uart_send(hal_uart_port_t port, const uint8_t* data, size_t len) {
  assert(port == UART_LOG);
  return LPUART_WriteBlocking(BOARD_DEBUG_UART_BASEADDR, data, len) == kStatus_Success ? HAL_OK : HAL_ERROR;
}

hal_err_t hal_delay_us(uint32_t usec) {
  GPT_StartTimer(GPT1);
  while(GPT_GetCurrentTimerCount(GPT1) < usec) {}
  GPT_StopTimer(GPT1);
  return HAL_OK;
}
