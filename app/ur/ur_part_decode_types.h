/*
 * Generated using zcbor version 0.8.1
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef UR_PART_DECODE_TYPES_H__
#define UR_PART_DECODE_TYPES_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <zcbor_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Which value for --default-max-qty this file was created with.
 *
 *  The define is used in the other generated file to do a build-time
 *  compatibility check.
 *
 *  See `zcbor --help` for more information about --default-max-qty
 */
#define DEFAULT_MAX_QTY 3

struct ur_part {
	uint32_t ur_part_seqNum;
	uint32_t ur_part_seqLen;
	uint32_t ur_part_messageLen;
	uint32_t ur_part_checksum;
	struct zcbor_string ur_part_data;
};

#ifdef __cplusplus
}
#endif

#endif /* UR_PART_DECODE_TYPES_H__ */
