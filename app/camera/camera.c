#include "hal.h"
#include "common.h"
#include "camera.h"
#include "FreeRTOS.h"
#include "task.h"

#define _CAM_REG_NULL 0xFFFF
#define _CAM_REG_DELAY 0x0000
#define CAMERA_SETTLE_MS 10
#define CAMERA_FRAME_TIMEOUT 100

struct camera_regval {
  uint16_t addr;
  uint8_t val;
};

#if _CAM_MODEL == SC031GS
#include "sc031gs.h"
#else
#error _CAM_MODEL must be defined
#endif

APP_NOCACHE(uint8_t g_camera_fb[CAMERA_FB_COUNT][CAMERA_FB_SIZE], CAMERA_BUFFER_ALIGN);

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

hal_err_t _camera_load_regs(const struct camera_regval regs[]) {
  size_t i = 0;
  hal_err_t err = HAL_SUCCESS;

  while(1) {
    switch(regs[i].addr) {
      case _CAM_REG_DELAY:
        vTaskDelay(pdMS_TO_TICKS(regs[i].val));
        break;
      case _CAM_REG_NULL:
        goto finish;
      default:
        err = _camera_load_reg(regs[i].addr, regs[i].val);
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

  if ((err = _camera_load_regs(camera_regs)) != HAL_SUCCESS) {
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

hal_err_t camera_set_exposure(camera_exposure_t exposure) {
  struct camera_regval regs[3];
  _camera_exposure_regs(regs, exposure);
  return _camera_load_regs(regs);
}

hal_err_t camera_set_analog_gain(uint8_t gain) {
  struct camera_regval regs[3];
  _camera_analog_gain_regs(regs, gain);
  return _camera_load_regs(regs);
}

hal_err_t camera_set_digital_gain(uint8_t gain) {
  struct camera_regval regs[3];
  _camera_digital_gain_regs(regs, gain);
  return _camera_load_regs(regs);
}
