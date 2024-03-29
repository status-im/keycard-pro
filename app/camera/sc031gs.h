#ifndef __CAMERA_REGS__
#define __CAMERA_REGS__

#define SC031GS

#define SC031GS_OUTPUT_WINDOW_START_X_H_REG 0x3212
#define SC031GS_OUTPUT_WINDOW_START_X_L_REG 0x3213
#define SC031GS_OUTPUT_WINDOW_START_Y_H_REG 0x3210
#define SC031GS_OUTPUT_WINDOW_START_Y_L_REG 0x3211
#define SC031GS_OUTPUT_WINDOW_WIDTH_H_REG 0x3208
#define SC031GS_OUTPUT_WINDOW_WIDTH_L_REG 0x3209
#define SC031GS_OUTPUT_WINDOW_HIGH_H_REG 0x320a
#define SC031GS_OUTPUT_WINDOW_HIGH_L_REG 0x320b

#define SC031GS_EXPOSURE_H_REG 0x3e01
#define SC031GS_EXPOSURE_L_REG 0x3e02

#define SC031GS_GAIN_DIGITAL_H_REG 0x3e06
#define SC031GS_GAIN_DIGITAL_L_REG 0x3e07
#define SC031GS_GAIN_ANALOG_H_REG 0x3e08
#define SC031GS_GAIN_ANALOG_L_REG 0x3e09

#define SC031GS_GAIN_CFG_DIRECT {0x3e03,0x03}
#define SC031GS_GAIN_CFG_TABLE {0x3e03,0x0b}

#define SC031GS_MAX_FRAME_X 488
#define SC031GS_MAX_FRAME_Y 648

#define SC031GS_FPS_15 {0x320e,0x15}, {0x320f,0x58}
#define SC031GS_FPS_30 {0x320e,0x0a}, {0x320f,0xac}
#define SC031GS_FPS_60 {0x320e,0x05}, {0x320f,0x56}
#define SC031GS_FPS_120 {0x320e,0x02}, {0x320f,0xab}

#define SC031GS_MIRROR_FLIP {0x3221,0x63}

#define SC031GS_OUTPUT_WINDOW(w, h) \
    {SC031GS_OUTPUT_WINDOW_START_Y_H_REG, (SC031GS_MAX_FRAME_Y - w) >> 9}, \
    {SC031GS_OUTPUT_WINDOW_START_Y_L_REG, ((SC031GS_MAX_FRAME_Y - w) / 2) & 0xff}, \
    {SC031GS_OUTPUT_WINDOW_START_X_H_REG, (SC031GS_MAX_FRAME_X - h) >> 9}, \
    {SC031GS_OUTPUT_WINDOW_START_X_L_REG, ((SC031GS_MAX_FRAME_X - h) / 2) & 0xff}, \
    {SC031GS_OUTPUT_WINDOW_WIDTH_H_REG, w >> 8}, \
    {SC031GS_OUTPUT_WINDOW_WIDTH_L_REG, w & 0xff}, \
    {SC031GS_OUTPUT_WINDOW_HIGH_H_REG, h >> 8}, \
    {SC031GS_OUTPUT_WINDOW_HIGH_L_REG, h & 0xff}

#define _CAM_I2C_ADDR 0x30

#define _CAM_PWR_OFF GPIO_RESET
#define _CAM_PWR_ON GPIO_SET

#if CAMERA_WIDTH > 640 || CAMERA_HEIGHT > 480
#error Max resolution is 640x480
#endif

static const struct camera_regval camera_regs[] = {
  {0x0103, 0x01}, // soft reset.
  {_CAM_REG_DELAY, 10},
  {0x0100,0x00},
  {0x300f,0x0f},
  {0x3018,0x1f},
  {0x3019,0xff},
  {0x301c,0xb4},
  {0x3028,0x82},
  {0x320c,0x03},
  {0x320d,0x6e},
  SC031GS_FPS_30,
  SC031GS_OUTPUT_WINDOW(CAMERA_WIDTH, CAMERA_HEIGHT),
  {0x3220,0x10},
  {0x3250,0xf0},
  {0x3251,0x02},
  {0x3252,0x05},
  {0x3253,0x51},
  {0x3254,0x02},
  {0x3255,0x07},
  {0x3304,0x48},
  {0x3306,0x38},
  {0x3309,0x68},
  {0x330b,0xe0},
  {0x330c,0x18},
  {0x330f,0x20},
  {0x3310,0x10},
  {0x3314,0x42},
  {0x3315,0x38},
  {0x3316,0x48},
  {0x3317,0x20},
  {0x3329,0x3c},
  {0x332d,0x3c},
  {0x332f,0x40},
  {0x3335,0x44},
  {0x3344,0x44},
  {0x335b,0x80},
  {0x335f,0x80},
  {0x3366,0x06},
  {0x3385,0x31},
  {0x3387,0x51},
  {0x3389,0x01},
  {0x33b1,0x03},
  {0x33b2,0x06},
  {0x3621,0xa4},
  {0x3622,0x05},
  {0x3624,0x47},
  {0x3630,0x46},
  {0x3631,0x48},
  {0x3633,0x52},
  {0x3635,0x18},
  {0x3636,0x25},
  {0x3637,0x89},
  {0x3638,0x0f},
  {0x3639,0x08},
  {0x363a,0x00},
  {0x363b,0x48},
  {0x363c,0x06},
  {0x363d,0x00},
  {0x363e,0xf8},
  {0x3640,0x00},
  {0x3641,0x01},
  {0x36e9,0x00},
  {0x36ea,0x3b},
  {0x36eb,0x1a},
  {0x36ec,0x0a},
  {0x36ed,0x33},
  {0x36f9,0x00},
  {0x36fa,0x3a},
  {0x36fc,0x01},
  {0x3908,0x91},
  {0x3d08,0x01},
  SC031GS_GAIN_CFG_TABLE,
  {0x4500,0x59},
  {0x4501,0xc4},
  {0x5011,0x00},
  {0x0100,0x01},
  {0x4418,0x08},
  {0x4419,0x8e},
  {0x3314,0x42},
  {0x3317,0x20},
  {0x3314,0x4f},
  {0x3317,0x0f},
  {_CAM_REG_NULL, 0x00},
};

const uint16_t _camera_exposure_table[] = {
    0x0709, 0x0962, 0x0e13, 0x1198, 0x1776, 0x1c27, 0x384e
};

const uint16_t _camera_analog_gain_table[] = {
    0x0010, 0x0110, 0x0118, 0x0310, 0x0314, 0x0318, 0x031c, 0x0710,
    0x0712, 0x0714, 0x0716, 0x0718, 0x071a, 0x071c, 0x071e, 0x071f
};

const uint16_t _camera_digital_gain_table[] = {
    0x0080, 0x0180, 0x01c0, 0x0380, 0x03a0, 0x03c0, 0x03e0, 0x03f8,
};

static inline void _camera_exposure_regs(struct camera_regval regs[3], camera_exposure_t exposure) {
  uint16_t val = _camera_exposure_table[exposure];
  regs[0].addr = SC031GS_EXPOSURE_H_REG;
  regs[0].val = val >> 8;
  regs[1].addr = SC031GS_EXPOSURE_L_REG;
  regs[1].val = val & 0xff;
  regs[2].addr = _CAM_REG_NULL;
}

static inline void _camera_analog_gain_regs(struct camera_regval regs[3], uint8_t gain) {
  uint16_t val = _camera_analog_gain_table[gain - 1];
  regs[0].addr = SC031GS_GAIN_ANALOG_H_REG;
  regs[0].val = val >> 8;
  regs[1].addr = SC031GS_GAIN_ANALOG_L_REG;
  regs[1].val = val & 0xff;
  regs[2].addr = _CAM_REG_NULL;
}

static inline void _camera_digital_gain_regs(struct camera_regval regs[3], uint8_t gain) {
  uint16_t val = _camera_digital_gain_table[gain - 1];
  regs[0].addr = SC031GS_GAIN_DIGITAL_H_REG;
  regs[0].val = val >> 8;
  regs[1].addr = SC031GS_GAIN_DIGITAL_L_REG;
  regs[1].val = val & 0xff;
  regs[2].addr = _CAM_REG_NULL;
}

#else
#error This file must be only imported once.
#endif
