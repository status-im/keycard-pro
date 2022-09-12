/**
  ******************************************************************************
  * @file    Smartcard/inc/smartcard.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    30-Oct-2015
  * @brief   This file contains all the functions prototypes for the Smartcard
  *          firmware library.
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
#ifndef __SMARTCARD_H
#define __SMARTCARD_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Exported constants --------------------------------------------------------*/
#define T0_PROTOCOL               ((uint8_t)0x00)  /* T0 protocol */
#define T1_PROTOCOL               ((uint8_t)0x01)  /* T1 protocol */
#define DIRECT                    ((uint8_t)0x3B)  /* Direct bit convention */
#define INDIRECT                  ((uint8_t)0x3F)  /* Indirect bit convention */
#define SETUP_LENGTH              ((uint8_t)20)
#define HIST_LENGTH               ((uint8_t)20)
#define MAX_PROTOCOLLEVEL         ((uint8_t)7)  /* Maximun levels of protocol */
#define MAX_INTERFACEBYTE         ((uint8_t)4)  /* Maximum number of interface bytes per protocol */
#define LC_MAX                    ((uint8_t)20)
#define SC_RECEIVE_TIMEOUT        ((uint32_t)0x8000)  /* Direction to reader */

/* T=1 protocol constants */
#define T1_I_BLOCK           ((uint8_t)0x00)  /* PCB (I-block: b8 = 0)  */
#define T1_R_BLOCK           ((uint8_t)0x80)  /* PCB (R-block: b8 b7 = 10) */
#define T1_S_BLOCK           ((uint8_t)0xC0)  /* PCB (S-block: b8 b7 = 11) */

/* I block */
#define T1_I_SEQ_SHIFT     6    /* N(S) position (bit 7) */

/* R block */
#define T1_IS_ERROR(pcb)   ((pcb) & 0x0F)
#define T1_EDC_ERROR       ((uint8_t)0x01) /* [b6..b1] = 0-N(R)-0001 */
#define T1_OTHER_ERROR     ((uint8_t)0x02) /* [b6..b1] = 0-N(R)-0010 */
#define T1_R_SEQ_SHIFT     ((uint8_t)4)    /* N(R) position (b5) */

/* S block  */
#define T1_S_RESPONSE     ((uint8_t)0x20)   /* If response: set bit b6, if request reset b6 in PCB S-Block */
#define T1_S_RESYNC       ((uint8_t)0x00)   /* RESYNCH: b6->b1: 000000 of PCB S-Block */
#define T1_S_IFS          ((uint8_t)0x01)   /* IFS: b6->b1: 000001 of PCB S-Block */
#define T1_S_ABORT        ((uint8_t)0x02)   /* ABORT: b6->b1: 000010 of PCB S-Block */
#define T1_S_WTX          ((uint8_t)0x03)   /* WTX: b6->b1: 000011 of PCB S-Block */
#define T1_S_MASK         ((uint8_t)0x0F)   /* S block filtering mask */

/* Private macro -------------------------------------------------------------*/
#define NAD    (((DAD&0x7)<<4) | (SAD&0x7)) /* Node Address byte */
#define PCB                ((uint8_t)1)  /* PCB byte positon in the block */
#define LEN                ((uint8_t)2)  /* LEN byte positon in the block */
#define DATA               ((uint8_t)3)  /* The positon of the first byte of INF field in the block */

/* Modifiable parameters */
#define SAD           ((uint8_t)0x0)     /* Source address: reader (allowed values 0 -> 7) */
#define DAD           ((uint8_t)0x0)     /* Destination address: card (allowed values 0 -> 7) */
#define IFSD_VALUE    ((uint8_t)254)     /* Max length of INF field Supported by the reader */
#define SC_FILE_ID    ((uint16_t)0x0001)  /* File identifier */
#define SC_CLASS      ((uint8_t)0x00)

/* Constant parameters */
#define INS_SELECT_FILE    ((uint8_t)0xA4) /* Select file instruction */
#define INS_READ_FILE      ((uint8_t)0xB0) /* Read file instruction */
#define INS_WRITE_FILE     ((uint8_t)0xD6) /* Write file instruction */
#define TRAILER_LENGTH     ((uint8_t)2   ) /* Trailer lenght (SW1 and SW2: 2 bytes) */

#define SC_T1_BWT_TIMEOUT    ((uint32_t)1)
#define SC_CWT_TIMEOUT       ((uint32_t)25)   /* = ~10ms @4MHz + margin */

/* SC Tree Structure -----------------------------------------------------------
                              MasterFile
                           ________|___________
                          |        |           |
                        System   UserData     Note
------------------------------------------------------------------------------*/

/* SC ADPU Command: Operation Code -------------------------------------------*/
#define SC_CLA_GSM11              ((uint8_t)0xA0)

/*------------------------ Data Area Management Commands ---------------------*/
#define SC_SELECT_FILE            ((uint8_t)0xA4)
#define SC_GET_RESPONCE           ((uint8_t)0xC0)
#define SC_STATUS                 ((uint8_t)0xF2)
#define SC_UPDATE_BINARY          ((uint8_t)0xD6)
#define SC_READ_BINARY            ((uint8_t)0xB0)
#define SC_WRITE_BINARY           ((uint8_t)0xD0)
#define SC_UPDATE_RECORD          ((uint8_t)0xDC)
#define SC_READ_RECORD            ((uint8_t)0xB2)
#define SC_GET_CHALLENGE          ((uint8_t)0xB4)

/*-------------------------- Administrative Commands -------------------------*/
#define SC_CREATE_FILE            ((uint8_t)0xE0)

/*-------------------------- Safety Management Commands ----------------------*/
#define SC_VERIFY                 ((uint8_t)0x20)
#define SC_CHANGE                 ((uint8_t)0x24)
#define SC_DISABLE                ((uint8_t)0x26)
#define SC_ENABLE                 ((uint8_t)0x28)
#define SC_UNBLOCK                ((uint8_t)0x2C)
#define SC_EXTERNAL_AUTH          ((uint8_t)0x82)

/*-------------------------- Smartcard Interface Byte-------------------------*/
#define SC_INTERFACEBYTE_TA   ((uint8_t)0) /* Interface byte TA(i) */
#define SC_INTERFACEBYTE_TB   ((uint8_t)1) /* Interface byte TB(i) */
#define SC_INTERFACEBYTE_TC   ((uint8_t)2) /* Interface byte TC(i) */
#define SC_INTERFACEBYTE_TD   ((uint8_t)3) /* Interface byte TD(i) */

/*-------------------------- Answer to reset Commands ------------------------*/
#define SC_GET_A2R                ((uint8_t)0x00)

/* SC STATUS: Status Code ----------------------------------------------------*/
#define SC_EF_SELECTED            ((uint8_t)0x9F)
#define SC_DF_SELECTED            ((uint8_t)0x9F)
#define SC_OP_TERMINATED         ((uint16_t)0x9000)

/* Smartcard Voltage */
#define SC_VOLTAGE_5V              GPIO_PIN_RESET
#define SC_VOLTAGE_3V              GPIO_PIN_SET

/* Exported constants --------------------------------------------------------*/
/* T=1 protocol constants */
#define T1_I_BLOCK        ((uint8_t)0x00)  /* PCB (I-block: b8 = 0)  */
#define T1_R_BLOCK        ((uint8_t)0x80)  /* PCB (R-block: b8 b7 = 10) */
#define T1_S_BLOCK        ((uint8_t)0xC0)  /* PCB (S-block: b8 b7 = 11) */
#define T1_MORE_BLOCKS    ((uint8_t)0x20)  /* PCB (b6): More data bit */
#define T1_BUFFER_SIZE    ((uint32_t)(4 + 255 + 2)) /* Prologue + Information field + EDC */

/* Exported types ------------------------------------------------------------*/

typedef enum
{
  SC_POWER_ON = 0x00,
  SC_RESET_LOW,
  SC_RESET_HIGH,
  SC_ACTIVE,
  SC_ACTIVE_ON_T0,
  SC_ACTIVE_ON_T1,
  SC_POWER_OFF,
  T1_RESYNCH,
  T1_SENDING,
  T1_RECEIVING
} SC_State_t;

/* Smartcard communication protocol context ----------------------------------*/
typedef struct
{
  SMARTCARD_HandleTypeDef *pdevice; /* The HAL used in transmission */
  uint32_t   frequency; /* Smart card frequency */
  uint32_t   cwi;       /* Character waiting time */
  uint32_t   bwi;       /* Block waiting time */
  uint32_t   wtx;       /* Waiting time extension value (extension of BWT: Block Waiting Time) */
  SC_State_t state;     /* State of the communication */
  uint8_t    ns;       /* reader side: N(S): the send-sequence number of the block  */
  uint8_t    nr;       /* card side: N(R): the number of the expected l-block */
  uint8_t    ifsc;      /* Maximum length of Information field which can be received by the card */
  uint8_t    ifsd;      /* Maximum length of Information field which can be received by the interface device */
  uint8_t    convention;   /* Convention of the communication */
  uint8_t    retries;      /* Number of retries */
  uint8_t    rc_bytes;     /* Length of EDC */
  uint8_t    (* checksum)(uint8_t * data, uint32_t len, uint8_t *rc); /* Pointer to the function that computes EDC */
  uint8_t    more;  /* More data bit */
  uint8_t    previous_block[5]; /* To store the last R-block */
}
SCProtocol_t;

/* ADPU-Header command structure ---------------------------------------------*/
typedef struct
{
  uint8_t CLA;  /* Command class */
  uint8_t INS;  /* Operation code */
  uint8_t P1;   /* Selection Mode */
  uint8_t P2;   /* Selection Option */
}
SC_Header_t;

/* ADPU-Body command structure -----------------------------------------------*/
typedef struct
{
  uint8_t LC;           /* Data field length (P3 in T=0) */
  uint8_t Data[LC_MAX]; /* Command parameters */
  uint8_t LE;           /* Expected length of data to be returned */
}
SC_Body_t;

/* ADPU Command structure ----------------------------------------------------*/
typedef struct
{
  SC_Header_t Header;
  SC_Body_t Body;
}
SC_APDU_Cmd_t;

/* SC response structure -----------------------------------------------------*/
typedef struct
{
  uint8_t Data[LC_MAX];  /* Data returned from the card */
  uint8_t SW1;          /* Command Processing status */
  uint8_t SW2;          /* Command Processing qualification */
}
SC_APDU_Resp_t;

/* SC APDU structure unification ---------------------------------------------*/
typedef union
{
  SC_APDU_Cmd_t APDU_S;
  SC_APDU_Resp_t APDU_R;
  uint8_t buffer[T1_BUFFER_SIZE];
} SC_APDU_t;

/* Exported macro ------------------------------------------------------------*/

/**
  * @brief  Enables or disables the power to the Smartcard.
  * @param  __NEWSTATE__: new state of the Smartcard power supply.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
*/
#define SC_POWER(__NEWSTATE__)  HAL_GPIO_WritePin(SC_NCMDVCC_GPIO_Port, SC_NCMDVCC_Pin, (GPIO_PinState) __NEWSTATE__)

/**
  * @brief  Enables or disables the power to the Smartcard.
  * @param  __NEWSTATE__: new state of the Smartcard power supply.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
*/
#define SC_3_5V(__NEWSTATE__)  HAL_GPIO_WritePin(SC_5V3V_GPIO_Port, SC_5V3V_Pin, (GPIO_PinState) __NEWSTATE__)

/**
  * @brief  Sets or clears the Smartcard reset pin.
  * @param  __NEWSTATE__: this parameter specifies the state of the Smartcard
  *         reset pin. Must be one of the FunctionalState enum values:
  *                 @arg DISABLE: to clear the port pin.
  *                 @arg ENABLE: to set the port pin.
  * @retval None
*/
#define SC_RESET(__NEWSTATE__)  HAL_GPIO_WritePin(SC_RST_GPIO_Port, SC_RST_Pin, (GPIO_PinState) __NEWSTATE__)

/* Exported functions ------------------------------------------------------- */

void T1_Protocol_Init(SCProtocol_t *p_t1, uint32_t sc_freq);
int32_t T1_Negotiate_IFSD(SCProtocol_t *p_t1, uint8_t nad, uint8_t ifsd);
int32_t T1_APDU(SCProtocol_t *p_t1, uint8_t nad, SC_APDU_t *p_apdu_c, SC_APDU_t *p_apdu_r);
void T0_APDU_Prep( SC_APDU_t *p_apdu_c, uint8_t *p_data);
HAL_StatusTypeDef T0_APDU(SCProtocol_t *p_t0, SC_APDU_t *p_apdu_c, SC_APDU_t *p_apdu_r);
#endif /* __SMARTCARD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
