#include "log.h"
#include "hal.h"

const uint8_t LOG_MSG_START[] = { 0xa5, 0xca, 0xfe, 0x5a };

void _log(log_type_t type, const uint8_t* data, size_t len) {
  uint8_t header[5];
  header[0] = type;
  memcpy(&header[1], &len, 4);

  hal_uart_send(UART_LOG, LOG_MSG_START, 4);
  hal_uart_send(UART_LOG, header, 5);
  hal_uart_send(UART_LOG, data, len);
}
