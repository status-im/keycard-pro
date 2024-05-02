/*
 * Generated using zcbor version 0.8.1
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef AUTH_ENCODE_H__
#define AUTH_ENCODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "auth_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif


int cbor_encode_dev_auth(
		uint8_t *payload, size_t payload_len,
		const struct dev_auth *input,
		size_t *payload_len_out);


#ifdef __cplusplus
}
#endif

#endif /* AUTH_ENCODE_H__ */
