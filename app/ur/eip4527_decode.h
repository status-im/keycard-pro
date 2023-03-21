/*
 * Generated using zcbor version 0.6.0
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef EIP4527_DECODE_H__
#define EIP4527_DECODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_decode.h"
#include "eip4527_types.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif


int cbor_decode_eth_sign_request(
		const uint8_t *payload, size_t payload_len,
		struct eth_sign_request *result,
		size_t *payload_len_out);


#endif /* EIP4527_DECODE_H__ */
