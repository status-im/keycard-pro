#include "FreeRTOS.h"
#include "task.h"
#include "common.h"
#include "camera/camera.h"
#include "qrcode/qrcode.h"

static struct quirc_code qrcode;
static struct quirc_data qrdata;
static struct quirc qr;

void qrscan_task_entry(void* pvParameters) {
  APP_PRINTF("\r\nStarting QR\r\n");

  if (camera_start() != HAL_OK) {
    APP_PRINTF("\r\nFailed to init camera\r\n");
    goto fail;
  }

  while (1) {
    uint8_t* fb;
    if (camera_next_frame(&fb) != HAL_OK) {
      APP_PRINTF("\r\nFailed to acquire frame\r\n");
    }

    quirc_set_image(&qr, fb, CAMERA_WIDTH, CAMERA_HEIGHT);
    quirc_begin(&qr, NULL, NULL);

    quirc_end(&qr);

    if (camera_submit(fb) != HAL_OK) {
      APP_PRINTF("\r\nFailed to enqueue framebuffer\r\n");
    }

    int num_codes = quirc_count(&qr);
    for (int i = 0; i < num_codes; i++) {
      quirc_decode_error_t err;

      quirc_extract(&qr, i, &qrcode);

      err = quirc_decode(&qrcode, &qrdata);
      if (err) {
          APP_PRINTF("\r\nDECODE FAILED: %s\r\n", quirc_strerror(err));
      } else {
          APP_PRINTF("\r\nData: %s\r\n", qrdata.payload);
      }
    }
  }

fail:
  vTaskSuspend(NULL);
}
