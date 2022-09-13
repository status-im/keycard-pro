/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32g0c1e_eval_lcd.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define VCP_TX_Pin GPIO_PIN_10
#define VCP_TX_GPIO_Port GPIOC
#define VCP_RX_Pin GPIO_PIN_11
#define VCP_RX_GPIO_Port GPIOC
#define Tamper_key_Pin GPIO_PIN_13
#define Tamper_key_GPIO_Port GPIOC
#define EX_RESET_OD_Pin GPIO_PIN_0
#define EX_RESET_OD_GPIO_Port GPIOC
#define JOY_UP_Pin GPIO_PIN_2
#define JOY_UP_GPIO_Port GPIOC
#define JOY_DOWN_Pin GPIO_PIN_3
#define JOY_DOWN_GPIO_Port GPIOC
#define JOY_SEL_Pin GPIO_PIN_0
#define JOY_SEL_GPIO_Port GPIOA
#define SC_TX_Pin GPIO_PIN_2
#define SC_TX_GPIO_Port GPIOA
#define SC_1V8_Pin GPIO_PIN_3
#define SC_1V8_GPIO_Port GPIOA
#define SC_NCMDVCC_Pin GPIO_PIN_0
#define SC_NCMDVCC_GPIO_Port GPIOB
#define Potentiometer_Pin GPIO_PIN_2
#define Potentiometer_GPIO_Port GPIOB
#define SC_NOFF_Pin GPIO_PIN_12
#define SC_NOFF_GPIO_Port GPIOB
#define SC_NOFF_EXTI_IRQn EXTI4_15_IRQn
#define SC_5V3V_Pin GPIO_PIN_15
#define SC_5V3V_GPIO_Port GPIOB
#define JOY_RIGHT_Pin GPIO_PIN_7
#define JOY_RIGHT_GPIO_Port GPIOC
#define LED3_Pin GPIO_PIN_8
#define LED3_GPIO_Port GPIOD
#define LED4_Pin GPIO_PIN_9
#define LED4_GPIO_Port GPIOD
#define SC_RST_Pin GPIO_PIN_15
#define SC_RST_GPIO_Port GPIOA
#define JOY_LEFT_Pin GPIO_PIN_8
#define JOY_LEFT_GPIO_Port GPIOC
#define SC_CK_Pin GPIO_PIN_4
#define SC_CK_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_5
#define LED1_GPIO_Port GPIOD
#define LED2_Pin GPIO_PIN_6
#define LED2_GPIO_Port GPIOD
#define LCD_CS_OD_Pin GPIO_PIN_8
#define LCD_CS_OD_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
