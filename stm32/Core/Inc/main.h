/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern DCMI_HandleTypeDef hdcmi;
extern HASH_HandleTypeDef hhash;
extern I2C_HandleTypeDef hi2c2;
extern RNG_HandleTypeDef hrng;
extern SPI_HandleTypeDef hspi5;
extern SMARTCARD_HandleTypeDef hsmartcard6;
extern UART_HandleTypeDef huart3;
extern PCD_HandleTypeDef hpcd_USB_DRD_FS;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim2;
extern DMA_HandleTypeDef handle_GPDMA2_Channel5;
extern CRC_HandleTypeDef hcrc;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void MX_GPIO_Init(void);
void MX_GPDMA1_Init(void);
void MX_GPDMA2_Init(void);
void MX_USB_PCD_Init(void);
void MX_DCMI_Init(void);
void MX_RNG_Init(void);
void MX_ICACHE_Init(void);
void MX_HASH_Init(void);
void MX_I2C2_Init(void);
void MX_USART3_UART_Init(void);
void MX_TIM6_Init(void);
void MX_SPI5_Init(void);
void MX_USART6_SMARTCARD_Init(void);
void MX_TIM2_Init(void);
void MX_CRC_Init(void);

/* USER CODE BEGIN EFP */
void SystemClock_Config(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SC_DEFAULT_BAUD_RATE 10752
#define SC_DEFAULT_PSC 25
#define TRACE_CK_Pin GPIO_PIN_2
#define TRACE_CK_GPIO_Port GPIOE
#define TRACE_D0_Pin GPIO_PIN_3
#define TRACE_D0_GPIO_Port GPIOE
#define TRACE_D1_Pin GPIO_PIN_4
#define TRACE_D1_GPIO_Port GPIOE
#define TRACE_D2_Pin GPIO_PIN_5
#define TRACE_D2_GPIO_Port GPIOE
#define TRACE_D3_Pin GPIO_PIN_6
#define TRACE_D3_GPIO_Port GPIOE
#define GPIO_USER_BUTTON_Pin GPIO_PIN_13
#define GPIO_USER_BUTTON_GPIO_Port GPIOC
#define GPIO_KEYPAD_COL_2_Pin GPIO_PIN_3
#define GPIO_KEYPAD_COL_2_GPIO_Port GPIOF
#define GPIO_LED2_YELLOW_Pin GPIO_PIN_4
#define GPIO_LED2_YELLOW_GPIO_Port GPIOF
#define GPIO_LCD_CD_Pin GPIO_PIN_0
#define GPIO_LCD_CD_GPIO_Port GPIOA
#define GPIO_LCD_RST_Pin GPIO_PIN_1
#define GPIO_LCD_RST_GPIO_Port GPIOA
#define GPIO_LED1_GREEN_Pin GPIO_PIN_0
#define GPIO_LED1_GREEN_GPIO_Port GPIOB
#define GPIO_KEYPAD_COL_0_Pin GPIO_PIN_11
#define GPIO_KEYPAD_COL_0_GPIO_Port GPIOF
#define GPIO_KEYPAD_COL_1_Pin GPIO_PIN_15
#define GPIO_KEYPAD_COL_1_GPIO_Port GPIOF
#define GPIO_CAMERA_PWDN_Pin GPIO_PIN_10
#define GPIO_CAMERA_PWDN_GPIO_Port GPIOE
#define GPIO_CAMERA_RST_Pin GPIO_PIN_12
#define GPIO_CAMERA_RST_GPIO_Port GPIOE
#define T_VCP_TX_Pin GPIO_PIN_8
#define T_VCP_TX_GPIO_Port GPIOD
#define T_VCP_RX_Pin GPIO_PIN_9
#define T_VCP_RX_GPIO_Port GPIOD
#define GPIO_LED3_RED_Pin GPIO_PIN_4
#define GPIO_LED3_RED_GPIO_Port GPIOG
#define GPIO_KEYPAD_ROW_2_Pin GPIO_PIN_5
#define GPIO_KEYPAD_ROW_2_GPIO_Port GPIOG
#define GPIO_KEYPAD_ROW_3_Pin GPIO_PIN_6
#define GPIO_KEYPAD_ROW_3_GPIO_Port GPIOG
#define UCPD_FLT_Pin GPIO_PIN_7
#define UCPD_FLT_GPIO_Port GPIOG
#define GPIO_KEYPAD_ROW_1_Pin GPIO_PIN_8
#define GPIO_KEYPAD_ROW_1_GPIO_Port GPIOG
#define UCPD_DBn_Pin GPIO_PIN_9
#define UCPD_DBn_GPIO_Port GPIOA
#define USB_FS_N_Pin GPIO_PIN_11
#define USB_FS_N_GPIO_Port GPIOA
#define USB_FS_P_Pin GPIO_PIN_12
#define USB_FS_P_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define T_JTDI_Pin GPIO_PIN_15
#define T_JTDI_GPIO_Port GPIOA
#define GPIO_CARD_RST_Pin GPIO_PIN_10
#define GPIO_CARD_RST_GPIO_Port GPIOC
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define GPIO_KEYPAD_ROW_0_Pin GPIO_PIN_0
#define GPIO_KEYPAD_ROW_0_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
