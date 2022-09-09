/**
  ******************************************************************************
  * @file    Smartcard/src/t1_protocol_param.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    30-Oct-2015
  * @brief   This file provides some functions for T=1 protocol parameters
  *          configuration.
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

/** @addtogroup SmartCard
  * @{
  */

/** @defgroup T1_EX T1 protocol parameters processing
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "t1_protocol_param.h"
#include "checksum.h"
#include "pps.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint32_t SC_Configure_etu_baudrate(SMARTCARD_HandleTypeDef *hsc, uint32_t F, uint32_t D, uint32_t frequency);
/* Private functions --------------------------------------------------------*/

/**
  * @brief  Computes etu (elementary time unit) in us as well as the baudrate.
  * @param  hsc: HAL handle
  * @param  F: transmission factor F.
  * @param  D: transmission factor D.
  * @param  frequency: smartcard clock frequency.
  * @retval uint32_t ETU value in us.
  */
static uint32_t SC_Configure_etu_baudrate(SMARTCARD_HandleTypeDef *hsc, uint32_t F, uint32_t D, uint32_t frequency)
{
  uint32_t temp_etu;
  uint32_t temp_baudrate;

  temp_baudrate = (atr_d_table[D] * frequency) / atr_f_table[F];

  temp_etu = 1000000 / temp_baudrate;

  hsc->Init.BaudRate = temp_baudrate;
  hsc->Init.WordLength = SMARTCARD_WORDLENGTH_9B;
  hsc->Init.StopBits = SMARTCARD_STOPBITS_1_5;
  hsc->Init.Parity = SMARTCARD_PARITY_EVEN;
  hsc->Init.Mode = SMARTCARD_MODE_TX_RX | SMARTCARD_MODE_TX;
  HAL_SMARTCARD_Init(hsc);

  return temp_etu + 1;
}

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Set the smartcard baudrate function of F and D parameters and apply
  *         PPS procedure.
  * @param  p_t1: the HAL handle
  * @param  p_atr: a pointer to the ATR structure.
  * @param  SC_clk: the smartcard clock frequency in Hz.
  * @retval The etu value in us.
  */
uint32_t Set_F_D_parameters(SCProtocol_t * p_t1, ATR_TypeDef* p_atr, uint32_t SC_clk)
{
  uint8_t PPS_Response_length = 0, pps1 = 0;
  uint8_t PPS_buffer[4] = {0xFF, 0x10, 0x45, 0xAA};
  uint16_t F = ATR_DEFAULT_F;
  uint16_t D = ATR_DEFAULT_D;
  uint8_t TA2 = 0;

  /* ((Fd/Dd)/SC_clk) * 1000000 (in us) (+1 to get the ceiling of the value)*/
  uint32_t etu = (372000000 / SC_clk) + 1;

  /* TA2 is present ? */
  if (p_atr->ib[1][ATR_INTERFACE_BYTE_TA].present)
  {

    /* ---TA2 is present with value: TA2 (the SmartCard is in specific mode)--- */
    TA2 = p_atr->ib[1][ATR_INTERFACE_BYTE_TA].value;

    /* TA2 b5=0: Fi and Di should be applied after ATR */
    if ((TA2 & 0x10) == 0)
    {
      (void)ATR_GetParameter(p_atr, ATR_PARAMETER_D, &D);
      (void)ATR_GetParameter(p_atr, ATR_PARAMETER_F, &F);

      /* Configure the USART with the new baudrate */
      etu = SC_Configure_etu_baudrate(p_t1->pdevice, F, D, SC_clk);
    }
  }
  else
  {
    /* ---TA2 is absent: the SmartCard is in negotiable mode--- */

    /* TA1 is present ? (contains FI and DI)*/
    if (p_atr->ib[0][ATR_INTERFACE_BYTE_TA].present)
    {
      /* If the card supports values different from default values */
      if (p_atr->ib[0][ATR_INTERFACE_BYTE_TA].value != 0x11)
      {
        /* PPS1 */
        PPS_buffer[PPS1] = p_atr->ib[0][ATR_INTERFACE_BYTE_TA].value;

        /* Apply PPS (Protocol Parameters Selection) */
        (void)PPS_Exchange(p_t1, PPS_buffer, &PPS_Response_length, &pps1, etu);

        D = pps1 & 0xF;
        F = pps1 >> 4;

        /* Configure the usart with the new baudrate */
        etu = SC_Configure_etu_baudrate(p_t1->pdevice, F, D, SC_clk);

        /* ---TA1 is present with value: SC_atr->ib[0][ATR_INTERFACE_BYTE_TA].value--- */
        /* ---New SmartCard baudrate: baud--- */
      }
      else
      {
        /* ---TA1 is absent (use of default values F=372 and D=1)--- */
        /* ---SmartCard still having the same baudrate: baud--- */
      }
    }
  }

  return etu;
}


/**
  * @brief  Get the IFSC parameter from ATR and set it to p_t1 structure.
  * @param  p_t1: pointer to context structure of T=1 protocol.
  * @param  p_atr: a pointer to the ATR structure.
  * @retval None.
  */
void Set_IFSC(SCProtocol_t* p_t1, ATR_TypeDef * p_atr)
{
  __IO uint32_t i;
  uint8_t ifsc = 5;

  /* TAi (i>2) present? (see 9.5.2.1: IFS for the card) */
  for (i = 2; i < ATR_MAX_PROTOCOLS; i++)
  {
    if (p_atr->ib[i][ATR_INTERFACE_BYTE_TA].present)
    {
      ifsc = p_atr->ib[i][ATR_INTERFACE_BYTE_TA].value;

      /* only the first TAi (i>2) must be used */
      break;
    }
  }

  p_t1->ifsc = ifsc;
}

/**
  * @brief  Get the CWT and BWT parameters from ATR and set them to p_t1 structure.
  * @param  p_t1: pointer to context structure of T=1 protocol.
  * @param  p_atr: a pointer to the ATR structure.
  * @retval None.
  */
void Set_CWT_BWT( SCProtocol_t* p_t1, ATR_TypeDef* p_atr)
{
  __IO uint32_t i;
  uint8_t waiting_times = 0;
  uint8_t cwi = 13;
  uint8_t bwi = 4;

  /* TBi (i>2) present? (see 9.5.3: Waiting times (CWT and BWT)) */
  for (i = 2; i < ATR_MAX_PROTOCOLS; i++)
  {
    if (p_atr->ib[i][ATR_INTERFACE_BYTE_TB].present)
    {
      waiting_times = p_atr->ib[i][ATR_INTERFACE_BYTE_TB].value;
      cwi = waiting_times & 0xF;
      bwi = waiting_times >> 4;

      /* only the first TAi (i>2) must be used */
      break;
    }
  }
  /* Set BWI value */
  p_t1->bwi = bwi;
  /* Set CWI value */
  p_t1->cwi = cwi;
}

/**
  * @brief  Get the extra guard-time parameter from ATR and configure the USART
  *     guard-time.
  * @param  hsc: the HAL handle
  * @param  p_atr: a pointer to the ATR structure.
  * @retval None.
  */
void Set_EGT(SMARTCARD_HandleTypeDef *hsc, ATR_TypeDef* p_atr)
{
  uint16_t n;

  /* TC1 is present ? (contains the extra guard-time) */
  if (p_atr->ib[0][ATR_INTERFACE_BYTE_TC].present)
  {
    (void)ATR_GetParameter (p_atr, ATR_PARAMETER_N, &n);

    if (n > 1) /* There is 1.5 extra guard time added by the USART */
    {
      /* USART Guard Time set to 16 Bit */
      MODIFY_REG(hsc->Instance->GTPR, (((uint8_t)n - 1) << USART_GTPR_GT_Pos), USART_GTPR_GT);
    }
    else
    {
      /* USART Guard Time set to 16 Bit */
      MODIFY_REG(hsc->Instance->GTPR, 0, USART_GTPR_GT);
    }
    /* ---TC1 is present with value: N (extra guard-time is used)--- */
  }
  else
  {
    /* ---TC1 is absent (no extra guard-time is used)--- */
  }
}

/**
  * @brief  Get the type of checksum to be used from ATR and set it to p_t1 structure.
  * @param  p_t1: pointer to context structure of T=1 protocol.
  * @param  p_atr: a pointer to the ATR structure.
  * @retval None.
  */
void Set_EDC( SCProtocol_t *p_t1, ATR_TypeDef *p_atr)
{
  __IO uint32_t i;

  p_t1->rc_bytes = 1;
  p_t1->checksum = csum_lrc_compute;

  /* TCi (i>2) present? (see 9.5.4: Error detection code) */
  for (i = 2; i < ATR_MAX_PROTOCOLS; i++)
  {
    if (p_atr->ib[i][ATR_INTERFACE_BYTE_TC].present)
    {
      if (p_atr->ib[i][ATR_INTERFACE_BYTE_TC].value == 1)
      {
        p_t1->rc_bytes = 2;
        p_t1->checksum = csum_crc_compute;
      }
    }
  }
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
