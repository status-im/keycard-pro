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
#include "fsl_debug_console.h"
#include "fsl_trng.h"
#include "hal.h"

struct gpio_pin_spec {
    GPIO_Type* base;
    uint32_t pin;
};

struct gpio_pin_spec NXP_PIN_MAP[] = {
    {BOARD_CAMERA_PWDN_GPIO, BOARD_CAMERA_PWDN_PIN},
    {NULL, 0}, //{BOARD_CAMERA_RST_GPIO, BOARD_CAMERA_RST_PIN},
};

hal_err_t hal_init(void) {
   /* Init board hardware. */
  BOARD_ConfigMPU();
  BOARD_InitBootPins();
  BOARD_InitCSIPins();
  BOARD_InitBootClocks();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
  BOARD_InitDEBUG_UARTPins();
  /* Init FSL debug console. */
  BOARD_InitDebugConsole();
#endif

  BOARD_InitCameraResource();

  trng_config_t trngConfig;
  TRNG_GetDefaultConfig(&trngConfig);
  TRNG_Init(TRNG, &trngConfig);

  return HAL_OK;
}

hal_err_t hal_i2c_send(hal_i2c_port_t port, uint8_t addr, uint8_t* data, size_t len) {
  assert(port == I2C_CAMERA);
  return BOARD_LPI2C_Send(BOARD_CAMERA_I2C_BASEADDR, addr, 0, 0, data, len) == kStatus_Success ? HAL_OK : HAL_ERROR;
}

hal_err_t hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state) {
  if (pin == GPIO_CAMERA_RST) return HAL_OK; // dev board does not connect this PIN, will connect in real board though
  GPIO_WritePinOutput(NXP_PIN_MAP[pin].base, NXP_PIN_MAP[pin].pin, state);
  return HAL_OK;
}

hal_err_t hal_rng_next(uint8_t *buf, size_t len) {
  return TRNG_GetRandomData(TRNG, buf, len) == kStatus_Success ? HAL_OK : HAL_ERROR;
}


