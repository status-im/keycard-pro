#include "usb.h"

APP_ALIGNED(static uint8_t kpro_hid_report_desc[27], 4) = {
  0x06, 0x00, 0xff,  // Usage Page (Vendor Defined 0xff00)
  0x09, 0x01,        // Usage (0x01)
  0xa1, 0x01,        // Collection (Application)
  0x15, 0x00,        //   Logical Minimum (0)
  0x26, 0xff, 0x00,  //   Logical Maximum (255)
  0x75, 0x08,        //   Report Size (8)
  0x95, 0x40,        //   Report Count (64)
  0x09, 0x01,        //   Usage (0x01)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x95, 0x40,        //   Report Count (64)
  0x09, 0x01,        //   Usage (0x01)
  0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  0xc0,              // End Collection
};

APP_ALIGNED(usb_dev_desc_t kpro_dev_desc, 4) = {
    .len = 18,
    .type = 0x01, // Device descriptor
    .usb_version = 0x0200, // USB 2.0
    .dev_class = 0, // Class in IF descriptor
    .dev_subclass = 0,
    .dev_protocol = 0,
    .max_packet_size = 64, // EP0 packet size
    .vendor_id = 0x1209, // pid.codes VID for now
    .product_id = 0x0001, // pid.codes test PID
    .dev_version = 0x0100, // Version 1.0
    .manufacturer_str_idx = 0, //TODO: add strings
    .product_str_idx = 0,
    .serial_number_str_idx = 0,
    .config_count = 1
};

APP_ALIGNED(usb_hid_inout_desc_t kpro_conf_desc, 4) = {
    .confd = {
        .len = 9,
        .type = 0x02, // Config descriptor
        .total_len = sizeof(usb_hid_inout_desc_t),
        .if_count = 1,
        .config_id = 0,
        .config_str_idx = 0,
        .attr = 0x80, // bus powered
        .max_power = 250, // 500mA
    },
    .ifd = {
        .len = 9,
        .type = 0x04, // Interface descriptor
        .if_id = 0,
        .alt_id = 0,
        .ep_count = 2,
        .class = 3, // HID
        .subclass = 0,
        .protocol = 0,
        .if_str_idx = 0,
    },
    .hidd = {
        .len = 9,
        .type = 0x21, // HID
        .hid_version = 0x0111, //Version 1.11
        .country_code = 0,
        .desc_count = 1,
        .desc_type = 0x22, // HID Report
        .desc_len = sizeof(kpro_hid_report_desc),
    },
    .epind = {
        .len = 7,
        .type = 0x05, // Endpoint descriptor
        .addr = 0x81, // EP1 IN
        .attr = 0x3, // Interrupt transfer type
        .max_packet_size = 64,
        .poll_interval = 20, // in ms
    },
    .epoutd = {
        .len = 7,
        .type = 0x05, // Endpoint descriptor
        .addr = 0x01, // EP1 OUT
        .attr = 0x3, // Interrupt transfer type
        .max_packet_size = 64,
        .poll_interval = 20 // in ms
    }
};
