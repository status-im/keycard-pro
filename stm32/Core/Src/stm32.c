#include <string.h>

#include "main.h"
#include "hal.h"
#include "linked_list.h"
#include "FreeRTOS.h"
#include "task.h"

#define HAL_TIMEOUT 250
#define SC_RESET_DELAY 37
#define SMARTCARD_STOPBITS_1 0x00000000U

extern DMA_QListTypeDef Camera_DMA_LL;
extern DMA_NodeTypeDef Camera_DMA_Node1;
extern DMA_NodeTypeDef Camera_DMA_Node2;

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
  {GPIO_CAMERA_RST_GPIO_Port, GPIO_CAMERA_RST_Pin},
  {GPIO_LCD_CD_GPIO_Port, GPIO_LCD_CD_Pin},
  {GPIO_LCD_RST_GPIO_Port, GPIO_LCD_RST_Pin},
};

static void (*g_spi_callback)();
static TaskHandle_t g_dcmi_task = NULL;
static TaskHandle_t g_smartcard_task = NULL;
static int8_t g_acquiring;
static struct dcmi_buf g_dcmi_bufs[CAMERA_FB_COUNT];

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

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (g_spi_callback) {
    g_spi_callback();
  }
}

static void hal_smartcard_complete(uint32_t err) {
  configASSERT(g_smartcard_task);

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
  configASSERT(g_dcmi_task);

  hdcmi->Instance->CR &= ~(DCMI_CR_CAPTURE);

  g_dcmi_bufs[g_acquiring].status = DCMI_ACQUIRED;
  g_acquiring = -1;

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

hal_err_t hal_init() {
  HAL_Init();
  SystemClock_Config();

  MX_RNG_Init();
  MX_HASH_Init();

  MX_GPIO_Init();
  MX_TIM6_Init();
  MX_GPDMA2_Init();
  MX_GPDMA1_Init();

  MX_SPI5_Init();
  MX_I2C2_Init();
  MX_USART6_SMARTCARD_Init();
  MX_USART3_UART_Init();
  MX_DCMI_Init();

  mco_off();
  __HAL_DCMI_DISABLE_IT(&hdcmi, DCMI_IT_LINE | DCMI_IT_VSYNC);

  MX_Camera_DMA_LL_Config();

  __HAL_LINKDMA(&hdcmi, DMA_Handle, handle_GPDMA2_Channel5);
  HAL_DMAEx_List_LinkQ(&handle_GPDMA2_Channel5, &Camera_DMA_LL);

  MX_ICACHE_Init();

  return HAL_SUCCESS;
}

hal_err_t hal_camera_init() {
  mco_on();
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

hal_err_t hal_gpio_set(hal_gpio_pin_t pin, hal_gpio_state_t state) {
  if (STM32_PIN_MAP[pin].base == NULL) {
    return HAL_SUCCESS; // unconnected PIN
  }

  HAL_GPIO_WritePin(STM32_PIN_MAP[pin].base, STM32_PIN_MAP[pin].pin, state);
  return HAL_SUCCESS;
}

hal_err_t hal_i2c_send(hal_i2c_port_t port, uint8_t addr, const uint8_t* data, size_t len) {
  assert(port == I2C_CAMERA);
  return HAL_I2C_Master_Transmit(&hi2c2, (addr << 1), (uint8_t*) data, len, HAL_TIMEOUT);
}

hal_err_t hal_uart_send(hal_uart_port_t port, const uint8_t* data, size_t len) {
  assert(port == UART_LOG);
  return HAL_UART_Transmit(&huart3, (uint8_t*) data, len, HAL_TIMEOUT);
}

hal_err_t hal_spi_send(hal_spi_port_t port, const uint8_t* data, size_t len) {
  assert(port == SPI_LCD);
  return HAL_SPI_Transmit(&hspi5, (uint8_t*) data, len, HAL_TIMEOUT);
}

hal_err_t hal_spi_send_dma(hal_spi_port_t port, const uint8_t* data, size_t len, void (*cb)()) {
  assert(port == SPI_LCD);
  g_spi_callback = cb;
  return HAL_SPI_Transmit_DMA(&hspi5, (uint8_t*) data, len);
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

/*
hal_err_t hal_sha256_init(hal_sha256_ctx_t* ctx) {
  return HAL_FAIL;
}

hal_err_t hal_sha256_update(hal_sha256_ctx_t* ctx, const uint8_t* data, size_t len) {
  return HAL_FAIL;
}

hal_err_t hal_sha256_finish(hal_sha256_ctx_t* ctx, uint8_t out[SHA256_DIGEST_LENGTH]) {
  return HAL_FAIL;
}
*/

hal_err_t hal_delay_us(uint32_t usec) {
  __HAL_TIM_SET_COUNTER(&htim6, 0);
  HAL_TIM_Base_Start(&htim6);
  while(__HAL_TIM_GET_COUNTER(&htim6) < usec) {}
  HAL_TIM_Base_Stop(&htim6);
  return HAL_FAIL;
}

hal_err_t hal_smartcard_start() {
  HAL_GPIO_WritePin(GPIO_CARD_ON_GPIO_Port, GPIO_CARD_ON_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIO_CARD_RST_GPIO_Port, GPIO_CARD_RST_Pin, GPIO_PIN_RESET);

  vTaskDelay(pdMS_TO_TICKS(SC_RESET_DELAY));
  HAL_GPIO_WritePin(GPIO_CARD_ON_GPIO_Port, GPIO_CARD_ON_Pin, GPIO_PIN_RESET);
  vTaskDelay(pdMS_TO_TICKS(SC_RESET_DELAY));
  HAL_GPIO_WritePin(GPIO_CARD_RST_GPIO_Port, GPIO_CARD_RST_Pin, GPIO_PIN_SET);
  __HAL_SMARTCARD_FLUSH_DRREGISTER(&hsmartcard6);
  return HAL_SUCCESS;
}

hal_err_t hal_smartcard_stop() {
  HAL_GPIO_WritePin(GPIO_CARD_ON_GPIO_Port, GPIO_CARD_ON_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIO_CARD_RST_GPIO_Port, GPIO_CARD_RST_Pin, GPIO_PIN_SET);

  hsmartcard6.Init.StopBits = SMARTCARD_STOPBITS_1_5;
  hsmartcard6.Init.BaudRate = SC_DEFAULT_BAUD_RATE;
  hsmartcard6.Init.Prescaler = SC_DEFAULT_PSC;
  hsmartcard6.Init.GuardTime = 0;
  hsmartcard6.Init.NACKEnable = SMARTCARD_NACK_ENABLE;
  hsmartcard6.Init.AutoRetryCount = 3;

  return HAL_SMARTCARD_Init(&hsmartcard6);
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
  __HAL_SMARTCARD_ENABLE_IT(&hsmartcard6, SMARTCARD_IT_RTO);
  return HAL_SMARTCARD_Receive_IT(&hsmartcard6, data, len);
}

hal_err_t hal_flash_begin_program() {
  return HAL_FLASH_Unlock();
}

hal_err_t hal_flash_program(const uint32_t* data, uint32_t* addr) {
  return HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, (uint32_t) addr, (uint32_t) data);
}

hal_err_t hal_flash_erase(uint32_t block) {
  FLASH_EraseInitTypeDef op;
  op.TypeErase = FLASH_TYPEERASE_SECTORS;
  op.NbSectors = 1;
  op.Sector = block;

  if (op.Sector >= FLASH_SECTOR_NB) {
    op.Sector -= FLASH_SECTOR_NB;
    op.Banks = FLASH_BANK_2;
  } else {
    op.Banks = FLASH_BANK_1;
  }

  uint32_t err;
  return HAL_FLASHEx_Erase(&op, &err);
}

hal_err_t hal_flash_end_program() {
  return HAL_FLASH_Lock();
}

void vApplicationTickHook(void) {
  HAL_IncTick();
}
