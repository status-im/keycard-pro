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
#include <stdio.h>
#include "board.h"
#include "pin_mux.h"
#include "camera_support.h"
#include "clock_config.h"
#include "MIMXRT1064.h"
#include "fsl_debug_console.h"
#include "hal.h"

hal_err_t hal_init(void) {
   /* Init board hardware. */
  BOARD_ConfigMPU();
  BOARD_InitBootPins();
  BOARD_EarlyPrepareCamera();
  BOARD_InitCSIPins();
  BOARD_InitBootClocks();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
  BOARD_InitDEBUG_UARTPins();
  /* Init FSL debug console. */
  BOARD_InitDebugConsole();
#endif

  return HAL_OK;
}

hal_err_t hal_camera_init(uint8_t fb[CAMERA_FB_COUNT][CAMERA_FB_SIZE]) {
  const camera_config_t cameraConfig = {
      .pixelFormat                = kVIDEO_PixelFormatRAW8,
      .bytesPerPixel              = CAMERA_BPP,
      .resolution                 = FSL_VIDEO_RESOLUTION(CAMERA_WIDTH, CAMERA_HEIGHT),
      .frameBufferLinePitch_Bytes = CAMERA_WIDTH * CAMERA_BPP,
      .interface                  = kCAMERA_InterfaceGatedClock,
      .controlFlags               = (kCAMERA_HrefActiveHigh | kCAMERA_DataLatchOnRisingEdge),
      .framePerSec                = 30,
  };

  BOARD_InitCameraResource();

  CAMERA_RECEIVER_Init(&cameraReceiver, &cameraConfig, NULL, NULL);

  if (kStatus_Success != CAMERA_DEVICE_Init(&cameraDevice, &cameraConfig)) {
    return HAL_ERROR;
  }

  CAMERA_DEVICE_Start(&cameraDevice);

  for (uint32_t i = 0; i < CAMERA_FB_COUNT; i++) {
    CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)(fb[i]));
  }

  CAMERA_RECEIVER_Start(&cameraReceiver);

  return HAL_OK;
}

hal_err_t hal_camera_next_frame(uint8_t** fb) {
  while (CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, (uint32_t*)fb) != kStatus_Success) {
    ;
  }

  return HAL_OK;
}

hal_err_t hal_camera_submit(uint8_t* fb) {
  CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)fb);
  return HAL_OK;
}
