#include "FreeRTOS.h"
#include "task.h"
#include "common.h"
#include "hal.h"
#include "qrcode/qrcode.h"

APP_NOCACHE(static uint8_t fb[CAMERA_FB_COUNT][CAMERA_FB_SIZE], CAMERA_BUFFER_ALIGN);

static struct quirc_code qrcode;
static struct quirc_data qrdata;
static struct quirc qr;

void qrscan_task_entry(void* pvParameters) {
  APP_PRINTF("\r\nStarting QR\r\n");

  if (hal_camera_init(fb) != HAL_OK) {
    APP_PRINTF("\r\nFailed to init camera\r\n");
    goto fail;
  }

  memset(&qr, 0, sizeof(struct quirc));

  while (1) {
    uint8_t* fb;
    hal_camera_next_frame(&fb);

    quirc_set_image(&qr, fb, CAMERA_WIDTH, CAMERA_HEIGHT);
    quirc_begin(&qr, NULL, NULL);

    quirc_end(&qr);
    hal_camera_submit(fb);

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
