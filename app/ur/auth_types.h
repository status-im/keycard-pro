/*
 * Generated using zcbor version 0.6.0
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef AUTH_TYPES_H__
#define AUTH_TYPES_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_encode.h"

/** Which value for --default-max-qty this file was created with.
 *
 *  The define is used in the other generated file to do a build-time
 *  compatibility check.
 *
 *  See `zcbor --help` for more information about --default-max-qty
 */
#define DEFAULT_MAX_QTY 3

struct dev_auth_step_type_ {
	enum {
		_dev_auth_step_type__dev_auth_init = 1,
		_dev_auth_step_type__dev_auth_kpro = 2,
		_dev_auth_step_type__dev_auth_server = 3,
	} _dev_auth_step_type_choice;
};

struct dev_auth_device_id {
	struct zcbor_string _dev_auth_device_id;
};

struct dev_auth_first_auth {
	uint32_t _dev_auth_first_auth;
};

struct dev_auth_auth_count {
	int32_t _dev_auth_auth_count;
};

struct dev_auth_challenge {
	struct zcbor_string _dev_auth_challenge;
};

struct dev_auth_signature {
	struct zcbor_string _dev_auth_signature;
};

struct dev_auth {
	struct dev_auth_step_type_ _dev_auth_step;
	struct dev_auth_device_id _dev_auth_device_id;
	uint_fast32_t _dev_auth_device_id_present;
	struct dev_auth_first_auth _dev_auth_first_auth;
	uint_fast32_t _dev_auth_first_auth_present;
	struct dev_auth_auth_count _dev_auth_auth_count;
	uint_fast32_t _dev_auth_auth_count_present;
	struct dev_auth_challenge _dev_auth_challenge;
	uint_fast32_t _dev_auth_challenge_present;
	struct dev_auth_signature _dev_auth_signature;
	uint_fast32_t _dev_auth_signature_present;
};


#endif /* AUTH_TYPES_H__ */
