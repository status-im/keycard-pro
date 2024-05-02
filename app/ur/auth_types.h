/*
 * Generated using zcbor version 0.8.1
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef AUTH_TYPES_H__
#define AUTH_TYPES_H__

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

struct dev_auth_step_type_r {
	enum {
		dev_auth_step_type_dev_auth_init_m_c = 1,
		dev_auth_step_type_dev_auth_device_m_c = 2,
		dev_auth_step_type_dev_auth_server_m_c = 3,
	} dev_auth_step_type_choice;
};

struct dev_auth_device_id {
	struct zcbor_string dev_auth_device_id;
};

struct dev_auth_first_auth {
	uint32_t dev_auth_first_auth;
};

struct dev_auth_auth_time {
	uint32_t dev_auth_auth_time;
};

struct dev_auth_auth_count {
	uint32_t dev_auth_auth_count;
};

struct dev_auth_challenge {
	struct zcbor_string dev_auth_challenge;
};

struct dev_auth_signature {
	struct zcbor_string dev_auth_signature;
};

struct dev_auth {
	struct dev_auth_step_type_r dev_auth_step;
	struct dev_auth_device_id dev_auth_device_id;
	bool dev_auth_device_id_present;
	struct dev_auth_first_auth dev_auth_first_auth;
	bool dev_auth_first_auth_present;
	struct dev_auth_auth_time dev_auth_auth_time;
	bool dev_auth_auth_time_present;
	struct dev_auth_auth_count dev_auth_auth_count;
	bool dev_auth_auth_count_present;
	struct dev_auth_challenge dev_auth_challenge;
	bool dev_auth_challenge_present;
	struct dev_auth_signature dev_auth_signature;
	bool dev_auth_signature_present;
};

#ifdef __cplusplus
}
#endif

#endif /* AUTH_TYPES_H__ */
