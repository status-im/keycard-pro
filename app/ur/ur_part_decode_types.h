/*
 * Generated using zcbor version 0.6.0
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef UR_PART_DECODE_TYPES_H__
#define UR_PART_DECODE_TYPES_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_decode.h"

/** Which value for --default-max-qty this file was created with.
 *
 *  The define is used in the other generated file to do a build-time
 *  compatibility check.
 *
 *  See `zcbor --help` for more information about --default-max-qty
 */
#define DEFAULT_MAX_QTY 3

struct ur_part {
	uint32_t _ur_part_seqNum;
	uint32_t _ur_part_seqLen;
	uint32_t _ur_part_messageLen;
	uint32_t _ur_part_checksum;
	struct zcbor_string _ur_part_data;
};


#endif /* UR_PART_DECODE_TYPES_H__ */
