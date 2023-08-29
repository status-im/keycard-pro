#include "usb.h"
#include "hal.h"

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
        .config_id = 1,
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

static void usb_get_status(uint16_t status) {
  hal_usb_send(0x80, (uint8_t*)&status, 2);
}

static void usb_stall_ep0() {
  hal_usb_set_stall(0x00, 1);
  hal_usb_set_stall(0x80, 1);
}

static void usb_ep0_ack() {
  hal_usb_send(0x80, NULL, 0);
}

static void usb_get_descriptor(usb_desc_id_t desc_id) {
  switch(desc_id) {
  case USB_DESC_DEV:
    hal_usb_send(0x80, (uint8_t*) &kpro_dev_desc, sizeof(kpro_dev_desc));
    break;
  case USB_DESC_CONFIG:
    hal_usb_send(0x80, (uint8_t*) &kpro_conf_desc, sizeof(kpro_conf_desc));
    break;
  case USB_DESC_STRING:
    usb_ep0_ack();
    break;
  default:
    usb_stall_ep0();
  }
}

static void usb_dev_std_req(usb_setup_packet_t* packet) {
  uint8_t config = 1;

  switch(packet->req) {
  case USB_REQ_GET_STATUS:
    usb_get_status(0);
    break;
  case USB_REQ_CLEAR_FEATURE:
    usb_ep0_ack();
    break;
  case USB_REQ_SET_FEATURE:
    usb_ep0_ack();
    break;
  case USB_REQ_SET_ADDRESS:
    hal_usb_set_address(packet->val);
    usb_ep0_ack();
    break;
  case USB_REQ_GET_DESCRIPTOR:
    if (packet->len) {
      usb_get_descriptor(packet->desc_type);
    } else {
      usb_ep0_ack();
    }
    break;
  case USB_REQ_SET_DESCRIPTOR:
    usb_stall_ep0();
    break;
  case USB_REQ_GET_CONFIGURATION:
    hal_usb_send(0x80, &config, 1);
    break;
  case USB_REQ_SET_CONFIGURATION:
    usb_ep0_ack();
    break;
  default:
    break;
  }
}

static void usb_dev_class_req(usb_setup_packet_t* packet) {

}

static void usb_dev_vend_req(usb_setup_packet_t* packet) {

}

static void usb_if_std_req(usb_setup_packet_t* packet) {
  uint8_t if_id = 0;

  switch(packet->req) {
  case USB_REQ_GET_STATUS:
    usb_get_status(0);
    break;
  case USB_REQ_CLEAR_FEATURE:
    usb_stall_ep0();
    break;
  case USB_REQ_SET_FEATURE:
    usb_stall_ep0();
    break;
  case USB_REQ_GET_INTERFACE:
    hal_usb_send(0x80, &if_id, 1);
    break;
  case USB_REQ_SET_INTERFACE:
    usb_stall_ep0();
    break;
  default:
    break;
  }
}

static void usb_if_class_req(usb_setup_packet_t* packet) {
  switch(packet->req) {
  case USB_HID_GET_DESCRIPTOR:
    hal_usb_send(0x80, kpro_hid_report_desc, sizeof(kpro_hid_report_desc));
    break;
  default:
    usb_stall_ep0();
  }
}

static void usb_if_vend_req(usb_setup_packet_t* packet) {

}

static void usb_ep_std_req(usb_setup_packet_t* packet) {
  switch(packet->req) {
  case USB_REQ_GET_STATUS:
    usb_get_status(hal_usb_get_stall(packet->idx));
    break;
  case USB_REQ_CLEAR_FEATURE:
    if (packet->val == USB_EP_HALT) {
      hal_usb_set_stall(packet->idx, 0);
    }
    break;
  case USB_REQ_SET_FEATURE:
    hal_usb_set_stall(packet->idx, 1);
    break;
  case USB_REQ_SYNCH_FRAME:
    hal_usb_set_stall(packet->idx, 1);
    break;
  default:
    break;
  }
}

static void usb_ep_class_req(usb_setup_packet_t* packet) {

}

static void usb_ep_vend_req(usb_setup_packet_t* packet) {

}

static void usb_dev_req(usb_setup_packet_t* packet) {
  switch(packet->req_type & 0x60) {
  case USB_REQUEST_STD:
    usb_dev_std_req(packet);
    break;
  case USB_REQUEST_CLASS:
    usb_dev_class_req(packet);
    break;
  case USB_REQUEST_VEND:
    usb_dev_vend_req(packet);
    break;
  default:
    break;
  }
}

static void usb_if_req(usb_setup_packet_t* packet) {
  switch(packet->req_type & 0x60) {
  case USB_REQUEST_STD:
    usb_if_std_req(packet);
    break;
  case USB_REQUEST_CLASS:
    usb_if_class_req(packet);
    break;
  case USB_REQUEST_VEND:
    usb_if_vend_req(packet);
    break;
  default:
    break;
  }
}

static void usb_ep_req(usb_setup_packet_t* packet) {
  switch(packet->req_type & 0x60) {
  case USB_REQUEST_STD:
    usb_ep_std_req(packet);
    break;
  case USB_REQUEST_CLASS:
    usb_ep_class_req(packet);
    break;
  case USB_REQUEST_VEND:
    usb_ep_vend_req(packet);
    break;
  default:
    break;
  }
}

void hal_usb_setup_cb(uint8_t* data) {
  usb_setup_packet_t* packet = (usb_setup_packet_t*) data;

  switch(packet->req_type & 0x1f) {
  case USB_REQUEST_DEV:
    usb_dev_req(packet);
    break;
  case USB_REQUEST_IF:
    usb_if_req(packet);
    break;
  case USB_REQUEST_EP:
    usb_ep_req(packet);
    break;
  default:
    usb_stall_ep0();
    break;
  }
}
