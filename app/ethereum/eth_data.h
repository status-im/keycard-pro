#ifndef __ETH_DATA__
#define __ETH_DATA__

#include <stddef.h>

#include "crypto/bignum.h"
#include "eip712.h"
#include "eth_db.h"
#include "ethUstream.h"

#define EIP712_MAX_NAME_LEN 40
#define EIP712_ADDR_OFF 12

typedef enum {
  ETH_DATA_UNKNOWN,
  ETH_DATA_ERC20_TRANSFER,
  ETH_DATA_ERC20_APPROVE,
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

typedef struct {
  chain_desc_t chain;
  erc20_desc_t token;
  uint8_t* data_str;
  size_t data_str_len;
  eth_data_type_t data_type;
  const uint8_t* to;
  bignum256 value;
  bignum256 fees;
  uint8_t _chain_num[11];
} eth_transfer_info;

typedef struct {
  chain_desc_t chain;
  erc20_desc_t token;
  const uint8_t* spender;
  bignum256 value;
  bignum256 fees;
  uint8_t _addr[32];
  uint8_t _chain_num[11];
} eth_approve_info;

eth_data_type_t eth_data_recognize(const txContent_t* tx);
eip712_data_type_t eip712_recognize(const eip712_ctx_t* ctx);

app_err_t eip712_extract_domain(const eip712_ctx_t* ctx, eip712_domain_t* out);

void eth_extract_transfer_info(const txContent_t* tx, eth_transfer_info* info);
void eth_extract_approve_info(const txContent_t* tx, eth_approve_info* info);

app_err_t eip712_extract_approve_info(const eip712_ctx_t* ctx, eth_approve_info* info);

#endif
