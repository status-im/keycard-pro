#include "eth_data.h"
#include <string.h>

#define ETH_ERC20_TRANSFER_SIGNATURE_LEN 16
#define ETH_ERC20_TRANSFER_LEN 68

const uint8_t ETH_ERC20_TRANSFER_SIGNATURE[] = {
    0xa9, 0x05, 0x9c, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

eth_data_type_t eth_data_recognize(const txContent_t* tx) {
  if (tx->dataLength == 0) {
    return ETH_DATA_ABSENT;
  } else if (tx->value.length == 0 && (tx->dataLength == ETH_ERC20_TRANSFER_LEN) && !memcmp(tx->data, ETH_ERC20_TRANSFER_SIGNATURE, ETH_ERC20_TRANSFER_SIGNATURE_LEN)) {
    return ETH_DATA_ERC20_TRANSFER;
  } else {
    return ETH_DATA_UNKNOWN;
  }
}

eip712_data_type_t eip712_recognize(const eip712_ctx_t* msg) {
  return EIP712_UNKNOWN;
}

app_err_t eip712_extract_domain(const eip712_ctx_t* ctx, eip712_domain_t* out) {
  if (eip712_extract_uint256(ctx, ctx->index.domain, "verifyingContract", out->address) != ERR_OK) {
    return ERR_DATA;
  }

  if (eip712_extract_string(ctx, ctx->index.domain, "name", out->name, EIP712_MAX_NAME_LEN) != ERR_OK) {
    return ERR_DATA;
  }

  uint8_t chain_bytes[32];

  if (eip712_extract_uint256(ctx, ctx->index.domain, "chainId", chain_bytes) != ERR_OK) {
    return ERR_DATA;
  }

  out->chainID = (chain_bytes[28] << 24) | (chain_bytes[29] << 16) | (chain_bytes[30] << 8) | chain_bytes[31];

  return ERR_OK;
}
