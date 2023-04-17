#include "log/log.h"
#include "camera/camera.h"
#include "qrcode.h"
#include "ur/ur.h"
#include "ur/eip4527_decode.h"
#include "screen/screen.h"
#include "ui/theme.h"
#include "ui/ui_internal.h"
#include "error.h"

static struct quirc_code qrcode;
static struct quirc_data qrdata;
static struct quirc qr;
static ur_t ur;

app_err_t qrscan_scan() {
  app_err_t res = ERR_OK;

  screen_fill_area(&screen_fullarea, TH_COLOR_QR_BG);

  if (camera_start() != HAL_OK) {
    res = ERR_HW;
    goto end;
  }

  uint8_t* fb;

  // prime the buffers
  for(int i = 0; i < 2; i++) {
    if (camera_next_frame(&fb) != HAL_OK) {
      continue;
    }

    if (camera_submit(fb) != HAL_OK) {
      res = ERR_HW;
      goto end;
    }
  }

  while (1) {
    if (camera_next_frame(&fb) != HAL_OK) {
      LOG_MSG("Failed to acquire frame");
      continue;
    }

#ifdef APP_DEBUG_NO_SCREEN
    LOG(LOG_IMG, fb, CAMERA_FB_SIZE);
#endif

    quirc_set_image(&qr, fb, CAMERA_WIDTH, CAMERA_HEIGHT);
    quirc_begin(&qr, NULL, NULL);

    quirc_threshold(&qr);
    screen_camera_passthrough(fb);

    quirc_end(&qr);

    int num_codes = quirc_count(&qr);
    for (int i = 0; i < num_codes; i++) {
      quirc_decode_error_t err;

      quirc_extract(&qr, i, &qrcode);

      err = quirc_decode(&qrcode, &qrdata);
      if (!err) {
        LOG(LOG_MSG, qrdata.payload, qrdata.payload_len);
        if (ur_process_part(&ur, qrdata.payload, qrdata.payload_len) == ERR_OK) {
          if (ur.type == ETH_SIGN_REQUEST) {
            cbor_decode_eth_sign_request(ur.data, ur.data_len, g_ui_cmd.params.qrscan.out, NULL);
            screen_wait();
            goto end;
          } else {
            LOG_MSG("Unsupported UR type");
          }
        } else {
          LOG_MSG("Failed decoding UR data");
        }
      } else {
        LOG_MSG("Failed decoding QR Code");
      }
    }

    screen_wait();

    if (camera_submit(fb) != HAL_OK) {
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
