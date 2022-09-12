/**
  ******************************************************************************
  * @file    SmartCard/src/smartcard.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    30-Oct-2015
  * @brief   This file provides all the Smartcard firmware functions.
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

/** @defgroup APDU APDU exchange protocols implementation
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "smartcard.h"
#include "atr.h"
#include "checksum.h"
#include "buffer.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define T1_OK                ((int32_t)0)  /* APDU transfer OK */
#define T1_ERR_TIMEOUT       ((int32_t)-1) /* Timeout in communication */
#define T1_NOT_COMPLETE      ((int32_t)1)  /* Communication not complete */
#define T1_DO_RESYNC         ((int32_t)2)  /* Minor problem encounter */
#define T1_ERR_PARITY        ((int32_t)-2) /* Checksum or parity wrong */
#define T1_ERR_STRUCT        ((int32_t)-3) /* Block structure not recognized */

/* Private macro -------------------------------------------------------------*/
#define T1_S_IS_RESPONSE(pcb) (((pcb) & T1_S_RESPONSE) != 0 )
#define T1_S_TYPE(pcb)        ((pcb) & T1_S_MASK)
#define SWAP_NIBBLES(x)       (((x) >> 4) | (((x) & 0xF) << 4))
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
int32_t T1_TxRxBlock(SCProtocol_t *p_t1, uint8_t *p_buffer, uint32_t tx_length);
static uint8_t T1_GetBlockType(uint8_t pcb);
static uint8_t T1_GetBlockSequence(uint8_t pcb);
static uint32_t T1_RebuildRBlock(SCProtocol_t *p_t1, uint8_t *pblock);
static uint32_t T1_ComputeChecksum(SCProtocol_t * p_t1, uint8_t *pdata, uint32_t len);
static HAL_StatusTypeDef T1_VerifyChecksum(SCProtocol_t * p_t1, uint8_t *prbuf, uint32_t len);
static uint32_t T1_BuildBlock(SCProtocol_t * p_t1, uint8_t *p_tx_block, uint8_t nad, uint8_t pcb, buffer_TypeDef *p_tx_buff, uint32_t *p_tx_inf_len);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Transceive T=1 blocks.
  * @param  p_t1: a pointer to context structure of T=1 protocol.
  * @param  p_buffer: a pointer to Transmit/Receive buffer.
  * @param  tx_length: the length of the block to be transmitted.
  * @retval int32_t the reception status of the received block. The returned value can
  *   be one of the following:
  *     - (>0): the length of the received block if the reception has succeeded.
  *     - T1_ERR_TIMEOUT: in case of BWT time-out.
  *     - T1_ERR_PARITY: in case of parity error.
  *     - T1_ERR_STRUCT: block malformed.
  */
int32_t T1_TxRxBlock(SCProtocol_t * p_t1, uint8_t *p_buffer, uint32_t tx_length)
{
  HAL_StatusTypeDef  rx_status;
  uint32_t rx_len = 3;
  int32_t retvalue;
  SMARTCARD_HandleTypeDef *hsc = p_t1->pdevice;

  /* Send a block to the card */
  HAL_SMARTCARD_Transmit( hsc, p_buffer, tx_length, SC_CWT_TIMEOUT );

  /* Cleaning the receiver */
  //__HAL_SMARTCARD_CLEAR_IT(hsc, SMARTCARD_CLEAR_OREF);
  retvalue = __HAL_SMARTCARD_GET_DRREGISTER(hsc);
  __HAL_SMARTCARD_CLEAR_OREFLAG(hsc);

  /* Get the block response from the card */
  rx_status = HAL_SMARTCARD_Receive( hsc, p_buffer, rx_len, SC_CWT_TIMEOUT);

  if (rx_status == HAL_OK)
  {
    /* 3: the length of Prologue field */
    rx_len = p_buffer[2] + p_t1->rc_bytes;

    /* Get the block response from the card */
    rx_status = HAL_SMARTCARD_Receive( hsc, &p_buffer[3], rx_len, SC_CWT_TIMEOUT);

    if (rx_status == HAL_OK)
    {
      retvalue = rx_len + 3;
    }
  }

  if (rx_status == HAL_ERROR)
  {
    /* Receive Error: parity error */
    retvalue = T1_ERR_PARITY;
  }
  else if (rx_status == HAL_TIMEOUT)
  {
    /* Receive Error: timeout error */
    retvalue = T1_ERR_TIMEOUT;
  }

  return retvalue;
}

/**
  * @brief  Initialize T=1 protocol.
  * @param  p_t1: pointer to context structure of T=1 protocol.
  * @param  sc_freq: smartcard clock frequency.
  * @retval None.
  */
void T1_Protocol_Init(SCProtocol_t * p_t1, uint32_t sc_freq)
{
  /* Set default T=1 protocol parameters */
  p_t1->retries = 3;
  p_t1->ifsc = 32; /* Maximum length of Information field which can be received by the card (by default set to 32)*/
  p_t1->ifsd = 32; /* Maximum length of Information field which can be received by the interface (by default set to 32) */
  p_t1->nr = 0;    /* N(R): the number of the expected l-block */
  p_t1->ns = 0;    /* N(S): the send-sequence number of the block */
  p_t1->wtx = 1;   /* WTX: waiting time extension */

  /* Set parameter: LRC is selected for EDC */
  p_t1->rc_bytes = 1;
  p_t1->checksum = csum_lrc_compute;

  /* Set parameter: Sending state: prepare for sending */
  p_t1->state = T1_SENDING;

  /* Reset "more" parameter */
  p_t1->more = 0;

  /* Set the smartcard frequency to be used by the protocol */
  p_t1->frequency = sc_freq;
}

/**
  * @brief  Manages the T0 transport layer: send APDU commands and receives
  *   the APDU response.
  * @param  p_apdu_c: pointer to a SC_APDU_t with command prepared.
  * @param  p_data: pointer to a SC_APDU_t with command prepared.
  * @retval HAL_StatusTypeDef: HAL status
  */
void T0_APDU_Prep( SC_APDU_t *p_apdu_c, uint8_t *p_data)
{
  uint32_t index;
  
  p_apdu_c->APDU_S.Header.CLA = SC_CLA_GSM11;
  
  for (index = 0; index < p_apdu_c->APDU_S.Body.LC; index++)
  {
    p_apdu_c->APDU_S.Body.Data[index] = p_data[index];
  }
  while (index < LC_MAX)
  {
    p_apdu_c->APDU_S.Body.Data[index++] = 0;
  }
  p_apdu_c->APDU_S.Body.LE = 0;
}

/**
  * @brief  Manages the T0 transport layer: send APDU commands and receives
  *   the APDU response.
  * @param  p_t0: pointer to context structure of smartcard protocol.
  * @param  p_apdu_c: pointer to a SC_APDU_t with command prepared.
  * @param  p_apdu_r: pointer to a SC_APDU_t structure which will be initialized.
  * @retval HAL_StatusTypeDef: HAL status
  */
HAL_StatusTypeDef T0_APDU(SCProtocol_t *p_t0, SC_APDU_t *p_apdu_c, SC_APDU_t *p_apdu_r)
{
  uint32_t i = 0;
  uint8_t locData = 0;
  uint8_t sc_data;
  SMARTCARD_HandleTypeDef *hsc = p_t0->pdevice;
  HAL_StatusTypeDef retval;

  /* Reset response buffer ---------------------------------------------------*/
  for (i = 0; i < LC_MAX; i++)
  {
    p_apdu_r->APDU_R.Data[i] = 0;
  }

  p_apdu_r->APDU_R.SW1 = 0;
  p_apdu_r->APDU_R.SW2 = 0;

  /* clear UART data + reset delay */
  HAL_SMARTCARD_Receive(hsc, &sc_data, 2, SC_T1_BWT_TIMEOUT);
  /* clear UART errors */
  __HAL_SMARTCARD_CLEAR_OREFLAG(hsc);

  /* Send header -------------------------------------------------------------*/
  retval = HAL_SMARTCARD_Transmit(hsc, p_apdu_c->buffer, 4, SC_CWT_TIMEOUT);
  if ( retval == HAL_OK)
  {
    /* Send body length to/from SC ---------------------------------------------*/
    if (p_apdu_c->APDU_S.Body.LC)
    {
      sc_data = p_apdu_c->APDU_S.Body.LC;
    }
    else if (p_apdu_c->APDU_S.Body.LE)
    {
      sc_data = p_apdu_c->APDU_S.Body.LE;
    }

    retval = HAL_SMARTCARD_Transmit(hsc, &sc_data, 1, SC_CWT_TIMEOUT);
    if ( retval == HAL_OK)
    {
      /* Flush the SC_USART DR */
      __HAL_SMARTCARD_CLEAR_OREFLAG(hsc);
      sc_data = __HAL_SMARTCARD_GET_DRREGISTER(hsc);

      /* --------------------------------------------------------
        Wait Procedure byte from card:
        1 - ACK
        2 - NULL
        3 - SW1; SW2
      -------------------------------------------------------- */
      if ((HAL_SMARTCARD_Receive(hsc, &locData, 1, SC_RECEIVE_TIMEOUT)) == HAL_OK)
      {
        if (((locData & (uint8_t)0xF0) == 0x60) || ((locData & (uint8_t)0xF0) == 0x90))
        {
          /* SW1 received */
          p_apdu_r->APDU_R.SW1 = locData;

          if ((HAL_SMARTCARD_Receive(hsc, &locData, 1, SC_RECEIVE_TIMEOUT)) == HAL_OK)
          {
            p_apdu_r->APDU_R.SW2 = locData;
          }
        }
        else if (((locData & (uint8_t)0xFE) == (((uint8_t)~(p_apdu_c->APDU_S.Header.INS)) & (uint8_t)0xFE)) || \
                 ((locData & (uint8_t)0xFE) == (p_apdu_c->APDU_S.Header.INS & (uint8_t)0xFE)))
        {
          p_apdu_r->APDU_R.Data[0] = locData;/* ACK received */
        }
      }

      /* If no status bytes received ---------------------------------------------*/
      if (p_apdu_r->APDU_R.SW1 == 0x00)
      {
        /* Send body data to SC--------------------------------------------------*/
        if (p_apdu_c->APDU_S.Body.LC)
        {
          retval = HAL_SMARTCARD_Transmit(hsc, p_apdu_c->APDU_S.Body.Data, p_apdu_c->APDU_S.Body.LC, SC_T1_BWT_TIMEOUT);
          
          /* Flush the SC_USART RDR */
          sc_data = __HAL_SMARTCARD_GET_DRREGISTER(hsc);
          __HAL_SMARTCARD_CLEAR_OREFLAG(hsc);
        }

        /* Or receive body data from SC ------------------------------------------*/
        else if (p_apdu_c->APDU_S.Body.LE)
        {
          HAL_SMARTCARD_Receive(hsc, p_apdu_r->APDU_R.Data, p_apdu_c->APDU_S.Body.LE, SC_RECEIVE_TIMEOUT);
        }

        /* Wait SW1 --------------------------------------------------------------*/
        i = 0;
        while (i < 10)
        {
          if ((HAL_SMARTCARD_Receive(hsc, &locData, 1, SC_RECEIVE_TIMEOUT)) == HAL_OK)
          {
            p_apdu_r->APDU_R.SW1 = locData;
            i = 11;
          }
          else
          {
            i++;
          }
        }
        /* Wait SW2 ------------------------------------------------------------*/
        i = 0;
        while (i < 10)
        {
          if ((HAL_SMARTCARD_Receive(hsc, &locData, 1, SC_RECEIVE_TIMEOUT)) == HAL_OK)
          {
            p_apdu_r->APDU_R.SW2 = locData;
            i = 11;
          }
          else
          {
            i++;
          }
        }
      }
    }
  }
  return retval;
}

/**
  * @brief  Send APDU command and receive APDU response through T=1 protocol.
  * @param  p_t1: pointer to context structure of smartcard protocol.
  * @param  nad: nad value (Node Address).
  * @param  p_apdu_c: pointer to ADPU command buffer.
  * @param  p_apdu_r: pointer to ADPU command buffer.
  * @retval int32_t the status of the APDU transaction. The returned value can
  *   be one of the following:
  *     - if >0 : the length of the APDU response when the APDU transaction
  *      has succeeded.
  *     - (<0): in case of communication error.
  */
int32_t T1_APDU(SCProtocol_t * p_t1, uint8_t nad, SC_APDU_t *p_apdu_c, SC_APDU_t *p_apdu_r)
{
  buffer_TypeDef sbuf, rbuf, tbuf;
  uint8_t sdata[T1_BUFFER_SIZE], sblk[5];
  uint32_t apdu_c_len, slen, retries, resyncs, sent_length = 0;
  size_t last_send = 0;
  uint8_t pcb;
  int32_t n, status = T1_NOT_COMPLETE;

  apdu_c_len = p_apdu_c->APDU_S.Body.LC + 5;

  p_t1->state = T1_SENDING;
  retries = p_t1->retries;
  resyncs = 3;

  /* Initialize the send buffer */
  Buffer_set(&sbuf, (void *)p_apdu_c->buffer, apdu_c_len);

  /* Initialize the receive buffer */
  Buffer_init(&rbuf, p_apdu_r, T1_BUFFER_SIZE);

  /* ---Sending I-Block--- */

  /* Build the first I-block (sdata) and return the length of the buffer to send */
  slen = T1_BuildBlock(p_t1, sdata, nad, T1_I_BLOCK, &sbuf, (uint32_t *) & last_send);

  while ( status > T1_OK )
  {
    retries--;

    /* Send the block (sdata) and get the response in the same buffer */
    n = T1_TxRxBlock(p_t1, sdata, slen);

    if (n < 0) /* Receive Parity error or BWT time-out or block malformed */
    {
      /* ---Response Error Occurred--- */
      /* ISO 7816-3 Rule 7.4.2 */
      if (retries == 0)
      {
        /* Resynchronization required */
        status = T1_DO_RESYNC;
      }

      /* ISO 7816-3 Rule 7.2: retransmit R-Block */
      else if (T1_R_BLOCK == T1_GetBlockType(p_t1->previous_block[PCB]))
      {
        slen = T1_RebuildRBlock(p_t1, sdata);
      }

      else if (n == T1_ERR_PARITY) /* Parity Error */
      {
        /* ISO 7816-3 Rule 7.1  */
        slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_EDC_ERROR, NULL, NULL);
      }
      else if ((n == T1_ERR_TIMEOUT) || (n == T1_ERR_STRUCT)) /* BWT time-out or block malformed */
      {
        /* ---BWT time-out or block malformed, Sending R-Block--- */
        /* ISO 7816-3 Rule 7.1 */
        slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
      }
    }
    /* Wrong NAD or illegal LEN == 0xFF  */
    else if ((sdata[NAD] != SWAP_NIBBLES(nad)) || (sdata[LEN] == 0xFF))
    {
      /* ---Wrong NAD or Block Length...--- */
      /* ISO 7816-3 Rule 7.4.2 */
      if (retries == 0)
      {
        /* ---Resynchronization required--- */
        status = T1_DO_RESYNC;
      }

      /* ISO 7816-3 Rule 7.2: If the previous sent block was R-Block, then retransmit it */
      else if (T1_R_BLOCK == T1_GetBlockType(p_t1->previous_block[PCB]))
      {
        slen = T1_RebuildRBlock(p_t1, sdata);
      }
      else
      {
        /* Transmit R-Block signalling Other error */
        slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
      }
    }
    /* Wrong CRC */
    else if (T1_VerifyChecksum(p_t1, sdata, n) != HAL_OK)
    {
      /* ---Checksum failed--- */
      /* ISO 7816-3 Rule 7.4.2 */
      if (retries == 0)
      {
        status = T1_DO_RESYNC;
      }

      /* ISO 7816-3 Rule 7.2: If the previous sent block was R-Block, then retransmit it  */
      else if (T1_R_BLOCK == T1_GetBlockType(p_t1->previous_block[PCB]))
      {
        slen = T1_RebuildRBlock(p_t1, sdata);
      }
      else
      {

        /* If the received block is not R-Block, then transmit a R-Block signalling EDC error  */
        slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_EDC_ERROR, NULL, NULL);
      }
    }
    else
    {
      /* Get the pcb value of the received block */
      pcb = sdata[PCB];

      /* Determine the type of the received block */
      switch (T1_GetBlockType(pcb))
      {
        case T1_R_BLOCK:
          /* ---R-Block--- */
          /* length != 0x00 (illegal) or b6 of pcb is set*/
          if ((sdata[LEN] != 0x00) || (T1_S_IS_RESPONSE(pcb)))
          {
            /* ---Wrong R-Block received--- */
            /* ISO 7816-3 Rule 7.4.2 */
            if (retries == 0)
            {
              status = T1_DO_RESYNC;
            }

            /* ISO 7816-3 Rule 7.2 (if the previous block send was R-Block then retransmit it  */
            else if (T1_R_BLOCK == T1_GetBlockType(p_t1->previous_block[PCB]))
            {
              slen = T1_RebuildRBlock(p_t1, sdata);
            }
            else
            {
              /* If the previous block sent wasn't R-Block then transmit R-Block signalling "Other Error"  */
              slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
            }
          }
          /* Wrong sequence number: we have received N(R) != N(S) when we are not in chaining process (more = 0).
          Normally when we sent an I-Block and an R-Bloks is received and we are not in chaining mode, N(R) should
          be equal to N(S) (refer to rule 7.1) */
          else if (((T1_GetBlockSequence(pcb) != p_t1->ns) && (! p_t1->more)))
          {
            /* ---Wrong sequence number received: Sequence number received: T1_GetBlockSequence(pcb), expected: p_t1->ns, more: p_t1->more--- */
            /* ISO 7816-3 Rule 7.2: if the previous block send was R-Block then retransmit it  */
            if (T1_R_BLOCK == T1_GetBlockType(p_t1->previous_block[PCB]))
            {
              /* ---Rule 7.2: retransmit R-Block--- */
              slen = T1_RebuildRBlock(p_t1, sdata);
            }

            /* ISO 7816-3 Rule 7.4.2 */
            else if (retries == 0)
            {
              status = T1_DO_RESYNC;
            }
            else
            {
              /* ---Transmit R-Block signalling Other error--- */
              /* If the previous block sent wasn't R-Block then transmit R-Block signalling "Other Error"  */
              slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
            }
          }

          else if (p_t1->state == T1_RECEIVING)
          {
            /* ISO 7816-3 Rule 7.2: if the previous block send was R-Block then retransmit it  */
            if (T1_R_BLOCK == T1_GetBlockType(p_t1->previous_block[PCB]))
            {
              /* ---Rule 7.2: retransmit R-Block--- */
              slen = T1_RebuildRBlock(p_t1, sdata);
            }
            else
            {
              /* ---Sending R-Block--- */
              slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK, NULL, NULL);
            }
          }

          /* Rule 2.2 (case of chaining): If the card requests the next
          * I-block sequence number, this means that it has received
          * the previous block successfully */
          else
          {
            if (T1_GetBlockSequence(pcb) != p_t1->ns)
            {
              Buffer_get(&sbuf, NULL, last_send);
              sent_length += last_send; /* store the data length cumulation */
              last_send = 0;
              p_t1->ns ^= 1; /* Inverse the sequence number */
            }

            /* If there's no data available, the ICC
            * shouldn't be asking for more */
            if (Buffer_avail(&sbuf) == 0)
            {
              status = T1_DO_RESYNC;
            }
            else
            {
              /* Send an I-Block */
              slen = T1_BuildBlock(p_t1, sdata, nad, T1_I_BLOCK, &sbuf, (uint32_t *) & last_send);
            }
          }
          break;

        case T1_I_BLOCK:

          /* The first I-block sent by the ICC indicates
          * the last block we sent was received successfully */
          if (p_t1->state == T1_SENDING)
          {
            Buffer_get(&sbuf, NULL, last_send);
            last_send = 0;
            p_t1->ns ^= 1;
          }

          /* Switch to receive mode */
          p_t1->state = T1_RECEIVING;

          /* If the block sent by the card doesn't match
          * what we expected it to send, reply with
          * an R-block */
          if (T1_GetBlockSequence(pcb) != p_t1->nr)
          {
            /* ---wrong N(R), Transmit R-Block signalling Other error--- */
            slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
          }
          else
          {
            /* Inverse N(R) value */
            p_t1->nr ^= 1;

            if (Buffer_put(&rbuf, sdata + 3, sdata[LEN]) < 0)
            {
              /* ---Buffer overrun by (sdata[LEN] - (rbuf.size - rbuf.tail)) bytes--- */
              status = T1_ERR_STRUCT;
            }
            /* If more bit (b6 of I-Block) is equal to 0 the APDU is completed  */
            else if ((pcb & T1_MORE_BLOCKS) == 0)
            {
              status = T1_OK;
            }
            else
            {
              /* Send R-Block for chaining  */
              slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK, NULL, NULL);
            }
          }
          break;

        case T1_S_BLOCK:

          /* If the S-Block received is a response while the reader is
          in resynchronization state after it has sent a synchronization request */
          if (T1_S_IS_RESPONSE(pcb) && (p_t1->state == T1_RESYNCH) && (T1_S_TYPE(pcb) == T1_S_RESYNC))
          {
            /* ---S-Block answer received for resynchronization--- */
            /* ISO 7816-3 Rule 6.3 */
            p_t1->state = T1_SENDING;
            sent_length = 0;
            last_send = 0;
            resyncs = 3;
            retries = p_t1->retries;
            Buffer_init(&rbuf, p_apdu_r, T1_BUFFER_SIZE);

            /* Send I-Block  */
            slen = T1_BuildBlock(p_t1, sdata, nad, T1_I_BLOCK, &sbuf, (uint32_t *) & last_send);
          }

          /* There are 5 types of S-Block responses: RESYNCH, IFS, ABORT, WTX, VPP state error:
          - RESYNCH: is treated in the condition above.
          - IFS response is not allowed here because it's managed in T1_Negotiate_IFSD function, so the card is not allowed to send IFS response.
          - ABORT response is not allowed since we don't send in any case an ABORT request.
          - WTX response is allowed only from interface device side, so the card is not allowed to send this type of response.
          - VPP state error: we suppose that it was an error, we send en R-block
          --> only RESYNCH response is allowed. All other responses are treated as errors */
          else if (T1_S_IS_RESPONSE(pcb))
          {
            /* ISO 7816-3 Rule 7.4.2 */
            if (retries == 0)
            {
              status = T1_DO_RESYNC;
            }

            /* ISO 7816-3 Rule 7.2 */
            else if (T1_R_BLOCK == T1_GetBlockType(p_t1->previous_block[PCB]))
            {
              slen = T1_RebuildRBlock(p_t1, sdata);
            }
            else
            {
              /* Transmit R-Block signalling "Other Error"  */
              slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
            }
          }
          else
          {

            Buffer_init(&tbuf, sblk, sizeof(sblk));

            /* ---S-Block request received--- */

            switch (T1_S_TYPE(pcb))
            {
              case T1_S_RESYNC:  /* The SmartCard sent resynchronization request */

                /* If length different from 0 (see ISO7816 - 9.4.3) */
                if (sdata[LEN] != 0)
                {
                  /* ---Wrong length: sdata[LEN]--- */
                  /* ---Sending R-Block signalling other error--- */
                  slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
                }
                else
                {
                  status = T1_DO_RESYNC;
                }
                break;

              case T1_S_ABORT:  /* The SmartCard sent a data abortion request */
                /* If length different from 0 (see ISO7816 - 9.4.3) */
                if (sdata[LEN] != 0)
                {
                  /* ---Wrong length: sdata[LEN]--- */
                  /* ---Sending R-Block signalling other error--- */

                  /* See Annex A: A.3.3 scenario 16 */
                  slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
                }

                /* ISO 7816-3 Rule 9 */
                /* ---Abort requested, sending S-Abort response--- */
                break;

              case T1_S_IFS:  /* The SmartCard sent an IFS request */
                if (sdata[LEN] != 1)
                {
                  /* ---Wrong length: sdata[LEN]--- */
                  /* ---Sending R-Block signalling other error--- */

                  /* See Annex A: A.3.3 scenario 16 */
                  slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
                }
                /* ---The card sent an S(IFS_request) with ifs = sdata[DATA]--- */
                else if ((sdata[DATA] == 0) || (sdata[DATA] == 0xFF)) /* IFSC should be: 0x01 to 0xFE */
                {
                  status = T1_DO_RESYNC;
                }
                else
                {
                  /* Store the new IFSC value  */
                  p_t1->ifsc = sdata[DATA];
                  Buffer_putc(&tbuf, sdata[DATA]);
                }
                break;

              case T1_S_WTX: /* The SmartCard sent a WTX request */
                if (sdata[LEN] != 1)
                {
                  /* ---Wrong length: sdata[LEN]--- */
                  /* Send an R-Block signalling "Other Error" */
                  slen = T1_BuildBlock(p_t1, sdata, nad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
                }
                else
                {
                  /* ---The card sent S(WTX_request) with wtx=sdata[DATA]--- */
                  p_t1->wtx = sdata[DATA];
                  Buffer_putc(&tbuf, sdata[DATA]);
                }
                break;

              default:

                /* Wrong S-Block  */
                status = T1_DO_RESYNC;
                break;
            } /* switch (T1_S_TYPE(pcb)) */

            /* Send the S-block response according to the received S-Block request */
            slen = T1_BuildBlock(p_t1, sdata, nad, T1_S_BLOCK | T1_S_RESPONSE | T1_S_TYPE(pcb), &tbuf, NULL);
          }
          break;

        default:
          break;
      } /* End switch (T1_GetBlockType(pcb))*/

      /* Everything went splendid, initialize the number of retries */
      if (status == T1_OK)
      {
        retries = p_t1->retries;
      }
    }

    if ( status == T1_DO_RESYNC )
    {
      /* the number or re-syncs is limited, too */
      /* ISO 7816-3 Rule 6.4 */
      if (resyncs == 0)
      {
        status = T1_ERR_TIMEOUT;
      }
      else
      {
        /* ---Sending S-Block RESYNCH Req--- */
        /* ISO 7816-3 Rule 6 */
        resyncs--;
        p_t1->ns = 0;
        p_t1->nr = 0;
        slen = T1_BuildBlock(p_t1, sdata, nad, T1_S_BLOCK | T1_S_RESYNC, NULL, NULL);
        p_t1->state = T1_RESYNCH;
        p_t1->more = 0;
        retries = 1;
      }
    }
  }

  if (status == T1_OK)
  {
    status = Buffer_avail(&rbuf);
  }
  else
  {
    /* ---SCard should be initialized or deactivated--- */
    p_t1->state = SC_POWER_OFF;  /* At this stage, the interface device should reset the card */
    status = T1_ERR_TIMEOUT;
  }
  return status;
}


/**
  * @brief  Determine the type of a block.
  * @param  pcb: the PCB field of the block (the 2nd byte of the prologue field).
  * @retval int8_t the type of the block. The returned value can be one of
  *   the following:
  *     - T1_I_BLOCK : in case of I-Block.
  *     - T1_R_BLOCK : in case of R-Block.
  *     - T1_S_BLOCK : in case of S-Block.
  */
static uint8_t T1_GetBlockType(uint8_t pcb)
{
  uint8_t retval = T1_I_BLOCK;

  switch (pcb & 0xC0)
  {
    case T1_R_BLOCK:
      retval =  T1_R_BLOCK;
      break;

    case T1_S_BLOCK:
      retval =  T1_S_BLOCK;
      break;

    default:
      break;
  }

  return retval;
}

/**
  * @brief  Determine the sequence number of the block.
  * @param  pcb: the PCB field of the block (the 2nd byte of the prologue field).
  * @retval the sequence number of the block. The returned value can
  *   be one of the following:
  *     - 1 : sequence number equal to 1.
  *     - 0 : sequence number equal to 0.
  */
static uint8_t T1_GetBlockSequence(uint8_t pcb)
{
  uint8_t  retval = (pcb >> T1_I_SEQ_SHIFT) & 1;

  switch (pcb & 0xC0)
  {
    case T1_R_BLOCK:
      retval = (pcb >> T1_R_SEQ_SHIFT) & 1; /* Get bit5 of the PCB's R-block */
      break;

    case T1_S_BLOCK:
      retval =  0; /* Does not have sequence number */
      break;

    default:
      break;   /* Get bit7 of the PCB's I-block */
  }

  return retval;
}

/**
  * @brief  Build a Block.
  * @param  p_t1: pointer to context structure of T=1 protocol.
  * @param  p_tx_block: a pointer to the buffer of the block to be sent.
  * @param  nad: the value of the node address.
  * @param  pcb: the PCB byte to be sent in the prologue field.
  * @param  p_tx_buff: a pointer to the structure of the buffer to be sent.
  * @param  p_tx_inf_len: a pointer to a variable that contains the length of
  *         the information field (INF).
  * @retval the length of the whole block (NAD+PCB+LEN+INF+EDC).
  */
static uint32_t T1_BuildBlock(SCProtocol_t * p_t1, uint8_t *p_tx_block, uint8_t nad, uint8_t pcb, buffer_TypeDef *p_tx_buff, uint32_t *p_tx_inf_len)
{
  uint32_t len;
  uint8_t more = 0;

  /* if p_tx_buff=0: len <- 0,
  if p_tx_buff!=0: len <- value returned by Buffer_avail(p_tx_buff) */
  len = p_tx_buff ? Buffer_avail(p_tx_buff) : 0;

  if (len > p_t1->ifsc) /* If the information field length is greater than */
  {                   /* the maximum information field of the card: chaining function */
    pcb |= T1_MORE_BLOCKS;
    len = p_t1->ifsc;
    more = 1;
  }

  /* Add the sequence number */
  switch (T1_GetBlockType(pcb))
  {
    case T1_R_BLOCK:
      pcb |= p_t1->nr << T1_R_SEQ_SHIFT;
      break;

    case T1_I_BLOCK:
      pcb |= p_t1->ns << T1_I_SEQ_SHIFT;
      p_t1->more = more;
      break;

    default:
      break;
  }

  /* Build the Prologue field */
  p_tx_block[0] = nad; /* NAD field */
  p_tx_block[1] = pcb; /* PCB field */
  p_tx_block[2] = (uint8_t)len; /* The length of the data field (INF field) */

  if (len)
  {
    memcpy(p_tx_block + 3, Buffer_head(p_tx_buff), len);
  }

  if (p_tx_inf_len)
  {
    *p_tx_inf_len = len; /* At this stage len contains the length of the data field (INF field) */
  }

  /* Compute the checksum of the buffer (from NAD to INF) and
  return the value of the length of the whole buffer NAD -> CRC */
  len = T1_ComputeChecksum(p_t1, p_tx_block, len + 3);

  /* memorize the last sent block */
  /* only 4 bytes since we are only interested in R-blocks */
  memcpy(p_t1->previous_block, p_tx_block, 3 + p_t1->rc_bytes);

  /* Return the length of the whole buffer (NAD+PCB+LEN+INF+EDC) */
  return len;
}


/**
  * @brief  rebuild the last sent R-Block.
  * @param  p_t1: pointer to p_t1 structure of T=1 protocol.
  * @param  pblock: a pointer to a buffer to send.
  * @retval the length of the block. The returned value can
  *   be one of the following:
  *     - =!0 : the length of the R-Block.
  *     - 0 : if the previous block wasn't a R-Block.
  */
static uint32_t T1_RebuildRBlock(SCProtocol_t *p_t1, uint8_t *pblock)
{
  uint8_t pcb = p_t1->previous_block[1];

  /* Copy the last sent block */
  if (T1_R_BLOCK == T1_GetBlockType(pcb))
  {
    memcpy(pblock, p_t1->previous_block, 3 + p_t1->rc_bytes);
  }
  else
  {
    /* ---previous block was not R-Block: PCB = pcb--- */
    return 0;
  }

  /* 3 bytes of prologue field + EDC length */
  return (3 + p_t1->rc_bytes);
}

/**
  * @brief  Compute the checksum value depending on the used EDC (LRC or CRC).
  * @param  p_t1: a pointer to p_t1 structure of T=1 protocol.
  * @param  pdata: a pointer to the data buffer to apply the checksum.
  * @param  len: the length of the data buffer.
  * @retval the sum of the length of the buffer and the length of the Checksum fields.
  */
static uint32_t T1_ComputeChecksum(SCProtocol_t * p_t1, uint8_t *pdata, uint32_t len)
{
  /* Compute the checksum of data and put it in (data+len) position*/
  return len + p_t1->checksum(pdata, len, pdata + len);
}


/**
  * @brief  verify the checksum.
  * @param  p_t1: a pointer to context structure of T=1 protocol.
  * @param  prbuf: a pointer to the buffer to check its checksum.
  * @param  len: the length of the buffer including the EDC length.
  * @retval the status of the checksum. The returned value can
  *   be one of the following:
  *     - 1: checksum ok.
  *     - 0: checksum error.
  */
static HAL_StatusTypeDef T1_VerifyChecksum(SCProtocol_t * p_t1, uint8_t *prbuf, uint32_t len)
{
  uint8_t csum[2];
  int32_t m, n;
  HAL_StatusTypeDef result = HAL_ERROR;

  /* Compute the length of the block with EDC not included (NAD->INF) */
  m = len - p_t1->rc_bytes;

  /* Store the length of EDC */
  n = p_t1->rc_bytes;

  if (m >= 0)
  {
    /* Compute the checksum of the buffer pointed by rbuf and put it in csum buffer: LRC or CRC */
    p_t1->checksum(prbuf, m, csum);

    /* Compare the received Checksum (EDC) to the expected one */
    if (!memcmp(prbuf + m, csum, n))
    {
      result = HAL_OK; /* Checksum OK */
    }
  }
  return result; /* Checksum Error */
}

/**
  * @brief  Send IFS request to indicate a new IFSD that can support (reader side).
  * @param  p_t1: a pointer to context structure of T=1 protocol.
  * @param  nad: the value of the node address.
  * @param  ifsd: the new value of the reader IFS to be negotiated.
  * @retval int32_t the status of the IFSD request communication. The returned value can
  *   be one of the following:
  *     - (>0): if the transmission was successful.
  *     - (<1): if the transmission failed.
  */
int32_t T1_Negotiate_IFSD(SCProtocol_t * p_t1, uint8_t nad, uint8_t ifsd)
{
  buffer_TypeDef sbuf;
  uint8_t sdata[5];

  uint32_t slen;
  int32_t retransmission_attempt;
  int32_t resychronization_attempt = 3; /* Number of resynchronisation attempt */
  size_t snd_len;
  int32_t n = -1;
  uint8_t snd_buf[1];  /* Information field of S-Block to be sent */

  /* Get the number of retries */
  retransmission_attempt = p_t1->retries;

  /* S-block IFSD request */
  snd_buf[0] = ifsd;
  snd_len = 1;

  /* Initialize send/receive buffer */
  Buffer_set(&sbuf, (void *)snd_buf, snd_len);

  /* ---Sending IFSD request with value ifsd--- */

  while ((retransmission_attempt > 0) && (n < 0))
  {
    /* Build the S-block with nad=0000 (0), PCB=11000001 (0xC0 | 0x01) and return the S-Block length */
    slen = T1_BuildBlock(p_t1, sdata, nad, T1_S_BLOCK | T1_S_IFS, &sbuf, NULL);

    /* Send and the S-block IFS request and get the S-block IFS response */
    n = T1_TxRxBlock(p_t1, sdata, slen);

    retransmission_attempt--;

    if ((n < 0)                       /* Parity error or block malformed or BWT time-out */
        || (sdata[DATA] != ifsd)  /* Wrong ifsd received */
        || (sdata[NAD] != SWAP_NIBBLES(nad)) /* wrong NAD */
        || (T1_VerifyChecksum(p_t1, sdata, n) != HAL_OK) /* checksum failed */
        || (n != 4 + p_t1->rc_bytes)       /* wrong frame length */
        || (sdata[LEN] != 1)   /* wrong data length */
        || (sdata[PCB] != (T1_S_BLOCK | T1_S_RESPONSE | T1_S_IFS))) /* wrong PCB */
    {
      n = -1;
    }
  }

  if ( n < 0 )
  {
    do /* Do maximum three resynchronisation attempts */
    {
      /* the number or re-syncs is limited, too */
      /* ISO 7816-3 Rule 6.4 */
      resychronization_attempt--;

      if (resychronization_attempt == 0)
      {
        /* ---SCard should be initialized or deactivated--- */
        p_t1->state = SC_POWER_OFF;
        n = -1;
      }
      else
      {
        /* ---Sending S-Block RESYNCH Req n resychronization_attempt--- */
        /* ISO 7816-3 Rule 6 */
        p_t1->ns = 0;
        p_t1->nr = 0;
        slen = T1_BuildBlock(p_t1, sdata, nad, T1_S_BLOCK | T1_S_RESYNC, NULL, NULL);
        p_t1->state = T1_RESYNCH;
        p_t1->more = 0;
        /* Send the S-block (resynchronization request */
        n = T1_TxRxBlock(p_t1, sdata, slen);
      }
    }
    while (
      (p_t1->state != SC_POWER_OFF )
      && (
        (n < 0)
        || (sdata[NAD] != SWAP_NIBBLES(nad)) /* wrong NAD */
        || (T1_VerifyChecksum(p_t1, sdata, n) != HAL_OK) /* checksum failed */
        || (n != 3 + p_t1->rc_bytes)       /* wrong frame length */
        || (sdata[LEN] != 0)   /* wrong data length */
        || (sdata[PCB] != (T1_S_BLOCK | T1_S_RESPONSE | T1_S_RESYNC))
      )
    ); /* wrong PCB */

    if  (p_t1->state != SC_POWER_OFF )
    {
      /* If the resynchronization has succeeded then make one S-IFS request */
      /* Build the S-block with nad=0000 (0), PCB=11000001 (0xC0 | 0x01) and return the S-Block length */
      slen = T1_BuildBlock(p_t1, sdata, nad, T1_S_BLOCK | T1_S_IFS, &sbuf, NULL);

      /* Send and the S-block IFS request and get the S-block IFS response */
      n = T1_TxRxBlock(p_t1, sdata, slen);

      /* If the S-block IFS response has failed, deactivate the card */
      if ((n == T1_ERR_PARITY) || (n == T1_ERR_STRUCT)       /* Parity error or block malformed */
          || (sdata[DATA] != ifsd)          /* Wrong ifsd received */
          || (sdata[NAD] != SWAP_NIBBLES(nad)) /* wrong NAD */
          || (T1_VerifyChecksum(p_t1, sdata, n) != HAL_OK) /* checksum failed */
          || (n != 4 + p_t1->rc_bytes)               /* wrong frame length */
          || (sdata[LEN] != 1)   /* wrong data length */
          || (sdata[PCB] != (T1_S_BLOCK | T1_S_RESPONSE | T1_S_IFS))) /* wrong PCB */
      {
        p_t1->state = SC_POWER_OFF;
        n = -1;
      }
    }
  }
  /* ---IFSD request succeeded after resynchronization--- */
  return n;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
