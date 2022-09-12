/**
  ******************************************************************************
  * @file    Smartcard/inc/buffer.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    30-Oct-2015
  * @brief   This file contains all the functions prototypes for buffer library.
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
#ifndef __BUFFER_H
#define __BUFFER_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct ct_buf
{
  uint8_t * base;
  uint32_t head, tail, size;
  uint32_t overrun;
}
buffer_TypeDef;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Buffer_init(buffer_TypeDef *bp, void *buffer, uint32_t len);
void Buffer_set(buffer_TypeDef *bp, void *buffer, uint32_t len);
int32_t  Buffer_get(buffer_TypeDef *bp, void *buffer, uint32_t len);
int32_t  Buffer_put(buffer_TypeDef *bp, const void *buffer, uint32_t len);
int32_t  Buffer_putc(buffer_TypeDef *bp, uint8_t byte);
uint32_t Buffer_avail(buffer_TypeDef *bp);
void * Buffer_head(buffer_TypeDef *bp);

#endif /* __BUFFER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
