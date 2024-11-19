#include <string.h>

#include "crypto/secp256k1.h"
#include "crypto/util.h"
#include "eth_data.h"

#define ETH_ERC20_TRANSFER_SIGNATURE_LEN 16
#define ETH_ERC20_TRANSFER_LEN 68

#define ETH_ERC20_TRANSFER_ADDR_OFF 16
#define ETH_ERC20_TRANSFER_VALUE_OFF 36

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
  if (eip712_field_eq(msg, msg->index.primary_type, "Permit")) {
    return EIP712_PERMIT;
  }

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

static void eth_calculate_fees(const txContent_t* tx, bignum256* fees) {
  bignum256 gas_amount;
  bignum256 prime;
  bn_read_be(secp256k1.prime, &prime);
  bn_read_compact_be(tx->startgas.value, tx->startgas.length, &gas_amount);
  bn_read_compact_be(tx->gasprice.value, tx->gasprice.length, fees);
  bn_multiply(&gas_amount, fees, &prime);
}

void eth_extract_transfer_info(const txContent_t* tx, eth_transfer_info* info) {
  info->data_str_len = 0;
  info->chain.chain_id = tx->chainID;

  uint8_t num[11];
  if (eth_db_lookup_chain(&info->chain) != ERR_OK) {
    info->chain.name = (char*) u32toa(info->chain.chain_id, num, 11);
    info->chain.ticker = "???";
  }

  const uint8_t* value;
  uint8_t value_len;

  if (info->data_type == ETH_DATA_ERC20_TRANSFER) {
    info->token.chain = info->chain.chain_id;
    info->token.addr = tx->destination;

    value = &tx->data[ETH_ERC20_TRANSFER_VALUE_OFF];
    value_len = INT256_LENGTH;

    info->to = &tx->data[ETH_ERC20_TRANSFER_ADDR_OFF];

    if (eth_db_lookup_erc20(&info->token) != ERR_OK) {
      info->token.ticker = "???";
      info->token.decimals = 18;
    }
  } else {
    info->token.ticker = info->chain.ticker;
    info->token.decimals = 18;

    value = tx->value.value;
    value_len = tx->value.length;

    info->to = tx->destination;

    if (info->data_type == ETH_DATA_UNKNOWN) {
      base16_encode(tx->data, (char *) info->data_str, tx->dataLength);
      info->data_str_len = tx->dataLength * 2;
    }
  }

  bn_read_compact_be(value, value_len, &info->value);
  eth_calculate_fees(tx, &info->fees);
}
