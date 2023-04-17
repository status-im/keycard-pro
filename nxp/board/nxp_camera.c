#include <nxp_camera.h>
#include "fsl_gpio.h"
#include "fsl_csi.h"
#include "fsl_iomuxc.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"

static csi_handle_t g_csi_handle;
static TaskHandle_t g_csi_task = NULL;

static void __hal_frame_done_cb(CSI_Type *base, csi_handle_t *handle, status_t status, void *user_data) {
  configASSERT(g_csi_task);

  if (status != kStatus_CSI_FrameDone) {
    return;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveIndexedFromISR(g_csi_task, CAMERA_TASK_NOTIFICATION_IDX, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

hal_err_t hal_camera_init() {
  NVIC_SetPriority(CSI_IRQn, 2);

  csi_config_t cfg;
  CSI_GetDefaultConfig(&cfg);

  cfg.bytesPerPixel = CAMERA_BPP;
  cfg.linePitch_Bytes = CAMERA_WIDTH * CAMERA_BPP;
  cfg.polarityFlags = kCSI_HsyncActiveHigh | kCSI_DataLatchOnRisingEdge;
  cfg.workMode = kCSI_GatedClockMode;
  cfg.dataBus = kCSI_DataBus8Bit;
  cfg.useExtVsync = 1;
  cfg.width = CAMERA_WIDTH;
  cfg.height = CAMERA_HEIGHT;

  return CSI_Init(CSI, &cfg) == kStatus_Success ? HAL_OK : HAL_ERROR;
}

hal_err_t hal_camera_start(uint8_t fb[CAMERA_FB_COUNT][CAMERA_FB_SIZE]) {
  configASSERT(g_csi_task == NULL);
  g_csi_task = xTaskGetCurrentTaskHandle();

  if (CSI_TransferCreateHandle(CSI, &g_csi_handle, __hal_frame_done_cb, NULL) != kStatus_Success) {
    return HAL_ERROR;
  }

  for(int i = 0; i < CAMERA_FB_COUNT; i++) {
    CSI_TransferSubmitEmptyBuffer(CSI, &g_csi_handle, (uint32_t)fb[i]);
  }

  return CSI_TransferStart(CSI, &g_csi_handle) == kStatus_Success ? HAL_OK : HAL_ERROR;
}

hal_err_t hal_camera_stop() {
  CSI_TransferStop(CSI, &g_csi_handle);
  CSI_Deinit(CSI);
  g_csi_task = NULL;
  return HAL_OK;
}


hal_err_t hal_camera_next_frame(uint8_t** fb) {
  return CSI_TransferGetFullBuffer(CSI, &g_csi_handle, (uint32_t*) fb) == kStatus_Success ? HAL_OK : HAL_ERROR;
}

hal_err_t hal_camera_submit(uint8_t* fb) {
  return CSI_TransferSubmitEmptyBuffer(CSI, &g_csi_handle, (uint32_t) fb) == kStatus_Success ? HAL_OK : HAL_ERROR;
}
