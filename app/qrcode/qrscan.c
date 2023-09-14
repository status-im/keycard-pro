#include "common.h"
#include "error.h"
#include "mem.h"
#include "qrcode.h"
#include "camera/camera.h"
#include "crypto/crc32.h"
#include "screen/screen.h"
#include "ui/theme.h"
#include "ui/ui_internal.h"
#include "ur/ur.h"
#include "ur/eip4527_decode.h"
#include "ur/auth_decode.h"

app_err_t qrscan_decode(struct quirc *qrctx, ur_t* ur) {
  struct quirc_code qrcode;
  struct quirc_data *qrdata = (struct quirc_data *)qrctx;

  if (quirc_count(qrctx) != 1) {
    return ERR_RETRY;
  }

  quirc_extract(qrctx, 0, &qrcode);
  quirc_decode_error_t err = quirc_decode(&qrcode, qrdata);

  return !err ? ur_process_part(ur, qrdata->payload, qrdata->payload_len) : ERR_RETRY;
}

app_err_t qrscan_deserialize(ur_t* ur) {
  if (ur->type != g_ui_cmd.params.qrscan.type) {
    return ERR_DATA;
  }

  if ((ur->crc != 0) && (crc32(ur->data, ur->data_len) != ur->crc)) {
    return ERR_DATA;
  }

  app_err_t err;
  data_t* data;

  switch(ur->type) {
  case ETH_SIGN_REQUEST:
    err = cbor_decode_eth_sign_request(ur->data, ur->data_len, g_ui_cmd.params.qrscan.out, NULL) == ZCBOR_SUCCESS ? ERR_OK : ERR_DATA;
    break;
  case FS_DATA:
    data = g_ui_cmd.params.qrscan.out;
    data->data = ur->data;
    data->len = ur->data_len;
    err = ERR_OK;
    break;
  case DEV_AUTH:
    err = cbor_decode_dev_auth(ur->data, ur->data_len, g_ui_cmd.params.qrscan.out, NULL) == ZCBOR_SUCCESS ? ERR_OK : ERR_DATA;
    break;
  default:
    err = ERR_DATA;
    break;
  }

  return err;
}

app_err_t qrscan_scan() {
  struct quirc qrctx;
  app_err_t res = ERR_OK;
  ur_t ur;
  ur.data_max_len = MEM_HEAP_SIZE;
  ur.data = g_mem_heap;
  ur.crc = 0;

  screen_fill_area(&screen_fullarea, TH_COLOR_QR_BG);

  if (camera_start() != HAL_SUCCESS) {
    res = ERR_HW;
    goto end;
  }

  uint8_t* fb;

  while (1) {
    if (camera_next_frame(&fb) != HAL_SUCCESS) {
      continue;
    }

    quirc_set_image(&qrctx, fb, CAMERA_WIDTH, CAMERA_HEIGHT);
    quirc_begin(&qrctx, NULL, NULL);

    quirc_threshold(&qrctx);
    screen_camera_passthrough(fb);

    quirc_end(&qrctx);

    if (qrscan_decode(&qrctx, &ur) == ERR_OK) {
      hal_inactivity_timer_reset();
      if (qrscan_deserialize(&ur) == ERR_OK) {
        screen_wait();
        goto end;
      } else {
        ur.crc = 0;
      }
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
