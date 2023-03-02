#include "FreeRTOS.h"
#include "task.h"
#include "qrcode/qrcode.h"

//TODO: remove begin
#include "camera_support.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
//TODO: remove end

#define APP_CAMERA_BPP 1
#define APP_CAMERA_FRAME_BUFFER_COUNT 2

AT_NONCACHEABLE_SECTION_ALIGN(
    static uint8_t s_cameraBuffer[APP_CAMERA_FRAME_BUFFER_COUNT][DEMO_CAMERA_HEIGHT*DEMO_CAMERA_WIDTH*APP_CAMERA_BPP],
    DEMO_CAMERA_BUFFER_ALIGN);

static struct quirc_code qrcode;
static struct quirc_data qrdata;

static void APP_InitCamera(void) {
    const camera_config_t cameraConfig = {
        .pixelFormat                = kVIDEO_PixelFormatRAW8,
        .bytesPerPixel              = APP_CAMERA_BPP,
        .resolution                 = FSL_VIDEO_RESOLUTION(DEMO_CAMERA_WIDTH, DEMO_CAMERA_HEIGHT),
        .frameBufferLinePitch_Bytes = DEMO_CAMERA_WIDTH * APP_CAMERA_BPP,
        .interface                  = kCAMERA_InterfaceGatedClock,
        .controlFlags               = DEMO_CAMERA_CONTROL_FLAGS,
        .framePerSec                = 30,
    };

    memset(s_cameraBuffer, 0, sizeof(s_cameraBuffer));

    BOARD_InitCameraResource();

    CAMERA_RECEIVER_Init(&cameraReceiver, &cameraConfig, NULL, NULL);

    if (kStatus_Success != CAMERA_DEVICE_Init(&cameraDevice, &cameraConfig)) {
        vTaskSuspend(NULL);
    }

    CAMERA_DEVICE_Start(&cameraDevice);

    /* Submit the empty frame buffers to buffer queue. */
    for (uint32_t i = 0; i < APP_CAMERA_FRAME_BUFFER_COUNT; i++)
    {
        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)(s_cameraBuffer[i]));
    }
}

void qrscan_task_entry(void* pvParameters) {
  PRINTF("Starting QR\r\n");

  APP_InitCamera();

  uint32_t cameraReceivedFrameAddr;
  struct quirc *qr;

  qr = quirc_new();
  if (!qr) {
      PRINTF("Failed to allocate memory\r\n");
      return;
  }


  CAMERA_RECEIVER_Start(&cameraReceiver);

  while (1)
  {

      while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &cameraReceivedFrameAddr))
      {
      }

      quirc_set_image(qr, (uint8_t*)cameraReceivedFrameAddr, DEMO_CAMERA_WIDTH, DEMO_CAMERA_HEIGHT);

      quirc_begin(qr, NULL, NULL);

      quirc_end(qr);
      CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)cameraReceivedFrameAddr);

      int num_codes;
      int i;

      num_codes = quirc_count(qr);
      for (i = 0; i < num_codes; i++) {
          quirc_decode_error_t err;

          quirc_extract(qr, i, &qrcode);

          err = quirc_decode(&qrcode, &qrdata);
          if (err)
              PRINTF("DECODE FAILED: %s\r\n", quirc_strerror(err));
          else
              PRINTF("Data: %s\r\n", qrdata.payload);
      }
  }

  vTaskSuspend(NULL);
}
