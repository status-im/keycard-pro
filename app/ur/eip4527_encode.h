/*
 * Generated using zcbor version 0.6.0
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef EIP4527_ENCODE_H__
#define EIP4527_ENCODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_encode.h"
#include "eip4527_types.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif


int cbor_encode_eth_signature(
		uint8_t *payload, size_t payload_len,
		const struct eth_signature *input,
		size_t *payload_len_out);




int cbor_encode_hd_key(
		uint8_t *payload, size_t payload_len,
		const struct hd_key *input,
		size_t *payload_len_out);




int cbor_encode_crypto_multi_accounts(
		uint8_t *payload, size_t payload_len,
		const struct crypto_multi_accounts *input,
		size_t *payload_len_out);


#endif /* EIP4527_ENCODE_H__ */
