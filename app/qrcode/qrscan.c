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

const static screen_area_t indicator_area = { .x = 10, .y = 10, .width = 10, .height = 10 };

#define QR_SCORE_RED 1
#define QR_SCORE_YELLOW 3
#define QR_SCORE_GREEN 5

app_err_t qrscan_decode(struct quirc *qrctx, ur_t* ur) {
  struct quirc_code qrcode;
  struct quirc_data *qrdata = (struct quirc_data *)qrctx;

  if (quirc_count(qrctx) != 1) {
    return ERR_SCAN;
  }

  quirc_extract(qrctx, 0, &qrcode);
  quirc_decode_error_t err = quirc_decode(&qrcode, qrdata);

  return !err ? ur_process_part(ur, qrdata->payload, qrdata->payload_len) : ERR_DECODE;
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
  uint16_t score = QR_SCORE_RED;
  uint16_t prev_color = 0;

  while (1) {
    if (camera_next_frame(&fb) != HAL_SUCCESS) {
      continue;
    }

    quirc_set_image(&qrctx, fb, CAMERA_WIDTH, CAMERA_HEIGHT);
    quirc_begin(&qrctx, NULL, NULL);

    quirc_threshold(&qrctx);
    screen_camera_passthrough(fb);

    quirc_end(&qrctx);

    app_err_t qrerr = qrscan_decode(&qrctx, &ur);
    score--;

    if (qrerr == ERR_OK) {
      score = QR_SCORE_GREEN;
      hal_inactivity_timer_reset();
      if (qrscan_deserialize(&ur) == ERR_OK) {
        screen_wait();
        goto end;
      } else {
        ur.crc = 0;
      }
    } else if (qrerr == ERR_DECODE && score < QR_SCORE_YELLOW) {
      score = QR_SCORE_YELLOW;
    } else if (qrerr != ERR_SCAN) {
      score = QR_SCORE_GREEN;
    }

    screen_wait();

    if (camera_submit(fb) != HAL_SUCCESS) {
      res = ERR_HW;
      goto end;
    }

    uint16_t indicator_color;

    if (score > QR_SCORE_YELLOW) {
      indicator_color = TH_COLOR_QR_OK;
    } else if (score > QR_SCORE_RED) {
      indicator_color = TH_COLOR_QR_NOT_DECODED;
    } else {
      indicator_color = TH_COLOR_QR_NOT_FOUND;
      score = QR_SCORE_RED;
    }

    if (prev_color != indicator_color) {
      screen_fill_area(&indicator_area, indicator_color);
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
