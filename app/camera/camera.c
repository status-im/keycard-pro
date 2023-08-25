#include "hal.h"
#include "common.h"
#include "camera.h"
#include "FreeRTOS.h"
#include "task.h"

#if _CAM_MODEL == SC031GS
#include "sc031gs.h"
#else
#error _CAM_MODEL must be defined
#endif
APP_NOCACHE(uint8_t g_camera_fb[CAMERA_FB_COUNT][CAMERA_FB_SIZE], CAMERA_BUFFER_ALIGN);

#define CAMERA_SETTLE_MS 10
#define CAMERA_FRAME_TIMEOUT 100

hal_err_t _camera_reset() {
  hal_gpio_set(GPIO_CAMERA_PWR, GPIO_SET);
  vTaskDelay(pdMS_TO_TICKS(CAMERA_SETTLE_MS));

  hal_gpio_set(GPIO_CAMERA_PWDN, _CAM_PWR_ON);
  vTaskDelay(pdMS_TO_TICKS(CAMERA_SETTLE_MS));

  hal_gpio_set(GPIO_CAMERA_RST, GPIO_SET);
  vTaskDelay(pdMS_TO_TICKS(CAMERA_SETTLE_MS));

  return HAL_SUCCESS;
}

hal_err_t _camera_load_reg(uint16_t addr, uint8_t val) {
  uint8_t buf[3];
  buf[0] = addr >> 8;
  buf[1] = addr & 0xff;
  buf[2] = val;

  return hal_i2c_send(I2C_CAMERA, _CAM_I2C_ADDR, buf, 3);
}

hal_err_t _camera_load_regs() {
  size_t i = 0;
  hal_err_t err = HAL_SUCCESS;

  while(1) {
    switch(camera_regs[i].addr) {
      case _CAM_REG_DELAY:
        vTaskDelay(pdMS_TO_TICKS(camera_regs[i].val));
        break;
      case _CAM_REG_NULL:
        goto finish;
      default:
        err = _camera_load_reg(camera_regs[i].addr, camera_regs[i].val);
        if (err != HAL_SUCCESS) {
          goto finish;
        }
        break;
    }

    i++;
  }

finish:
  return err;
}

hal_err_t camera_start() {
  hal_err_t err;

  if ((err = hal_camera_init()) != HAL_SUCCESS) {
    return err;
  }

  if ((err = _camera_reset()) != HAL_SUCCESS) {
    return err;
  }

  if ((err = _camera_load_regs()) != HAL_SUCCESS) {
    return err;
  }

  err = hal_camera_start(g_camera_fb);
  return err;
}

hal_err_t camera_stop() {
  hal_gpio_set(GPIO_CAMERA_RST, GPIO_RESET);
  hal_gpio_set(GPIO_CAMERA_PWDN, _CAM_PWR_OFF);
  hal_gpio_set(GPIO_CAMERA_PWR, GPIO_RESET);

  hal_err_t err = hal_camera_stop();
  ulTaskNotifyTakeIndexed(CAMERA_TASK_NOTIFICATION_IDX, pdTRUE, 0);

  return err;
}

hal_err_t camera_next_frame(uint8_t** frame) {
  ulTaskNotifyTakeIndexed(CAMERA_TASK_NOTIFICATION_IDX, pdFALSE, pdMS_TO_TICKS(CAMERA_FRAME_TIMEOUT));
  return hal_camera_next_frame(frame);
}

hal_err_t camera_submit(uint8_t* frame) {
  return hal_camera_submit(frame);
}
