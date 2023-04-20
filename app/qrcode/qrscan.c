#include "log/log.h"
#include "camera/camera.h"
#include "qrcode.h"
#include "ur/ur.h"
#include "ur/eip4527_decode.h"
#include "screen/screen.h"
#include "ui/theme.h"
#include "ui/ui_internal.h"
#include "error.h"

app_err_t qrscan_decode() {
  if (quirc_count(&g_ui_ctx.qr_internal.qrctx) != 1) {
    return ERR_RETRY;
  }

  quirc_extract(&g_ui_ctx.qr_internal.qrctx, 0, &g_ui_ctx.qrcode);
  quirc_decode_error_t err = quirc_decode(&g_ui_ctx.qrcode, &g_ui_ctx.qr_internal.qrdata);

  return !err ? ur_process_part(&g_ui_ctx.ur, g_ui_ctx.qr_internal.qrdata.payload, g_ui_ctx.qr_internal.qrdata.payload_len) : ERR_RETRY;
}

app_err_t qrscan_scan() {
  app_err_t res = ERR_OK;
  g_ui_ctx.ur.data_max_len = sizeof(struct quirc_code);
  g_ui_ctx.ur.data = (uint8_t*)&g_ui_ctx.qrcode;

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

    quirc_set_image(&g_ui_ctx.qr_internal.qrctx, fb, CAMERA_WIDTH, CAMERA_HEIGHT);
    quirc_begin(&g_ui_ctx.qr_internal.qrctx, NULL, NULL);

    quirc_threshold(&g_ui_ctx.qr_internal.qrctx);
    screen_camera_passthrough(fb);

    quirc_end(&g_ui_ctx.qr_internal.qrctx);

    if (qrscan_decode() == ERR_OK && g_ui_ctx.ur.type == ETH_SIGN_REQUEST) {
      cbor_decode_eth_sign_request(g_ui_ctx.ur.data, g_ui_ctx.ur.data_len, g_ui_cmd.params.qrscan.out, NULL);
      screen_wait();
      goto end;
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
