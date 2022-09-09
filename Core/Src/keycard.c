#include "keycard.h"
#include "stm32g0c1e_eval_lcd.h"
SCProtocol_t SCInterface;

#define CARD_CHILL_TIME      ((uint32_t)4)  /* Not to strain the card too much */

/* Private variables ---------------------------------------------------------*/
static SC_APDU_t cAPDU;  /* APDU Command buffer */
static SC_APDU_t rAPDU;  /* APDU Response buffer */

/* Private constants ---------------------------------------------------------*/
/* T0 Directories & Files ID */
static const uint8_t MasterRoot[2] =
  {
    0x3F, 0x00
  };
static const uint8_t GSMDir[2] =
  {
    0x7F, 0x20
  };
static const uint8_t ICCID[2] =
  {
    0x2F, 0xE2
  };
static const uint8_t IMSI[2] =
  {
    0x6F, 0x07
  };
static const uint8_t CHV1[8] =
  {'0', '0', '0', '0', '0', '0', '0', '0'
  };

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef TEST_T0( void );
static HAL_StatusTypeDef TEST_T1( void );

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Brief testing of T0 card protocol interface
 * @retval HAL_StatusTypeDef Test status
 */
static HAL_StatusTypeDef TEST_T0( void )
{
  uint8_t index;
  uint32_t chv1_status = 0;
  __IO uint8_t iccid_context[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  __IO uint8_t imsi_context[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  /* Select MF -------------------------------------------------------------*/
  cAPDU.APDU_S.Header.CLA = SC_CLA_GSM11;
  cAPDU.APDU_S.Header.INS = SC_SELECT_FILE;
  cAPDU.APDU_S.Header.P1 = 0x00;
  cAPDU.APDU_S.Header.P2 = 0x00;
  cAPDU.APDU_S.Body.LC = 0x02;

  T0_APDU_Prep(&cAPDU, (uint8_t*)&MasterRoot);

  T0_APDU(&SCInterface, &cAPDU, &rAPDU);

  /* Get Response on MF ----------------------------------------------------*/
  if (rAPDU.APDU_R.SW1 == SC_DF_SELECTED)
  {
    cAPDU.APDU_S.Header.CLA = SC_CLA_GSM11;
    cAPDU.APDU_S.Header.INS = SC_GET_RESPONCE;
    cAPDU.APDU_S.Header.P1 = 0x00;
    cAPDU.APDU_S.Header.P2 = 0x00;
    cAPDU.APDU_S.Body.LC = 0x00;
    cAPDU.APDU_S.Body.LE = rAPDU.APDU_R.SW2;

    T0_APDU(&SCInterface, &cAPDU, &rAPDU);
  }

  /* Select ICCID ----------------------------------------------------------*/
  if (((rAPDU.APDU_R.SW1 << 8) | (rAPDU.APDU_R.SW2)) == SC_OP_TERMINATED)
  {
    /* Check if the CHV1 is enabled */
    if ((rAPDU.APDU_R.Data[13] & 0x80) == 0x00)
    {
      chv1_status = 0x01;
    }
    /* Send APDU Command for ICCID selection */
    cAPDU.APDU_S.Header.INS = SC_SELECT_FILE;
    cAPDU.APDU_S.Header.P1 = 0x00;
    cAPDU.APDU_S.Header.P2 = 0x00;
    cAPDU.APDU_S.Body.LC = 0x02;

    T0_APDU_Prep(&cAPDU, (uint8_t*)&ICCID);

    T0_APDU(&SCInterface, &cAPDU, &rAPDU);
  }

  /* Read Binary in ICCID --------------------------------------------------*/
  if (rAPDU.APDU_R.SW1 == SC_EF_SELECTED)
  {
    cAPDU.APDU_S.Header.CLA = SC_CLA_GSM11;
    cAPDU.APDU_S.Header.INS = SC_READ_BINARY;
    cAPDU.APDU_S.Header.P1 = 0x00;
    cAPDU.APDU_S.Header.P2 = 0x00;
    cAPDU.APDU_S.Body.LC = 0x00;
    cAPDU.APDU_S.Body.LE = 10;

    T0_APDU(&SCInterface, &cAPDU, &rAPDU);
  }

  /* Select GSMDir ---------------------------------------------------------*/
  if (((rAPDU.APDU_R.SW1 << 8) | (rAPDU.APDU_R.SW2)) == SC_OP_TERMINATED)
  {
    /* Copy the ICCID File content into iccid_context buffer */
    for (index = 0; index < cAPDU.APDU_S.Body.LE; index++)
    {
      iccid_context[index] =  rAPDU.APDU_R.Data[index];
    }
    /* Send APDU Command for GSMDir selection */
    cAPDU.APDU_S.Header.INS = SC_SELECT_FILE;
    cAPDU.APDU_S.Header.P1 = 0x00;
    cAPDU.APDU_S.Header.P2 = 0x00;
    cAPDU.APDU_S.Body.LC = 0x02;

    T0_APDU_Prep(&cAPDU, (uint8_t*)&GSMDir);

    T0_APDU(&SCInterface, &cAPDU, &rAPDU);
  }

  /* Select IMSI -----------------------------------------------------------*/
  if (rAPDU.APDU_R.SW1 == SC_DF_SELECTED)
  {
    cAPDU.APDU_S.Header.INS = SC_SELECT_FILE;
    cAPDU.APDU_S.Header.P1 = 0x00;
    cAPDU.APDU_S.Header.P2 = 0x00;
    cAPDU.APDU_S.Body.LC = 0x02;

    T0_APDU_Prep(&cAPDU, (uint8_t*)&IMSI);

    T0_APDU(&SCInterface, &cAPDU, &rAPDU);
  }

  /* Get Response on IMSI File ---------------------------------------------*/
  if (rAPDU.APDU_R.SW1 == SC_EF_SELECTED)
  {
    cAPDU.APDU_S.Header.CLA = SC_CLA_GSM11;
    cAPDU.APDU_S.Header.INS = SC_GET_RESPONCE;
    cAPDU.APDU_S.Header.P1 = 0x00;
    cAPDU.APDU_S.Header.P2 = 0x00;
    cAPDU.APDU_S.Body.LC = 0x00;
    cAPDU.APDU_S.Body.LE = rAPDU.APDU_R.SW2;

    T0_APDU(&SCInterface, &cAPDU, &rAPDU);
  }

  /* Read Binary in IMSI ---------------------------------------------------*/
  if (chv1_status == 0x00)
  {
    if (((rAPDU.APDU_R.SW1 << 8) | (rAPDU.APDU_R.SW2)) == SC_OP_TERMINATED)
    {
      /* Enable CHV1 (PIN1) ------------------------------------------------*/
      cAPDU.APDU_S.Header.INS = SC_ENABLE;
      cAPDU.APDU_S.Header.P1 = 0x00;
      cAPDU.APDU_S.Header.P2 = 0x01;
      cAPDU.APDU_S.Body.LC = 0x08;

      T0_APDU_Prep(&cAPDU, (uint8_t*)&CHV1);

      T0_APDU(&SCInterface, &cAPDU, &rAPDU);
    }
  }
  else
  {
    if (((rAPDU.APDU_R.SW1 << 8) | (rAPDU.APDU_R.SW2)) == SC_OP_TERMINATED)
    {
      /* Verify CHV1 (PIN1) ------------------------------------------------*/
      cAPDU.APDU_S.Header.INS = SC_VERIFY;
      cAPDU.APDU_S.Header.P1 = 0x00;
      cAPDU.APDU_S.Header.P2 = 0x01;
      cAPDU.APDU_S.Body.LC = 0x08;      
      
      T0_APDU_Prep(&cAPDU, (uint8_t*)&CHV1);

      T0_APDU(&SCInterface, &cAPDU, &rAPDU);
    }
  }
  /* Read Binary in IMSI ---------------------------------------------------*/
  if (((rAPDU.APDU_R.SW1 << 8) | (rAPDU.APDU_R.SW2)) == SC_OP_TERMINATED)
  {
    cAPDU.APDU_S.Header.CLA = SC_CLA_GSM11;
    cAPDU.APDU_S.Header.INS = SC_READ_BINARY;
    cAPDU.APDU_S.Header.P1 = 0x00;
    cAPDU.APDU_S.Header.P2 = 0x00;
    cAPDU.APDU_S.Body.LC = 0x00;
    cAPDU.APDU_S.Body.LE = 9;

    T0_APDU(&SCInterface, &cAPDU, &rAPDU);
  }

  if (((rAPDU.APDU_R.SW1 << 8) | (rAPDU.APDU_R.SW2)) == SC_OP_TERMINATED)
  {
    /* Copy the IMSI File content into imsi_context buffer */
    for (index = 0; index < cAPDU.APDU_S.Body.LE; index++)
    {
      imsi_context[index] =  rAPDU.APDU_R.Data[index];
    }
  }
  
  return HAL_OK;
}

/**
 * @brief Test T1 Protocol
 * @retval HAL_StatusTypeDef Test status
 */
static HAL_StatusTypeDef TEST_T1( void )
{
  uint32_t apdu_index;   /* Index of the APDU buffer */
  int32_t resp_status = 0; /* Communication Response status */

  BSP_LCD_DisplayStringAtLine(4, "Testing T1");
  /*------- Send IFSD request --------------------------------------------------*/

  /* Negotiate IFSD: we indicate to the card a new IFSD that the reader can support */
  resp_status = T1_Negotiate_IFSD(&SCInterface, NAD, IFSD_VALUE);

  /* If the IFSD request communication has failed */
  if (resp_status < 0)
  {
    /* ---IFSD communication error--- */
    Error_Handler();
  }

  /*------- Send APDU: Select File Command -------------------------------------*/

  /* Send Select File Command */
  cAPDU.APDU_S.Header.CLA = SC_CLASS;                    /* CLA */
  cAPDU.APDU_S.Header.INS = INS_SELECT_FILE;             /* INS: Select File */
  cAPDU.APDU_S.Header.P1 = 0x00;                        /* P1 */
  cAPDU.APDU_S.Header.P2 = 0x00;                        /* P2 */
  cAPDU.APDU_S.Body.LC = 0x02;                        /* Lc */
  cAPDU.APDU_S.Body.Data[0] = (uint8_t)(SC_FILE_ID >> 8);    /* Data 1 */
  cAPDU.APDU_S.Body.Data[1] = (uint8_t)(SC_FILE_ID & 0xFF);  /* Data 2 */

  /* ---Sending APDU:  cAPDU, 7--- */

  HAL_Delay(CARD_CHILL_TIME);
  /* Send/Receive APDU command/response: Select File having ID = SC_FILE_ID */
  resp_status = T1_APDU(&SCInterface, NAD, &cAPDU, &rAPDU);

  /* If the APDU communication has failed */
  if (resp_status < 0)
  {
    /* ---APDU communication error--- */
    Error_Handler();
  }
  else
  {
    /* ---APDU response: rAPDU, resp_status--- */
  }


  /*------- Send APDU: Read File Command ---------------------------------------*/

  /* Select file */
  cAPDU.APDU_S.Header.CLA = SC_CLASS;       /* CLA */
  cAPDU.APDU_S.Header.INS = INS_READ_FILE;  /* INS: Read File */
  cAPDU.APDU_S.Header.P1 = 0x00;           /* P1 */
  cAPDU.APDU_S.Header.P2 = 0x00;           /* P2 */
  cAPDU.APDU_S.Body.LC = 0x00;           /* Lc: read 256 bytes */

  /* ---Sending APDU: cAPDU, 5--- */

  HAL_Delay(CARD_CHILL_TIME);
  /* Send/Receive APDU command/response: Read File (256 bytes) */
  resp_status = T1_APDU(&SCInterface, NAD, &cAPDU, &rAPDU);

  /* If the APDU communication has failed */
  if (resp_status < 0)
  {
    /* ---APDU communication error--- */
    Error_Handler();
  }
  else
  {
    /* ---APDU response: rAPDU, resp_status--- */
  }

  /*------- Send APDU: Write File Command --------------------------------------*/

  /* Select file */
  cAPDU.APDU_S.Header.CLA = SC_CLASS;        /* CLA */
  cAPDU.APDU_S.Header.INS = INS_WRITE_FILE;  /* INS: Write File */
  cAPDU.APDU_S.Header.P1 = 0x00;            /* P1 */
  cAPDU.APDU_S.Header.P2 = 0x00;            /* P2 */
  cAPDU.APDU_S.Body.LC = 0xFF;            /* Lc */

  apdu_index = 5;

  while (apdu_index < 261)
  {
    /* initialize the APDU buffer (data field) */
    cAPDU.buffer[apdu_index] = apdu_index - 5;
    apdu_index++;
  }

  /* ---Sending APDU--- */

  HAL_Delay(CARD_CHILL_TIME);
  /* Send/Receive APDU command/response: Write File 256 bytes */
  resp_status = T1_APDU(&SCInterface, NAD, &cAPDU, &rAPDU);

  /* If the APDU communication has failed */
  if (resp_status < 0)
  {
    /* ---APDU communication error--- */
    Error_Handler();
  }
  else
  {
    /* ---APDU response: rAPDU, resp_status--- */
  }

  /*------- Send APDU: Read File Command ---------------------------------------*/

  /* Select file */
  cAPDU.APDU_S.Header.CLA = SC_CLASS;       /* CLA */
  cAPDU.APDU_S.Header.INS = INS_READ_FILE;  /* INS: Read File */
  cAPDU.APDU_S.Header.P1 = 0x00;           /* P1 */
  cAPDU.APDU_S.Header.P2 = 0x00;           /* P2 */
  cAPDU.APDU_S.Body.LC   = 0x00;           /* Lc */

  /* ---Sending APDU:  cAPDU, 5--- */

  HAL_Delay(CARD_CHILL_TIME);
  /* Send/Receive APDU command/response: Read File (256 bytes) */
  resp_status = T1_APDU(&SCInterface, NAD, &cAPDU, &rAPDU);

  /* If the APDU communication has failed */
  if (resp_status < 0)
  {
    /* ---APDU communication error--- */
    Error_Handler();
  }
  else
  {
    /* ---APDU response: rAPDU, resp_status--- */
  }

  /*------- Send APDU: Write File Command --------------------------------------*/

  /* Select file */
  cAPDU.APDU_S.Header.CLA = SC_CLASS;        /* CLA */
  cAPDU.APDU_S.Header.INS = INS_WRITE_FILE;  /* INS: Write File */
  cAPDU.APDU_S.Header.P1  = 0x00;            /* P1 */
  cAPDU.APDU_S.Header.P2  = 0x00;            /* P2 */
  cAPDU.APDU_S.Body.LC    = 0xFF;            /* Lc */

  apdu_index = 5;

  while (apdu_index < 261)
  {
    /* Get and inverse the file received from the card */
    cAPDU.buffer[apdu_index] = ~(rAPDU.buffer[apdu_index-5]);
    apdu_index++;
  }

  /* ---Sending APDU: ", cAPDU, 261--- */

  HAL_Delay(CARD_CHILL_TIME);
  /* Send/Receive APDU command/response: Write File (256 bytes) */
  resp_status =  T1_APDU(&SCInterface, NAD, &cAPDU, &rAPDU);

  /* If the APDU communication has failed */
  if (resp_status < 0)
  {
    /* ---APDU communication error--- */
    Error_Handler();
  }
  else
  {
    /* ---APDU response: rAPDU, resp_status--- */
  }

  /*------- Send APDU: Read File Command -------------------------------------*/

  /* Select file */
  cAPDU.APDU_S.Header.CLA = SC_CLASS;       /* CLA */
  cAPDU.APDU_S.Header.INS = INS_READ_FILE;  /* INS: Read File */
  cAPDU.APDU_S.Header.P1  = 0x00;           /* P1 */
  cAPDU.APDU_S.Header.P2  = 0x00;           /* P2 */
  cAPDU.APDU_S.Body.LC    = 0x00;           /* Lc */

  /* ---Sending APDU: cAPDU, 5--- */

  HAL_Delay(CARD_CHILL_TIME);
  /* Send/Receive APDU command/response: Read File (256 bytes) */
  resp_status =  T1_APDU(&SCInterface, NAD, &cAPDU, &rAPDU);

  /* If the APDU communication has failed */
  if (resp_status < 0)
  {
    /* ---APDU communication error--- */
    Error_Handler();
  }
  else
  {
    /* ---APDU response:  rAPDU, resp_status--- */
    /* ---All operations were executed successfully --- */
  }

  return HAL_OK;
}

int Keycard_Run(SMARTCARD_HandleTypeDef* SmartCardHandle) {
  uint16_t atr_size;
  uint8_t  atr_buff[40];
  uint8_t convention;
  ATR_TypeDef atr;        /* Answer To Reset structure */
  uint32_t card_clk = 0;  /* The Smartcard clock frequency in Hz */
  uint32_t etu_usec = 0;  /* Elementary Time Unit in microsecond */
  int8_t protocol = -1;   /* Default protocol (initialized to -1) */


  /* MCU Configuration----------------------------------------------------------*/

  /* Assign the device */
  SCInterface.pdevice = SmartCardHandle;

  /* Checking initial card state -----------------------------------------------*/
  if (HAL_GPIO_ReadPin(SC_NOFF_GPIO_Port, SC_NOFF_Pin) == GPIO_PIN_SET)
  {
    /* Smartcard detected */
    SCInterface.state = SC_POWER_ON;
    HAL_GPIO_WritePin(SC_NCMDVCC_GPIO_Port, SC_NCMDVCC_Pin, GPIO_PIN_RESET);
  }
  else
  {
    /* power off smartcard*/
    SCInterface.state = SC_POWER_OFF;
    HAL_GPIO_WritePin(SC_NCMDVCC_GPIO_Port, SC_NCMDVCC_Pin, GPIO_PIN_SET);
  }

  /* Infinite loop */
  while (1)
  {
    /* waiting for card to come online */
    while (SCInterface.state == SC_POWER_OFF)
    {}

    /* Insert delay of 20ms for signal stabilization */
    HAL_Delay(20);

    /* Initialising the ATR structure to zero */
    for (protocol = 0; protocol < 40; protocol++)
    {
      ((uint8_t*)&atr)[protocol] = 0;
      atr_buff[protocol] = 0;
    }

    /* Read ATR - loop until it's read */
    do
    {
      atr_size = sizeof(atr_buff);
    }
    while (HAL_OK != ATR_Read(&SCInterface, atr_buff, &atr_size));

    /* Decode the ATR */
    convention = ATR_Decode(&atr, atr_buff, atr_size );
    if ( convention > ATR_INVERSE)
    {
      Error_Handler();
    }
    if ( convention == ATR_INVERSE )
    {
      SmartCardHandle->Instance->CR1 &= ~USART_CR1_UE;
      SmartCardHandle->Instance->CR2 |= SMARTCARD_ADVFEATURE_DATAINV_ENABLE | USART_CR2_MSBFIRST;
      SmartCardHandle->Instance->CR1 |= USART_CR1_UE;
    }

    /********************* Protocol initialization ****************************/

    /* Init the protocol structure */
    ATR_GetDefaultProtocol( &atr, &protocol);
    if ( protocol == ATR_PROTOCOL_TYPE_T1 )
    {
      T1_Protocol_Init(&SCInterface, card_clk);
      SCInterface.state = SC_ACTIVE_ON_T1;
      /* Set F and D parameters and get the new etu value in micro-seconds */
      etu_usec = Set_F_D_parameters(&SCInterface, &atr, card_clk);
    }
    else if ( protocol == ATR_PROTOCOL_TYPE_T0 )
    {
      SCInterface.state = SC_ACTIVE_ON_T0;
    }

    /* Set the etu to be used by the protocol */
    SCInterface.etu_us = etu_usec;

    /* Set the convention of the protocol */
    SCInterface.convention = atr.TS;

    if ( protocol == ATR_PROTOCOL_TYPE_T1 )
    {
      /* Set and configure the extra guard time value */
      Set_EGT(SmartCardHandle, &atr);

      /* Set the IFSC (card IFS) value */
      Set_IFSC(&SCInterface, &atr);

      /* Set the CWT and BWT values */
      Set_CWT_BWT(&SCInterface, &atr);

      /* Set the EDC type (LRC or CRC) */
      Set_EDC(&SCInterface, &atr);

      TEST_T1();
    }
    else
    {
      TEST_T0();
    }

    /* waiting for card to come off line */
    while (SCInterface.state != SC_POWER_OFF)
    {}

    /* disable the peripheral */
    __HAL_SMARTCARD_DISABLE(SmartCardHandle);
  }  
}

void Keycard_Card_In() {
  /* Smartcard detected */
  SCInterface.state = SC_POWER_ON;

  /* Enable CMDVCC */
  SC_POWER(GPIO_PIN_RESET);
}

void Keycard_Card_Out() {
  /* Reset the card */
  SC_RESET(DISABLE);

  /* Smartcard lost */
  SCInterface.state = SC_POWER_OFF;

  /* Enable CMDVCC */
  SC_POWER(GPIO_PIN_SET);
}
