/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_common.h"
#include "fsl_dcp.h"
#include "pin_mux.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_BASEADDR BOARD_INITDEBUG_UART_UART1_TXD_PERIPHERAL

#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE (115200U)
#endif /* BOARD_DEBUG_UART_BAUDRATE */

/*! @brief The board flash size */
#define BOARD_FLASH_SIZE (0x400000U)

/* @Brief Board CAMERA configuration */
#define BOARD_CAMERA_I2C_BASEADDR BOARD_INITCAMERA_CSI_I2C_SDA_PERIPHERAL

#define BOARD_CAMERA_PWDN_GPIO    BOARD_INITCAMERA_CSI_PWDN_PERIPHERAL
#define BOARD_CAMERA_PWDN_PIN     BOARD_INITCAMERA_CSI_PWDN_CHANNEL

#define BOARD_CAMERA_RST_GPIO     NULL
#define BOARD_CAMERA_RST_PIN      0

/* @Brief Board LCD configuration */
#define BOARD_LCD_SPI_BASEADDR BOARD_INITLCD_SD1_CMD_PERIPHERAL

#define BOARD_LCD_RST_GPIO     BOARD_INITLCD_LCD_RESET_GPIO
#define BOARD_LCD_RST_PIN      BOARD_INITLCD_LCD_RESET_GPIO_PIN

#define BOARD_LCD_CD_GPIO      BOARD_INITLCD_LCD_CD_GPIO
#define BOARD_LCD_CD_PIN       BOARD_INITLCD_LCD_CD_GPIO_PIN

#define BOARD_LCD_BAUD_RATE    60000000
#define BOARD_LCD_DMA_RX_CH    0
#define BOARD_LCD_DMA_TX_CH    1

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/
uint32_t BOARD_DebugConsoleSrcFreq(void);

void BOARD_InitDebugConsole(void);

void BOARD_ConfigMPU(void);

void BOARD_IO_Init();
void BOARD_Crypto_Init(dcp_handle_t* sha256_handle);
void BOARD_Timer_Init();
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
