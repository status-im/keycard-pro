#include "log.h"
#include "hal.h"

void _log(log_type_t type, const uint8_t* data, size_t len) {
  uint8_t header[5];
  header[0] = type;
  memcpy(&header[1], &len, 4);

  //hal_uart_send(UART_LOG, header, 5);
  hal_uart_send(UART_LOG, data, len);
}
