#ifndef __USB__
#define __USB__

#include "common.h"

typedef struct __attribute__((packed)) {
  uint8_t len;
  uint8_t type;
  uint16_t usb_version;
  uint8_t dev_class;
  uint8_t dev_subclass;
  uint8_t dev_protocol;
  uint8_t max_packet_size;
  uint16_t vendor_id;
  uint16_t product_id;
  uint16_t dev_version;
  uint8_t manufacturer_str_idx;
  uint8_t product_str_idx;
  uint8_t serial_number_str_idx;
  uint8_t config_count;
} usb_dev_desc_t;

typedef struct __attribute__((packed)) {
  uint8_t len;
  uint8_t type;
  uint16_t total_len;
  uint8_t if_count;
  uint8_t config_id;
  uint8_t config_str_idx;
  uint8_t attr;
  uint8_t max_power;
} usb_conf_desc_t;

typedef struct __attribute__((packed)) {
  uint8_t len;
  uint8_t type;
  uint8_t if_id;
  uint8_t alt_id;
  uint8_t ep_count;
  uint8_t class;
  uint8_t subclass;
  uint8_t protocol;
  uint8_t if_str_idx;
} usb_if_desc_t;

typedef struct __attribute__((packed)) {
  uint8_t len;
  uint8_t type;
  uint8_t addr;
  uint8_t attr;
  uint16_t max_packet_size;
  uint8_t poll_interval;
} usb_ep_desc_t;

#endif
