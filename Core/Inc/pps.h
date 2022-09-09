/**
  ******************************************************************************
  * @file    Smartcard/inc/pps.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    30-Oct-2015
  * @brief   This file contains all the functions prototypes for the PPS library.
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
#ifndef _PPS_H
#define _PPS_H

/* Includes ------------------------------------------------------------------*/
#include "smartcard.h"

/* Exported constants --------------------------------------------------------*/
#define PPS_MAX_LENGTH            ((uint8_t)6)  /* Max lenght of the PPS frame */

#define PPSS   ((uint8_t)0)   /* PPS initial character */
#define PPS0   ((uint8_t)1)   /* PPS format character */
#define PPS1   ((uint8_t)2)   /* PPS parameter character 1 */
#define PPS2   ((uint8_t)3)   /* PPS parameter character 2 */
#define PPS3   ((uint8_t)4)   /* PPS parameter character 3 */

#define PPS_OK                    ((int32_t)0)  /* Negotiation OK */
#define PPS_HANDSAKE_ERROR        ((int32_t)-1) /* Agreement not reached */
#define PPS_COMMUNICATION_ERROR   ((int32_t)-2) /* Comunication error */

/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define PPS_HAS_PPS1(pps_buffer)   ((pps_buffer[1] & 0x10) == 0x10)
#define PPS_HAS_PPS2(pps_buffer)   ((pps_buffer[1] & 0x20) == 0x20)
#define PPS_HAS_PPS3(pps_buffer)   ((pps_buffer[1] & 0x40) == 0x40)

/* Exported functions ------------------------------------------------------- */
int32_t PPS_Exchange (SCProtocol_t * p_ptc, uint8_t* request, uint8_t* resp_length, uint8_t* pps1, uint32_t etu);

#endif /* _PPS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
