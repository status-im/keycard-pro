/**
  ******************************************************************************
  * @file    Smartcard/inc/t1_protocol_param.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    30-Oct-2015
  * @brief   This file contains all the functions prototypes for the T=1 protocol
  *          parameters library.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __T1_PROTOCOL_PARAM_H
#define __T1_PROTOCOL_PARAM_H

/* Includes ------------------------------------------------------------------*/
#include "smartcard.h"
#include "atr.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint32_t Set_F_D_parameters(SCProtocol_t * p_t1, ATR_TypeDef* p_atr, uint32_t SC_clk);
void Set_IFSC(SCProtocol_t* p_t1, ATR_TypeDef* p_atr);
void Set_CWT_BWT(SCProtocol_t* p_t1, ATR_TypeDef* p_atr);
void Set_EGT(SMARTCARD_HandleTypeDef *hsc, ATR_TypeDef* p_atr);
void Set_EDC(SCProtocol_t* p_t1, ATR_TypeDef* p_atr);
void Set_Convention(SCProtocol_t* p_t1, ATR_TypeDef * p_atr);

#endif /* __T1_PROTOCOL_PARAM_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
