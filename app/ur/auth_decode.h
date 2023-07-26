/*
 * Generated using zcbor version 0.6.0
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef AUTH_DECODE_H__
#define AUTH_DECODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_decode.h"
#include "auth_types.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif


int cbor_decode_dev_auth(
		const uint8_t *payload, size_t payload_len,
		struct dev_auth *result,
		size_t *payload_len_out);


#endif /* AUTH_DECODE_H__ */
