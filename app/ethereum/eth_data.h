#ifndef __ETH_DATA__
#define __ETH_DATA__

#include <stddef.h>
#include "ethUstream.h"
#include "eip712.h"

#define ETH_ERC20_ADDR_OFF 16
#define ETH_ERC20_VALUE_OFF 36

#define EIP712_MAX_NAME_LEN 40
#define EIP712_ADDR_OFF 12

typedef enum {
  ETH_DATA_ABSENT,
  ETH_DATA_UNKNOWN,
  ETH_DATA_ERC20_TRANSFER
} eth_data_type_t;

typedef enum {
  EIP712_UNKNOWN,
  EIP712_PERMIT,
} eip712_data_type_t;

typedef struct {
  uint8_t address[32];
  char name[EIP712_MAX_NAME_LEN];
  uint32_t chainID;
} eip712_domain_t;

eth_data_type_t eth_data_recognize(const txContent_t* tx);
eip712_data_type_t eip712_recognize(const eip712_ctx_t* msg);

app_err_t eip712_extract_domain(const eip712_ctx_t* ctx, eip712_domain_t* out);

#endif
