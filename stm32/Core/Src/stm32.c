#include <string.h>

#include "main.h"
#include "hal.h"
#include "linked_list.h"
#include "FreeRTOS.h"
#include "task.h"

extern DMA_QListTypeDef Camera_DMA_LL;

#define HAL_TIMEOUT 250

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

static hal_err_t inline _hal_acquire(int8_t idx) {
  HAL_DCMI_Stop(&hdcmi);
  g_acquiring = idx;
  g_dcmi_bufs[idx].status = DCMI_ACQUIRING;
  hal_err_t err = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t) g_dcmi_bufs[idx].addr, (CAMERA_FB_SIZE/4));

  if (err != HAL_SUCCESS) {
    g_acquiring = -1;
    g_dcmi_bufs[idx].status = DCMI_READY;
  }

  return err;
}

void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  configASSERT(g_dcmi_task);
  g_dcmi_bufs[g_acquiring].status = DCMI_ACQUIRED;
  g_acquiring = -1;

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
  MX_USART2_SMARTCARD_Init();
  MX_USART3_UART_Init();
  MX_DCMI_Init();

  mco_off();

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

  for (int i = 0; i < CAMERA_FB_COUNT; i++) {
    g_dcmi_bufs[i].status = DCMI_READY;
    g_dcmi_bufs[i].addr = fb[i];
  }

  return _hal_acquire(0);
}

hal_err_t hal_camera_stop() {
  g_dcmi_task = NULL;
  HAL_DCMI_Stop(&hdcmi);
  mco_off();
  return HAL_SUCCESS;
}

hal_err_t hal_camera_next_frame(uint8_t** fb) {
  hal_err_t err = HAL_FAIL;
  for (int i = 0; i < CAMERA_FB_COUNT; i++) {
    if ((err == HAL_FAIL) && (g_dcmi_bufs[i].status == DCMI_ACQUIRED)) {
      g_dcmi_bufs[i].status = DCMI_PROCESSING;
      *fb = g_dcmi_bufs[i].addr;
      err = HAL_SUCCESS;
    } else if ((g_acquiring == -1) && (g_dcmi_bufs[i].status == DCMI_READY)) {
      _hal_acquire(i);
    }
  }

  return err;
}

hal_err_t hal_camera_submit(uint8_t* fb) {
  for (int i = 0; i < CAMERA_FB_COUNT; i++) {
    if (g_dcmi_bufs[i].addr == fb) {
      if (g_acquiring == -1) {
        return _hal_acquire(i);
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

void vApplicationTickHook(void) {
  HAL_IncTick();
}
