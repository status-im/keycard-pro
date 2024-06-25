/*
 * Generated using zcbor version 0.8.1
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef BTC_DECODE_H__
#define BTC_DECODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "btc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif


int cbor_decode_psbt(
		const uint8_t *payload, size_t payload_len,
		struct zcbor_string *result,
		size_t *payload_len_out);


#ifdef __cplusplus
}
#endif

#endif /* BTC_DECODE_H__ */
