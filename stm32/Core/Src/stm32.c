#include <string.h>

#include "main.h"
#include "hal.h"
#include "keypad/keypad.h"
#include "linked_list.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pwr.h"

#define HAL_TIMEOUT 250
#define PKA_TIMEOUT 500
#define SC_RESET_DELAY 10
#define CLOCK_STABLE_DELAY 5
#define SMARTCARD_STOPBITS_1 0x00000000U
#define ANALOG_DIV 4095
#define VBAT_OFFSET 1660

#define FLASH_BANK_SWAPPED() (FLASH->OPTSR_CUR & FLASH_OPTSR_SWAP_BANK)

#define HAL_WAIT(__PREDICATE__) { \
  uint32_t base = HAL_GetTick(); \
\
  while (__PREDICATE__) {\
    if ((HAL_GetTick() - base) > HAL_TIMEOUT) {\
      return HAL_FAIL;\
    }\
  }\
}

#define __PKA_RAM_PARAM_END(TAB,INDEX) { TAB[INDEX] = 0UL; TAB[INDEX + 1U] = 0UL;}

HAL_StatusTypeDef PKA_Process(PKA_HandleTypeDef *hpka, uint32_t mode, uint32_t Timeout);
void PKA_Memcpy_u8_to_u32(__IO uint32_t dst[], const uint8_t src[], size_t n);
void PKA_Memcpy_u32_to_u8(uint8_t dst[], __IO const uint32_t src[], size_t n);
void PKA_Memcpy_u32_to_u32(__IO uint32_t dst[], __IO const uint32_t src[], size_t n);

extern DMA_QListTypeDef Camera_DMA_LL;
extern DMA_NodeTypeDef Camera_DMA_Node1;
extern DMA_NodeTypeDef Camera_DMA_Node2;

const uint8_t BN_ONE[BN_SIZE] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};

struct gpio_pin_spec {
  GPIO_TypeDef* base;
  uint16_t pin;
};

enum dcmi_buf_status {
  DCMI_READY,
  DCMI_ACQUIRING,
  DCMI_ACQUIRED,
  DCMI_PROCESSING
};

struct dcmi_buf {
  enum dcmi_buf_status status;
  uint8_t* addr;
};

const struct gpio_pin_spec STM32_PIN_MAP[] = {
  {GPIO_CAMERA_PWDN_GPIO_Port, GPIO_CAMERA_PWDN_Pin},
  {GPIO_CAM_EN_GPIO_Port, GPIO_CAM_EN_Pin},
  {GPIO_CAMERA_RST_GPIO_Port, GPIO_CAMERA_RST_Pin},
  {GPIO_LCD_CD_GPIO_Port, GPIO_LCD_CD_Pin},
  {GPIO_LCD_RST_GPIO_Port, GPIO_LCD_RST_Pin},
  {GPIO_KEYPAD_ROW_0_GPIO_Port, GPIO_KEYPAD_ROW_0_Pin},
  {GPIO_KEYPAD_ROW_1_GPIO_Port, GPIO_KEYPAD_ROW_1_Pin},
  {GPIO_KEYPAD_ROW_2_GPIO_Port, GPIO_KEYPAD_ROW_2_Pin},
  {GPIO_KEYPAD_ROW_3_GPIO_Port, GPIO_KEYPAD_ROW_3_Pin},
  {GPIO_KEYPAD_COL_0_GPIO_Port, GPIO_KEYPAD_COL_0_Pin},
  {GPIO_KEYPAD_COL_1_GPIO_Port, GPIO_KEYPAD_COL_1_Pin},
  {GPIO_KEYPAD_COL_2_GPIO_Port, GPIO_KEYPAD_COL_2_Pin},
  {GPIO_VUSB_OK_GPIO_Port, GPIO_VUSB_OK_Pin},
  {GPIO_SC_PRES_GPIO_Port, GPIO_SC_PRES_Pin},
  {GPIO_HALT_REQ_GPIO_Port, GPIO_HALT_REQ_Pin},
  {GPIO_PWR_KILL_GPIO_Port, GPIO_PWR_KILL_Pin},
};

static void (*g_spi_callback)();
static TaskHandle_t g_dcmi_task = NULL;
static TaskHandle_t g_smartcard_task = NULL;
static int8_t g_acquiring;
static struct dcmi_buf g_dcmi_bufs[CAMERA_FB_COUNT];
static uint8_t g_uid[HAL_DEVICE_UID_LEN] __attribute__((aligned(4)));
static uint32_t g_adc_calibration;
static uint32_t g_adc_vref;

static inline void mco_off() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static inline void mco_on() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

#ifdef BOOTLOADER
void SysTick_Handler(void) {
  HAL_IncTick();
}
#endif

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (g_spi_callback) {
    g_spi_callback();
  }
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t gpio_pin) {
  switch(gpio_pin) {
  case GPIO_HALT_REQ_Pin:
    HAL_TIM_Base_Stop_IT(&htim3);
    break;
  case GPIO_SC_PRES_Pin:
    pwr_smartcard_inserted();
    break;
  case GPIO_VUSB_OK_Pin:
    pwr_usb_unplugged();
    break;
  }
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t gpio_pin) {
  switch(gpio_pin) {
  case GPIO_HALT_REQ_Pin:
    HAL_TIM_Base_Stop_IT(&htim3);
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(&htim3);
    break;
  case GPIO_SC_PRES_Pin:
    pwr_smartcard_removed();
    break;
  case GPIO_VUSB_OK_Pin:
    pwr_usb_plugged(true);
    break;
  }
}

static void hal_smartcard_complete(uint32_t err) {
  if (!g_smartcard_task) {
    return;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xTaskNotifyIndexedFromISR(g_smartcard_task, SMARTCARD_TASK_NOTIFICATION_IDX, err, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
  g_smartcard_task = NULL;
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_SMARTCARD_TxCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard) {
  hal_smartcard_complete(HAL_SMARTCARD_GetError(hsmartcard));
}

void HAL_SMARTCARD_RxCpltCallback(SMARTCARD_HandleTypeDef *hsmartcard) {
  __HAL_SMARTCARD_DISABLE_IT(&hsmartcard6, SMARTCARD_IT_RTO);
  hal_smartcard_complete(HAL_SMARTCARD_GetError(hsmartcard));
}

void HAL_SMARTCARD_ErrorCallback(SMARTCARD_HandleTypeDef *hsmartcard) {
  hal_smartcard_complete(HAL_SMARTCARD_GetError(hsmartcard));
}

static void inline _hal_acquire(int8_t idx) {
  g_acquiring = idx;
  g_dcmi_bufs[idx].status = DCMI_ACQUIRING;

  hdcmi.pBuffPtr = (uint32_t) g_dcmi_bufs[idx].addr;

  __HAL_DMA_DISABLE(hdcmi.DMA_Handle);
  __HAL_DMA_CLEAR_FLAG(hdcmi.DMA_Handle, (DMA_FLAG_TC | DMA_FLAG_HT | DMA_FLAG_DTE | DMA_FLAG_ULE | DMA_FLAG_USE | DMA_FLAG_SUSP | DMA_FLAG_TO));
  hdcmi.DMA_Handle->Instance->CBR1 = 0U;
  Camera_DMA_Node1.LinkRegisters[NODE_CDAR_DEFAULT_OFFSET] = hdcmi.pBuffPtr;
  Camera_DMA_Node2.LinkRegisters[NODE_CDAR_DEFAULT_OFFSET] = hdcmi.pBuffPtr + hdcmi.XferSize;

  MODIFY_REG(hdcmi.DMA_Handle->Instance->CBR1, DMA_CBR1_BNDT, (hdcmi.XferSize & DMA_CBR1_BNDT));
  hdcmi.DMA_Handle->Instance->CDAR = hdcmi.pBuffPtr;

  __HAL_DMA_ENABLE(hdcmi.DMA_Handle);

  hdcmi.Instance->CR |= DCMI_CR_CAPTURE;
}

void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi) {
  hdcmi->Instance->CR &= ~(DCMI_CR_CAPTURE);

  g_dcmi_bufs[g_acquiring].status = DCMI_ACQUIRED;
  g_acquiring = -1;

  if(g_dcmi_task == NULL) {
    return;
  }

  for (int i = 0; i < CAMERA_FB_COUNT; i++) {
    if (g_dcmi_bufs[i].status == DCMI_READY) {
      _hal_acquire(i);
      break;
    }
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveIndexedFromISR(g_dcmi_task, CAMERA_TASK_NOTIFICATION_IDX, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd) {
  hal_usb_setup_cb((uint8_t*) hpcd->Setup);
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
  hal_usb_data_out_cb(epnum);
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
  hal_usb_data_in_cb(epnum);
}

static void _hal_usb_close_ep() {
  HAL_PCD_EP_Close(&hpcd_USB_DRD_FS, HAL_USB_EPIN_ADDR);
  HAL_PCD_EP_Close(&hpcd_USB_DRD_FS, HAL_USB_EPOUT_ADDR);
}

static void _hal_usb_open_ep() {
  HAL_PCD_EP_Open(&hpcd_USB_DRD_FS, HAL_USB_EPIN_ADDR, HAL_USB_MPS, EP_TYPE_INTR);
  HAL_PCD_EP_Open(&hpcd_USB_DRD_FS, HAL_USB_EPOUT_ADDR, HAL_USB_MPS, EP_TYPE_INTR);
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd) {
  _hal_usb_close_ep();

  HAL_PCD_EP_Open(&hpcd_USB_DRD_FS, 0x00, HAL_USB_MPS, EP_TYPE_CTRL);
  HAL_PCD_EP_Open(&hpcd_USB_DRD_FS, 0x80, HAL_USB_MPS, EP_TYPE_CTRL);

  _hal_usb_open_ep();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim2) {
    keypad_scan_tick();
  } else if (htim == &htim3) {
    pwr_shutdown();
  } else if (htim == &htim5) {
    pwr_inactivity_timer_elapsed();
  }
}

static void _hal_aes_enable() {
  __HAL_RCC_SAES_CLK_ENABLE();
}

hal_err_t hal_init() {
  // Copies UID, Flash size, package info before it becomes privileged
  memcpy(g_uid, (uint32_t*) UID_BASE, HAL_DEVICE_UID_LEN);
  uint16_t vrefint_cal = *VREFINT_CAL_ADDR;

  HAL_Init();
  SystemClock_Config();
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);

  MX_RNG_Init();
  MX_HASH_Init();
  MX_CRC_Init();
  _hal_aes_enable();
  MX_PKA_Init();

  MX_GPIO_Init();
  MX_TIM6_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();

  MX_GPDMA2_Init();
  MX_GPDMA1_Init();

  MX_SPI6_Init();
  MX_I2C2_Init();
  MX_USART6_SMARTCARD_Init();
  __HAL_RCC_USART6_CLK_DISABLE();

  MX_DCMI_Init();

  mco_off();
  __HAL_DCMI_DISABLE_IT(&hdcmi, DCMI_IT_LINE | DCMI_IT_VSYNC);

  MX_Camera_DMA_LL_Config();

  __HAL_LINKDMA(&hdcmi, DMA_Handle, handle_GPDMA2_Channel5);
  HAL_DMAEx_List_LinkQ(&handle_GPDMA2_Channel5, &Camera_DMA_LL);

  MX_ICACHE_Init();

  __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  MX_ADC1_Init();
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADC_Start(&hadc1);

  if (HAL_ADC_PollForConversion(&hadc1, HAL_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  g_adc_vref = (VREFINT_CAL_VREF * vrefint_cal) / HAL_ADC_GetValue(&hadc1);
  HAL_ADCEx_EnterADCDeepPowerDownMode(&hadc1);

  MX_ADC2_Init();
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
  g_adc_calibration = HAL_ADCEx_Calibration_GetValue(&hadc2, ADC_SINGLE_ENDED);
  HAL_ADCEx_EnterADCDeepPowerDownMode(&hadc2);

  return HAL_SUCCESS;
}

hal_err_t hal_init_bootloader() {
  HAL_Init();
  SystemClock_Config();
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);

  MX_HASH_Init();

  return HAL_SUCCESS;
}

hal_err_t hal_teardown_bootloader() {
  HAL_HASH_DeInit(&hhash);
  HAL_RCC_DeInit();

  return HAL_SUCCESS;
}

hal_err_t hal_device_uid(uint8_t out[HAL_DEVICE_UID_LEN]) {
  memcpy(out, g_uid, HAL_DEVICE_UID_LEN);
  return HAL_SUCCESS;
}

hal_boot_t hal_boot_type() {
  return __HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) ? BOOT_HOT : BOOT_COLD;
}

hal_err_t hal_camera_init() {
  mco_on();
  vTaskDelay(pdMS_TO_TICKS(CLOCK_STABLE_DELAY));
  return HAL_SUCCESS;
}

hal_err_t hal_camera_start(uint8_t fb[CAMERA_FB_COUNT][CAMERA_FB_SIZE]) {
  configASSERT(g_dcmi_task == NULL);
  g_dcmi_task = xTaskGetCurrentTaskHandle();

  g_dcmi_bufs[0].status = DCMI_ACQUIRING;
  g_dcmi_bufs[0].addr = fb[0];
  g_acquiring = 0;

  for (int i = 1; i < CAMERA_FB_COUNT; i++) {
    g_dcmi_bufs[i].status = DCMI_READY;
    g_dcmi_bufs[i].addr = fb[i];
  }

  return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t) g_dcmi_bufs[0].addr, (CAMERA_FB_SIZE/4));
}

hal_err_t hal_camera_stop() {
  g_dcmi_task = NULL;
  HAL_DCMI_Stop(&hdcmi);
  mco_off();
  return HAL_SUCCESS;
}

hal_err_t hal_camera_next_frame(uint8_t** fb) {
  int ready = -1;

  for (int i = 0; i < CAMERA_FB_COUNT; i++) {
    if ((g_dcmi_bufs[i].status == DCMI_ACQUIRED)) {
      g_dcmi_bufs[i].status = DCMI_PROCESSING;
      *fb = g_dcmi_bufs[i].addr;
      return HAL_SUCCESS;
    } else if (g_dcmi_bufs[i].status == DCMI_READY) {
      ready = i;
    }
  }

  if (g_acquiring == -1 && ready != -1) {
    _hal_acquire(ready);
  }

  return HAL_FAIL;
}

hal_err_t hal_camera_submit(uint8_t* fb) {
  for (int i = 0; i < CAMERA_FB_COUNT; i++) {
    if (g_dcmi_bufs[i].addr == fb) {
      if (g_acquiring == -1) {
        _hal_acquire(i);
        return HAL_SUCCESS;
      } else {
        g_dcmi_bufs[i].status = DCMI_READY;
        return HAL_SUCCESS;
      }
    }
  }
  return HAL_FAIL;
}

void hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state) {
  HAL_GPIO_WritePin(STM32_PIN_MAP[pin].base, STM32_PIN_MAP[pin].pin, state);
}

hal_gpio_state_t hal_gpio_get(hal_gpio_pin_t pin) {
  return HAL_GPIO_ReadPin(STM32_PIN_MAP[pin].base, STM32_PIN_MAP[pin].pin);
}

hal_err_t hal_i2c_send(hal_i2c_port_t port, uint8_t addr, const uint8_t* data, size_t len) {
  assert(port == I2C_CAMERA);
  return HAL_I2C_Master_Transmit(&hi2c2, (addr << 1), (uint8_t*) data, len, HAL_TIMEOUT);
}

hal_err_t hal_spi_send(hal_spi_port_t port, const uint8_t* data, size_t len) {
  assert(port == SPI_LCD);
  return HAL_SPI_Transmit(&hspi6, (uint8_t*) data, len, HAL_TIMEOUT);
}

hal_err_t hal_spi_send_dma(hal_spi_port_t port, const uint8_t* data, size_t len, void (*cb)()) {
  assert(port == SPI_LCD);
  g_spi_callback = cb;
  return HAL_SPI_Transmit_DMA(&hspi6, (uint8_t*) data, len);
}

hal_err_t hal_rng_next(uint8_t *buf, size_t len) {
  uint32_t rnd;

  while(len >= 4) {
    while (HAL_RNG_GenerateRandomNumber(&hrng, &rnd) != HAL_OK) {}
    len -= 4;
    memcpy(buf, &rnd, 4);
    buf += 4;
  }

  if (len) {
    while (HAL_RNG_GenerateRandomNumber(&hrng, &rnd) != HAL_OK) {}
    memcpy(buf, &rnd, len);
  }

  return HAL_SUCCESS;
}

hal_err_t hal_delay_us(uint32_t usec) {
  __HAL_TIM_SET_COUNTER(&htim6, 0);
  HAL_TIM_Base_Start(&htim6);
  while(__HAL_TIM_GET_COUNTER(&htim6) < usec) {}
  HAL_TIM_Base_Stop(&htim6);
  return HAL_FAIL;
}

hal_err_t hal_smartcard_start() {
  __HAL_RCC_USART6_CLK_ENABLE();
  vTaskDelay(pdMS_TO_TICKS(CLOCK_STABLE_DELAY));

  HAL_GPIO_WritePin(GPIO_CARD_RST_GPIO_Port, GPIO_CARD_RST_Pin, GPIO_PIN_RESET);
  vTaskDelay(pdMS_TO_TICKS(SC_RESET_DELAY));
  HAL_GPIO_WritePin(GPIO_CARD_RST_GPIO_Port, GPIO_CARD_RST_Pin, GPIO_PIN_SET);

  __HAL_SMARTCARD_FLUSH_DRREGISTER(&hsmartcard6);

  return HAL_SUCCESS;
}

hal_err_t hal_smartcard_stop() {
  HAL_GPIO_WritePin(GPIO_CARD_RST_GPIO_Port, GPIO_CARD_RST_Pin, GPIO_PIN_RESET);

  hsmartcard6.Init.StopBits = SMARTCARD_STOPBITS_1_5;
  hsmartcard6.Init.BaudRate = SC_DEFAULT_BAUD_RATE;
  hsmartcard6.Init.Prescaler = SC_DEFAULT_PSC;
  hsmartcard6.Init.GuardTime = 0;
  hsmartcard6.Init.NACKEnable = SMARTCARD_NACK_ENABLE;
  hsmartcard6.Init.AutoRetryCount = 3;

  hal_err_t err = HAL_SMARTCARD_Init(&hsmartcard6);
  __HAL_RCC_USART6_CLK_DISABLE();
  return err;
}

hal_err_t hal_smartcard_pps(smartcard_protocol_t protocol, uint32_t baud, uint32_t freq, uint8_t guard, uint32_t timeout) {
  hsmartcard6.Init.BaudRate = baud;
  hsmartcard6.Init.Prescaler = (SMARTCARD_CLOCK / freq / 2);

  if (protocol == SC_T1) {
    hsmartcard6.Init.StopBits = SMARTCARD_STOPBITS_1;
    hsmartcard6.Init.NACKEnable = SMARTCARD_NACK_DISABLE;
    hsmartcard6.Init.BlockLength = 255;
    hsmartcard6.Init.AutoRetryCount = 0;
  } else {
    hsmartcard6.Init.TimeOutValue = timeout;
  }

  hsmartcard6.Init.GuardTime = guard;

  return HAL_SMARTCARD_Init(&hsmartcard6);
}

hal_err_t hal_smartcard_set_timeout(uint32_t timeout) {
  HAL_SMARTCARDEx_TimeOut_Config(&hsmartcard6, timeout);
  return HAL_SUCCESS;
}

hal_err_t hal_smartcard_set_blocklen(uint32_t len) {
  HAL_SMARTCARDEx_BlockLength_Config(&hsmartcard6, len);
  return HAL_SUCCESS;
}

hal_err_t hal_smartcard_send(const uint8_t* data, size_t len) {
  configASSERT(g_smartcard_task == NULL);
  g_smartcard_task = xTaskGetCurrentTaskHandle();
  return HAL_SMARTCARD_Transmit_IT(&hsmartcard6, data, len);
}

hal_err_t hal_smarcard_recv(uint8_t* data, size_t len) {
  configASSERT(g_smartcard_task == NULL);
  g_smartcard_task = xTaskGetCurrentTaskHandle();
  __HAL_SMARTCARD_CLEAR_FLAG(&hsmartcard6, SMARTCARD_CLEAR_RTOF);
  __HAL_SMARTCARD_ENABLE_IT(&hsmartcard6, SMARTCARD_IT_RTO);
  return HAL_SMARTCARD_Receive_IT(&hsmartcard6, data, len);
}

void hal_smartcard_abort() {
  HAL_SMARTCARD_Abort_IT(&hsmartcard6);
}

const hal_flash_data_segment_t hal_flash_data_map[] = {
    { .addr = HAL_FLASH_BLOCK_ADDR(80), .count = 48},
    { .addr = HAL_FLASH_BLOCK_ADDR(208), .count = 48},
};

const hal_flash_data_segment_t hal_flash_data_map_swap[] = {
    { .addr = HAL_FLASH_BLOCK_ADDR(208), .count = 48},
    { .addr = HAL_FLASH_BLOCK_ADDR(80), .count = 48},
};

const hal_flash_data_segment_t* hal_flash_get_data_segments() {
  return FLASH_BANK_SWAPPED() ? hal_flash_data_map_swap : hal_flash_data_map ;
}

hal_err_t hal_flash_begin_program() {
  return HAL_FLASH_Unlock();
}

hal_err_t hal_flash_wait_program() {
  HAL_WAIT(hal_flash_busy());
  return HAL_SUCCESS;
}

hal_err_t hal_flash_program(const uint8_t* data, uint8_t* addr, size_t len) {
  size_t write_len = (16 - (((uint32_t) addr) & 0xf)) & 0xf;

  app_err_t err = HAL_SUCCESS;

  if (!write_len) {
    SET_BIT(FLASH_NS->NSCR, FLASH_CR_PG);
  } else {
    memcpy(addr, data, write_len);
    err = hal_flash_wait_program();

    data += write_len;
    addr += write_len;
    len -= write_len;
  }

  write_len = (len & ~0xf);

  if (write_len) {
    memcpy(addr, data, write_len);
    err = hal_flash_wait_program();

    data += write_len;
    addr += write_len;
    len -= write_len;
  }

  if (!len) {
    CLEAR_BIT(FLASH_NS->NSCR, FLASH_CR_PG);
  } else {
    memcpy(addr, data, len);
  }

  return err;
}

hal_err_t hal_flash_erase(uint32_t block) {
  FLASH_EraseInitTypeDef op;
  op.TypeErase = FLASH_TYPEERASE_SECTORS;
  op.NbSectors = 1;
  op.Sector = block;

  if (op.Sector >= FLASH_SECTOR_NB) {
    op.Sector -= FLASH_SECTOR_NB;
    op.Banks = FLASH_BANK_SWAPPED() ? FLASH_BANK_1 : FLASH_BANK_2;
  } else {
    op.Banks = FLASH_BANK_SWAPPED() ? FLASH_BANK_2 : FLASH_BANK_1;
  }

  uint32_t err;
  return HAL_FLASHEx_Erase(&op, &err);
}

hal_err_t hal_flash_end_program() {
  HAL_ICACHE_Invalidate();
  CLEAR_BIT(FLASH_NS->NSCR, FLASH_CR_PG);
  return HAL_FLASH_Lock();
}

void hal_flash_switch_firmware() {
  HAL_FLASH_OB_Unlock();
  FLASH_OBProgramInitTypeDef ob;
  memset(&ob, 0, sizeof(ob));
  HAL_FLASHEx_OBGetConfig(&ob);
  ob.OptionType = OPTIONBYTE_USER;
  ob.USERType = OB_USER_SWAP_BANK;
  ob.USERConfig = (ob.USERConfig & (~FLASH_OPTSR_SWAP_BANK)) | ((~ob.USERConfig) & FLASH_OPTSR_SWAP_BANK);
  HAL_FLASHEx_OBProgram(&ob);
  HAL_FLASH_OB_Launch();
  NVIC_SystemReset();
}

void hal_tick() {
  HAL_IncTick();
}

hal_err_t hal_pwm_set_dutycycle(hal_pwm_output_t out, uint8_t cycle) {
  assert(out == PWM_BACKLIGHT);
  assert(cycle <= 100);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (cycle * 5));
  return HAL_SUCCESS;
}

hal_err_t hal_crc32_init(hal_crc32_ctx_t* ctx) {
  __HAL_CRC_DR_RESET(&hcrc);
  return HAL_SUCCESS;
}

hal_err_t hal_crc32_update(hal_crc32_ctx_t* ctx, uint8_t b) {
  *(__IO uint8_t *)(__IO void *)(&hcrc.Instance->DR) = b;
  return HAL_SUCCESS;
}

hal_err_t hal_crc32_finish(hal_crc32_ctx_t* ctx, uint32_t *out) {
  *out = ~hcrc.Instance->DR;
  return HAL_SUCCESS;
}

hal_err_t hal_sha256_init(hal_sha256_ctx_t* ctx) {
  *ctx = 0;
  CLEAR_BIT(hhash.Instance->CR, HASH_CR_MODE);
  MODIFY_REG(hhash.Instance->CR, HASH_CR_INIT, HASH_CR_INIT);

  return HAL_SUCCESS;
}

hal_err_t hal_sha256_update(hal_sha256_ctx_t* ctx, const uint8_t* data, size_t len) {
  *ctx += len;

  __IO uint32_t data32 = (uint32_t) data;

  for (int i = 0; i < len ; i += 4) {
    hhash.Instance->DIN = *(uint32_t *)data32;
    data32 += 4;
  }

  return HAL_SUCCESS;
}

hal_err_t hal_sha256_finish(hal_sha256_ctx_t* ctx, uint8_t out[SHA256_DIGEST_LENGTH]) {
  HAL_WAIT(__HAL_HASH_GET_FLAG(&hhash, HASH_FLAG_BUSY) == SET);

  MODIFY_REG(hhash.Instance->STR, HASH_STR_NBLW, 8 * (*ctx & 3));
  SET_BIT(hhash.Instance->STR, HASH_STR_DCAL);

  HAL_WAIT(__HAL_HASH_GET_FLAG(&hhash, HASH_FLAG_DCIS) == RESET);

  __IO uint32_t* out32 = (uint32_t *) out;

  *(out32++) = __REV(hhash.Instance->HR[0]);
  *(out32++) = __REV(hhash.Instance->HR[1]);
  *(out32++) = __REV(hhash.Instance->HR[2]);
  *(out32++) = __REV(hhash.Instance->HR[3]);
  *(out32++) = __REV(hhash.Instance->HR[4]);
  *(out32++) = __REV(HASH_DIGEST->HR[5]);
  *(out32++) = __REV(HASH_DIGEST->HR[6]);
  *(out32++) = __REV(HASH_DIGEST->HR[7]);

  return HAL_SUCCESS;
}

static inline void _hal_aes256_load_iv(const uint8_t iv[AES_IV_SIZE]) {
  SAES->IVR3 = (iv[0] << 24) | (iv[1] << 16) | (iv[2] << 8) | iv[3];
  SAES->IVR2 = (iv[4] << 24) | (iv[5] << 16) | (iv[6] << 8) | iv[7];
  SAES->IVR1 = (iv[8] << 24) | (iv[9] << 16) | (iv[10] << 8) | iv[11];
  SAES->IVR0 = (iv[12] << 24) | (iv[13] << 16) | (iv[14] << 8) | iv[15];
}

static inline void _hal_aes256_load_key(const uint8_t key[AES_256_KEY_SIZE]) {
  SAES->KEYR7 = (key[0] << 24) | (key[1] << 16) | (key[2] << 8) | key[3];
  SAES->KEYR6 = (key[4] << 24) | (key[5] << 16) | (key[6] << 8) | key[7];
  SAES->KEYR5 = (key[8] << 24) | (key[9] << 16) | (key[10] << 8) | key[11];
  SAES->KEYR4 = (key[12] << 24) | (key[13] << 16) | (key[14] << 8) | key[15];
  SAES->KEYR3 = (key[16] << 24) | (key[17] << 16) | (key[18] << 8) | key[19];
  SAES->KEYR2 = (key[20] << 24) | (key[21] << 16) | (key[22] << 8) | key[23];
  SAES->KEYR1 = (key[24] << 24) | (key[25] << 16) | (key[26] << 8) | key[27];
  SAES->KEYR0 = (key[28] << 24) | (key[29] << 16) | (key[30] << 8) | key[31];
}

hal_err_t hal_aes256_init(hal_aes_mode_t mode, hal_aes_chaining_t chaining, const uint8_t key[AES_256_KEY_SIZE], const uint8_t iv[AES_IV_SIZE]) {
  assert(chaining == AES_CBC);

  HAL_WAIT((SAES->SR & AES_SR_BUSY));

  uint32_t cr = AES_CR_CHMOD_0 | AES_CR_KEYSIZE | AES_CR_DATATYPE_1;

  if (mode == AES_ENCRYPT) {
    SAES->CR = cr;
  } else {
    SAES->CR = cr | AES_CR_MODE_0;
  }

  _hal_aes256_load_key(key);

  HAL_WAIT(!(SAES->SR & AES_SR_KEYVALID));

  if (mode == AES_DECRYPT) {
    SAES->CR = cr | AES_CR_MODE_0 | AES_CR_EN;

    HAL_WAIT(!(SAES->SR & AES_SR_CCF));

    SAES->ICR = AES_ICR_CCF;

    cr |= AES_CR_MODE_1;
    SAES->CR = cr;
  }

  _hal_aes256_load_iv(iv);
  SAES->CR = cr | AES_CR_EN;

  return HAL_SUCCESS;
}

hal_err_t hal_aes256_block_process(const uint8_t in[AES_BLOCK_SIZE], uint8_t out[AES_BLOCK_SIZE]) {
  __IO uint32_t* in32 = (uint32_t*) in;
  __IO uint32_t* out32 = (uint32_t*) out;

  SAES->DINR = in32[0];
  SAES->DINR = in32[1];
  SAES->DINR = in32[2];
  SAES->DINR = in32[3];

  HAL_WAIT(!(SAES->SR & AES_SR_CCF));

  SAES->ICR = AES_ICR_CCF;

  out32[0] = SAES->DOUTR;
  out32[1] = SAES->DOUTR;
  out32[2] = SAES->DOUTR;
  out32[3] = SAES->DOUTR;

  return HAL_SUCCESS;
}

hal_err_t hal_aes256_finalize() {
  SAES->CR = AES_CR_IPRST;
  HAL_WAIT((SAES->SR & AES_SR_BUSY));
  SAES->CR = 0;

  return HAL_SUCCESS;
}

static hal_err_t _hal_bn_r2(const uint8_t mod[BN_SIZE], uint32_t* r2_mod) {
  PKA_MontgomeryParamInTypeDef mont;
  mont.pOp1 = mod;
  mont.size = BN_SIZE;
  if (HAL_PKA_MontgomeryParam(&hpka, &mont, PKA_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  HAL_PKA_MontgomeryParam_GetResult(&hpka, r2_mod);

  return HAL_OK;
}


hal_err_t hal_ecdsa_sign(const ecdsa_curve* curve, const uint8_t* priv_key, const uint8_t* digest, const uint8_t* k, uint8_t* sig_out) {
  PKA_ECDSASignInTypeDef sign_params;
  sign_params.modulus = curve->prime;
  sign_params.modulusSize = ECC256_ELEMENT_SIZE;
  sign_params.basePointX = curve->G;
  sign_params.basePointY = &curve->G[ECC256_ELEMENT_SIZE];
  sign_params.coef = curve->a;
  sign_params.coefSign = curve->a_sign & 1;
  sign_params.coefB = curve->b;
  sign_params.primeOrder = curve->order;
  sign_params.primeOrderSize = ECC256_ELEMENT_SIZE;
  sign_params.hash = digest;
  sign_params.integer = k;
  sign_params.privateKey = priv_key;

  if (HAL_PKA_ECDSASign(&hpka, &sign_params, PKA_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  PKA_ECDSASignOutTypeDef out;
  out.RSign = sig_out;
  out.SSign = &sig_out[ECC256_ELEMENT_SIZE];

  HAL_PKA_ECDSASign_GetResult(&hpka, &out, NULL);

  return HAL_SUCCESS;
}

hal_err_t hal_ecdsa_verify(const ecdsa_curve* curve, const uint8_t* pub_key, const uint8_t* sig, const uint8_t* digest) {
  PKA_ECDSAVerifInTypeDef verify_params;

  verify_params.modulus = curve->prime;
  verify_params.modulusSize = ECC256_ELEMENT_SIZE;
  verify_params.basePointX = curve->G;
  verify_params.basePointY = &curve->G[ECC256_ELEMENT_SIZE];
  verify_params.coef = curve->a;
  verify_params.coefSign = curve->a_sign & 1;
  verify_params.primeOrder = curve->order;
  verify_params.primeOrderSize = ECC256_ELEMENT_SIZE;
  verify_params.hash = digest;
  verify_params.pPubKeyCurvePtX = pub_key;
  verify_params.pPubKeyCurvePtY = &pub_key[ECC256_ELEMENT_SIZE];
  verify_params.RSign = sig;
  verify_params.RSign = &sig[ECC256_ELEMENT_SIZE];

  if (HAL_PKA_ECDSAVerif(&hpka, &verify_params, PKA_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  return HAL_PKA_ECDSAVerif_IsValidSignature(&hpka) ? HAL_SUCCESS : HAL_FAIL;
}

hal_err_t hal_ec_point_multiply(const ecdsa_curve* curve, const uint8_t* scalar, const uint8_t* point, uint8_t* point_out) {
  PKA_ECCMulInTypeDef mult;

  mult.modulus = curve->prime;
  mult.modulusSize = ECC256_ELEMENT_SIZE;
  mult.coefA = curve->a;
  mult.coefSign = curve->a_sign & 1;
  mult.coefB = curve->b;
  mult.primeOrder = curve->order;
  mult.pointX = point;
  mult.pointY = &point[ECC256_ELEMENT_SIZE];
  mult.scalarMul = scalar;
  mult.scalarMulSize = ECC256_ELEMENT_SIZE;

  if (HAL_PKA_ECCMul(&hpka, &mult, PKA_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  PKA_ECCMulOutTypeDef out;
  out.ptX = point_out;
  out.ptY = &point_out[ECC256_ELEMENT_SIZE];

  HAL_PKA_ECCMul_GetResult(&hpka, &out);

  return HAL_SUCCESS;
}

hal_err_t hal_ec_double_ladder(const ecdsa_curve* curve, const uint8_t* s1, const uint8_t* p1, const uint8_t* s2, const uint8_t* p2, uint8_t* point_out) {
  PKA_ECCDoubleBaseLadderInTypeDef dbl;
  dbl.modulus = curve->prime;
  dbl.modulusSize = ECC256_ELEMENT_SIZE;
  dbl.primeOrderSize = ECC256_ELEMENT_SIZE;
  dbl.coefA = curve->a;
  dbl.coefSign = curve->a_sign & 1;
  dbl.integerK = s1;
  dbl.basePointX1 = p1;
  dbl.basePointY1 = &p1[ECC256_ELEMENT_SIZE];
  dbl.basePointZ1 = BN_ONE;
  dbl.integerM = s2;
  dbl.basePointX2 = p2;
  dbl.basePointY2 = &p2[ECC256_ELEMENT_SIZE];
  dbl.basePointZ2 = BN_ONE;

  if (HAL_PKA_ECCDoubleBaseLadder(&hpka, &dbl, PKA_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  PKA_ECCDoubleBaseLadderOutTypeDef out;
  out.ptX = point_out;
  out.ptY = &point_out[ECC256_ELEMENT_SIZE];

  HAL_PKA_ECCDoubleBaseLadder_GetResult(&hpka, &out);

  return HAL_SUCCESS;
}

hal_err_t hal_ec_point_check(const ecdsa_curve* curve, const uint8_t* point) {
  uint32_t r2_mod[BN_SIZE/4];
  if (_hal_bn_r2(curve->prime, r2_mod) != HAL_SUCCESS) {
    return HAL_FAIL;
  }

  PKA_PointCheckInTypeDef pc;
  pc.modulus = curve->prime;
  pc.modulusSize = ECC256_ELEMENT_SIZE;
  pc.coefA = curve->a;
  pc.coefSign = curve->a_sign & 1;
  pc.coefB = curve->b;
  pc.pointX = point;
  pc.pointY = &point[ECC256_ELEMENT_SIZE];
  pc.pMontgomeryParam = r2_mod;

  if (HAL_PKA_PointCheck(&hpka, &pc, PKA_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  return HAL_PKA_PointCheck_IsOnCurve(&hpka) ? HAL_SUCCESS : HAL_FAIL;
}

static void _hal_pka_ari_set(const uint8_t *pOp1, const uint8_t *pOp2, const uint8_t *pOp3) {
  hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_NB_BITS] = BN_SIZE * 8;

  if (pOp1 != NULL) {
    PKA_Memcpy_u8_to_u32(&hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_IN_OP1], pOp1, BN_SIZE);
    __PKA_RAM_PARAM_END(hpka.Instance->RAM, (PKA_ARITHMETIC_ALL_OPS_IN_OP1 + (BN_SIZE / 4)));
  }

  if (pOp2 != NULL) {
    PKA_Memcpy_u8_to_u32(&hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_IN_OP2], pOp2, BN_SIZE);
    __PKA_RAM_PARAM_END(hpka.Instance->RAM, (PKA_ARITHMETIC_ALL_OPS_IN_OP2 + (BN_SIZE / 4)));
  }

  if (pOp3 != NULL) {
    PKA_Memcpy_u8_to_u32(&hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_IN_OP3], pOp3, BN_SIZE);
    __PKA_RAM_PARAM_END(hpka.Instance->RAM, (PKA_ARITHMETIC_ALL_OPS_IN_OP3 + (BN_SIZE / 4)));
  }
}

static inline hal_err_t _hal_pka_ari_do(uint32_t op, const uint8_t a[BN_SIZE], const uint8_t b[BN_SIZE], const uint8_t mod[BN_SIZE], uint8_t r[BN_SIZE]) {
  _hal_pka_ari_set(a, b, mod);
  if (PKA_Process(&hpka, op, PKA_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  PKA_Memcpy_u32_to_u8(r, &hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_OUT_RESULT], BN_SIZE);

  return HAL_OK;
}

hal_err_t hal_bn_mul_r2(const uint8_t a[BN_SIZE], const uint8_t mod[BN_SIZE], uint8_t r[BN_SIZE]) {
  uint32_t r2_mod[BN_SIZE/4];
  if (_hal_bn_r2(mod, r2_mod) != HAL_SUCCESS) {
    return HAL_FAIL;
  }

  hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_NB_BITS] = BN_SIZE * 8;
  PKA_Memcpy_u8_to_u32(&hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_IN_OP1], a, BN_SIZE);
  __PKA_RAM_PARAM_END(hpka.Instance->RAM, (PKA_ARITHMETIC_ALL_OPS_IN_OP1 + (BN_SIZE / 4)));

  PKA_Memcpy_u32_to_u32(&hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_IN_OP2], r2_mod, (BN_SIZE / 4));
  __PKA_RAM_PARAM_END(hpka.Instance->RAM, (PKA_ARITHMETIC_ALL_OPS_IN_OP2 + (BN_SIZE / 4)));

  PKA_Memcpy_u8_to_u32(&hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_IN_OP3], mod, BN_SIZE);
  __PKA_RAM_PARAM_END(hpka.Instance->RAM, (PKA_ARITHMETIC_ALL_OPS_IN_OP3 + (BN_SIZE / 4)));

  if (PKA_Process(&hpka, PKA_MODE_MONTGOMERY_MUL, PKA_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  PKA_Memcpy_u32_to_u8(r, &hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_OUT_RESULT], BN_SIZE);

  return HAL_SUCCESS;
}

hal_err_t hal_bn_mul_mont(const uint8_t a[BN_SIZE], const uint8_t b[BN_SIZE], const uint8_t mod[BN_SIZE], uint8_t r[BN_SIZE]) {
  return _hal_pka_ari_do(PKA_MODE_MONTGOMERY_MUL, a, b, mod, r);
}

hal_err_t hal_bn_mul_mod(const uint8_t a[BN_SIZE], const uint8_t b[BN_SIZE], const uint8_t mod[BN_SIZE], uint8_t r[BN_SIZE]) {
  uint8_t tmp[BN_SIZE];
  if (hal_bn_mul_r2(a, mod, tmp) != HAL_SUCCESS) {
    return HAL_FAIL;
  }

  return hal_bn_mul_mont(tmp, b, mod, r);
}

hal_err_t hal_bn_add(const uint8_t a[BN_SIZE], const uint8_t b[BN_SIZE], uint8_t r[BN_SIZE]) {
  return _hal_pka_ari_do(PKA_MODE_ARITHMETIC_ADD, a, b, NULL, r);
}

hal_err_t hal_bn_sub(const uint8_t a[BN_SIZE], const uint8_t b[BN_SIZE], uint8_t r[BN_SIZE]) {
  return _hal_pka_ari_do(PKA_MODE_ARITHMETIC_SUB, a, b, NULL, r);
}

hal_err_t hal_bn_mul(const uint8_t a[BN_SIZE], const uint8_t b[BN_SIZE], uint8_t r[BN_SIZE]) {
  return _hal_pka_ari_do(PKA_MODE_ARITHMETIC_MUL, a, b, NULL, r);
}

hal_err_t hal_bn_add_mod(const uint8_t a[BN_SIZE], const uint8_t b[BN_SIZE], const uint8_t mod[BN_SIZE], uint8_t r[BN_SIZE]) {
  return _hal_pka_ari_do(PKA_MODE_MODULAR_ADD, a, b, mod, r);
}

hal_err_t hal_bn_sub_mod(const uint8_t a[BN_SIZE], const uint8_t b[BN_SIZE], const uint8_t mod[BN_SIZE], uint8_t r[BN_SIZE]) {
  return _hal_pka_ari_do(PKA_MODE_MODULAR_SUB, a, b, mod, r);
}

hal_err_t hal_bn_exp_mod(const uint8_t a[BN_SIZE], const uint8_t e[BN_SIZE], const uint8_t mod[BN_SIZE], uint8_t r[BN_SIZE]) {
  PKA_ModExpInTypeDef exp;

  exp.OpSize = BN_SIZE;
  exp.expSize = BN_SIZE;
  exp.pOp1 = a;
  exp.pExp = e;
  exp.pMod = mod;

  if (HAL_PKA_ModExp(&hpka, &exp, PKA_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  HAL_PKA_ModExp_GetResult(&hpka, r);

  return HAL_SUCCESS;
}

hal_err_t hal_bn_inv_mod(const uint8_t a[BN_SIZE], const uint8_t mod[BN_SIZE], uint8_t r[BN_SIZE]) {
  PKA_Memcpy_u8_to_u32(&hpka.Instance->RAM[PKA_MODULAR_INV_IN_OP1], a, BN_SIZE);
  __PKA_RAM_PARAM_END(hpka.Instance->RAM, PKA_MODULAR_INV_IN_OP1 + (BN_SIZE / 4));

  PKA_Memcpy_u8_to_u32(&hpka.Instance->RAM[PKA_MODULAR_INV_IN_OP2_MOD], mod, BN_SIZE);
  __PKA_RAM_PARAM_END(hpka.Instance->RAM, PKA_MODULAR_INV_IN_OP2_MOD + (BN_SIZE / 4));

  if (PKA_Process(&hpka, PKA_MODE_MODULAR_INV, PKA_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  PKA_Memcpy_u32_to_u8(r, &hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_OUT_RESULT], BN_SIZE);

  return HAL_OK;
}

int hal_bn_cmp(const uint8_t a[BN_SIZE], const uint8_t b[BN_SIZE]) {
  _hal_pka_ari_set(a, b, NULL);
  if (PKA_Process(&hpka, PKA_MODE_COMPARISON, PKA_TIMEOUT) != HAL_OK) {
    return INT32_MAX;
  }

  uint32_t res = hpka.Instance->RAM[PKA_ARITHMETIC_ALL_OPS_OUT_RESULT];

  switch(res) {
  case 0x916a:
    return -1;
  case 0xed2c:
    return 0;
  case 0x7af8:
    return 1;
  default:
    return INT32_MAX;
  }
}

hal_err_t hal_usb_start() {
  HAL_PWREx_EnableVddUSB();

  MX_USB_PCD_Init();
  HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x00, PCD_SNG_BUF, 0x18);
  HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x80, PCD_SNG_BUF, 0x58);
  HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, HAL_USB_EPIN_ADDR, PCD_SNG_BUF, 0x98);
  HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, HAL_USB_EPOUT_ADDR, PCD_SNG_BUF, 0xD8);

  USB_DevConnect(hpcd_USB_DRD_FS.Instance);

  hpcd_USB_DRD_FS.Instance->ISTR = 0U;
  hpcd_USB_DRD_FS.Instance->CNTR = USB_CNTR_CTRM  | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_ERRM | USB_CNTR_RESETM | USB_CNTR_L1REQM;

  return HAL_OK;
}

hal_err_t hal_usb_stop() {
  if (READ_BIT(PWR->USBSCR, PWR_USBSCR_USB33SV)) {
    _hal_usb_close_ep();
    HAL_PCD_Stop(&hpcd_USB_DRD_FS);
    HAL_PCD_DeInit(&hpcd_USB_DRD_FS);
    HAL_PWREx_DisableVddUSB();
  }

  return HAL_OK;
}

hal_err_t hal_usb_send(uint8_t epaddr, const uint8_t* data, size_t len) {
  return HAL_PCD_EP_Transmit(&hpcd_USB_DRD_FS, epaddr, (uint8_t *) data, len);
}

hal_err_t hal_usb_set_stall(uint8_t epaddr, uint8_t stall) {
  if (stall) {
    return HAL_PCD_EP_SetStall(&hpcd_USB_DRD_FS, epaddr);
  } else {
    return HAL_PCD_EP_ClrStall(&hpcd_USB_DRD_FS, epaddr);
  }
}

uint8_t hal_usb_get_stall(uint8_t epaddr) {
  if (epaddr & 0x80) {
    return hpcd_USB_DRD_FS.IN_ep[epaddr & 0x7].is_stall;
  } else {
    return hpcd_USB_DRD_FS.OUT_ep[epaddr & 0x7].is_stall;
  }
}

hal_err_t hal_usb_set_address(uint8_t addr) {
  return HAL_PCD_SetAddress(&hpcd_USB_DRD_FS, addr);
}

hal_err_t hal_usb_next_recv(uint8_t epaddr, uint8_t* data, size_t len) {
  return HAL_PCD_EP_Receive(&hpcd_USB_DRD_FS, epaddr, data, len);
}

void hal_inactivity_timer_set(uint32_t delay_ms) {
  HAL_TIM_Base_Stop_IT(&htim5);
  __HAL_TIM_SET_COUNTER(&htim5, 0);
  __HAL_TIM_SET_AUTORELOAD(&htim5, delay_ms * 4);
  __HAL_TIM_CLEAR_IT(&htim5, TIM_IT_UPDATE);
  HAL_TIM_Base_Start_IT(&htim5);
}

void hal_inactivity_timer_reset() {
  __HAL_TIM_SET_COUNTER(&htim5, 0);
}

hal_err_t hal_adc_read(hal_adc_channel_t ch, uint32_t* val) {
  assert(ch == ADC_VBAT);

  MX_ADC2_Init();
  HAL_ADCEx_Calibration_SetValue(&hadc2, ADC_SINGLE_ENDED, g_adc_calibration);
  HAL_ADC_Start(&hadc2);

  if (HAL_ADC_PollForConversion(&hadc2, HAL_TIMEOUT) != HAL_OK) {
    return HAL_FAIL;
  }

  *val = VBAT_OFFSET + (((HAL_ADC_GetValue(&hadc2) * g_adc_vref)) / ANALOG_DIV);

  HAL_ADC_Stop(&hadc2);
  HAL_ADCEx_EnterADCDeepPowerDownMode(&hadc2);

  return HAL_SUCCESS;
}
