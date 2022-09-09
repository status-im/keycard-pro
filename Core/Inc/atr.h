/**
  ******************************************************************************
  * @file    Smartcard/inc/atr.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    30-Oct-2015
  * @brief   This file contains all the functions prototypes for the ATR library.
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
#ifndef _ATR_H
#define _ATR_H

/* Includes ------------------------------------------------------------------*/
#include "smartcard.h"

/* Exported constants --------------------------------------------------------*/
/* Return values */
#define ATR_OK        ((uint8_t)0) /* ATR could be parsed and data returned */
#define ATR_INVERSE   ((uint8_t)1) /* ATR received in indirect mode */
#define ATR_MALFORMED ((uint8_t)2) /* ATR could not be parsed */
#define ATR_IO_ERROR  ((uint8_t)3) /* I/O stream error */
#define ATR_NOT_FOUND ((uint8_t)4) /* Data not present in ATR */

/* Paramenters */
#define ATR_MAX_SIZE           ((uint8_t)33) /* Maximum size of ATR byte array */
#define ATR_MAX_HISTORICAL     ((uint8_t)15) /* Maximum number of historical bytes */
#define ATR_MAX_PROTOCOLS      ((uint8_t)7) /* Maximun number of protocols */
#define ATR_MAX_IB             ((uint8_t)4) /* Maximum number of interface bytes per protocol */
#define ATR_CONVENTION_DIRECT  ((uint8_t)0) /* Direct convention */
#define ATR_CONVENTION_INVERSE ((uint8_t)1) /* Inverse convention */
#define ATR_PROTOCOL_TYPE_T0   ((uint8_t)0) /* Protocol type T=0 */
#define ATR_PROTOCOL_TYPE_T1   ((uint8_t)1) /* Protocol type T=1 */
#define ATR_PROTOCOL_TYPE_T2   ((uint8_t)2) /* Protocol type T=2 */
#define ATR_PROTOCOL_TYPE_T3   ((uint8_t)3) /* Protocol type T=3 */
#define ATR_PROTOCOL_TYPE_T14  ((uint8_t)14) /* Protocol type T=14 */
#define ATR_INTERFACE_BYTE_TA  ((uint8_t)0) /* Interface byte TAi */
#define ATR_INTERFACE_BYTE_TB  ((uint8_t)1) /* Interface byte TBi */
#define ATR_INTERFACE_BYTE_TC  ((uint8_t)2) /* Interface byte TCi */
#define ATR_INTERFACE_BYTE_TD  ((uint8_t)3) /* Interface byte TDi */
#define ATR_PARAMETER_F        ((uint8_t)0) /* Parameter F */
#define ATR_PARAMETER_D        ((uint8_t)1) /* Parameter D */
#define ATR_PARAMETER_I        ((uint8_t)2) /* Parameter I */
#define ATR_PARAMETER_P        ((uint8_t)3) /* Parameter P */
#define ATR_PARAMETER_N        ((uint8_t)4) /* Parameter N */
#define ATR_INTEGER_VALUE_FI   ((uint8_t)0) /* Integer value FI */
#define ATR_INTEGER_VALUE_DI   ((uint8_t)1) /* Integer value DI */
#define ATR_INTEGER_VALUE_II   ((uint8_t)2) /* Integer value II */
#define ATR_INTEGER_VALUE_PI1  ((uint8_t)3) /* Integer value PI1 */
#define ATR_INTEGER_VALUE_N    ((uint8_t)4) /* Integer value N */
#define ATR_INTEGER_VALUE_PI2  ((uint8_t)5) /* Integer value PI2 */

/* Default parameters values */
#define ATR_DEFAULT_F         ((uint16_t)372)
#define ATR_DEFAULT_D         ((uint16_t)1  )
#define ATR_DEFAULT_I         ((uint16_t)50 )
#define ATR_DEFAULT_N         ((uint16_t)0  )
#define ATR_DEFAULT_P         ((uint16_t)5  )

/* Fi table */
extern const uint16_t atr_f_table[16];

/* Di table */
extern const uint16_t atr_d_table[16];

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint32_t length;
  uint8_t TS; /* Initial character */
  uint8_t T0; /* Format character */

  struct
  {
    uint8_t value;   /* The value of the Interface byte */
    uint8_t present; /* The presence of the Interface byte */
  }
  ib[ATR_MAX_PROTOCOLS][ATR_MAX_IB], TCK; /* Interface bytes, TCK */

  uint8_t pn;                            /* Protocol number: the value of i of the Interface bytes */
  uint8_t hb[ATR_MAX_HISTORICAL];        /* Historical bytes table */
  uint8_t hbn;                           /* Historical bytes number */
}
ATR_TypeDef;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
HAL_StatusTypeDef ATR_Read(SCProtocol_t *protocol, uint8_t *buffer, uint16_t *buffersize);
uint8_t ATR_Decode(ATR_TypeDef * atr, uint8_t *atr_buffer, uint32_t length);
void ATR_GetDefaultProtocol(ATR_TypeDef *atr, int8_t *protocol);
uint8_t ATR_GetParameter (ATR_TypeDef * atr, uint8_t name, uint16_t *parameter);

#endif /* _ATR_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
