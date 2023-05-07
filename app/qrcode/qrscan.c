#include "log/log.h"
#include "camera/camera.h"
#include "qrcode.h"
#include "ur/ur.h"
#include "ur/eip4527_decode.h"
#include "screen/screen.h"
#include "ui/theme.h"
#include "ui/ui_internal.h"
#include "error.h"

app_err_t qrscan_decode(struct quirc *qrctx, ur_t* ur) {
  struct quirc_code qrcode;
  struct quirc_data qrdata;

  if (quirc_count(qrctx) != 1) {
    return ERR_RETRY;
  }

  quirc_extract(qrctx, 0, &qrcode);
  quirc_decode_error_t err = quirc_decode(&qrcode, &qrdata);

  return !err ? ur_process_part(ur, qrdata.payload, qrdata.payload_len) : ERR_RETRY;
}

app_err_t qrscan_scan() {
  struct quirc qrctx;
  app_err_t res = ERR_OK;
  ur_t ur;
  ur.data_max_len = sizeof(struct quirc);
  ur.data = (uint8_t*) &qrctx;

  screen_fill_area(&screen_fullarea, TH_COLOR_QR_BG);

  LOG_MSG("Starting QR scanner");

  if (camera_start() != HAL_SUCCESS) {
    LOG_MSG("Failed to start camera");
    res = ERR_HW;
    goto end;
  }

  uint8_t* fb;

  // prime the buffers
  for(int i = 0; i < 2; i++) {
    if (camera_next_frame(&fb) != HAL_SUCCESS) {
      continue;
    }

    if (camera_submit(fb) != HAL_SUCCESS) {
      res = ERR_HW;
      goto end;
    }
  }

  while (1) {
    if (camera_next_frame(&fb) != HAL_SUCCESS) {
      LOG_MSG("Failed to acquire frame");
      continue;
    }

#ifdef APP_DEBUG_NO_SCREEN
    LOG(LOG_IMG, fb, CAMERA_FB_SIZE);
#endif

    quirc_set_image(&qrctx, fb, CAMERA_WIDTH, CAMERA_HEIGHT);
    quirc_begin(&qrctx, NULL, NULL);

    quirc_threshold(&qrctx);
    screen_camera_passthrough(fb);

    quirc_end(&qrctx);

    if (qrscan_decode(&qrctx, &ur) == ERR_OK && ur.type == ETH_SIGN_REQUEST) {
      cbor_decode_eth_sign_request(ur.data, ur.data_len, g_ui_cmd.params.qrscan.out, NULL);
      screen_wait();
      goto end;
    }

    screen_wait();

    if (camera_submit(fb) != HAL_SUCCESS) {
      res = ERR_HW;
      goto end;
    }

    keypad_key_t k = ui_wait_keypress(0);

    if ((k == KEYPAD_KEY_CANCEL) || (k == KEYPAD_KEY_BACK)) {
      res = ERR_CANCEL;
      goto end;
    }
  }

end:
  camera_stop();
  return res;
}
