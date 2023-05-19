/*
 * Generated using zcbor version 0.6.0
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef UR_PART_DECODE_H__
#define UR_PART_DECODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_decode.h"
#include "ur_part_decode_types.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif


int cbor_decode_ur_part(
		const uint8_t *payload, size_t payload_len,
		struct ur_part *result,
		size_t *payload_len_out);


#endif /* UR_PART_DECODE_H__ */
