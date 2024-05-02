/*
 * Generated using zcbor version 0.8.1
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_decode.h"
#include "auth_decode.h"
#include "zcbor_print.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif

static bool decode_dev_auth_step_type(zcbor_state_t *state, struct dev_auth_step_type_r *result);
static bool decode_uuid(zcbor_state_t *state, struct zcbor_string *result);
static bool decode_repeated_dev_auth_device_id(zcbor_state_t *state, struct dev_auth_device_id *result);
static bool decode_repeated_dev_auth_first_auth(zcbor_state_t *state, struct dev_auth_first_auth *result);
static bool decode_repeated_dev_auth_auth_time(zcbor_state_t *state, struct dev_auth_auth_time *result);
static bool decode_repeated_dev_auth_auth_count(zcbor_state_t *state, struct dev_auth_auth_count *result);
static bool decode_repeated_dev_auth_challenge(zcbor_state_t *state, struct dev_auth_challenge *result);
static bool decode_repeated_dev_auth_signature(zcbor_state_t *state, struct dev_auth_signature *result);
static bool decode_dev_auth(zcbor_state_t *state, struct dev_auth *result);


static bool decode_dev_auth_step_type(
		zcbor_state_t *state, struct dev_auth_step_type_r *result)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((((zcbor_uint_decode(state, &(*result).dev_auth_step_type_choice, sizeof((*result).dev_auth_step_type_choice)))) && ((((((*result).dev_auth_step_type_choice == dev_auth_step_type_dev_auth_init_m_c) && ((1)))
	|| (((*result).dev_auth_step_type_choice == dev_auth_step_type_dev_auth_device_m_c) && ((1)))
	|| (((*result).dev_auth_step_type_choice == dev_auth_step_type_dev_auth_server_m_c) && ((1)))) || (zcbor_error(state, ZCBOR_ERR_WRONG_VALUE), false))))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool decode_uuid(
		zcbor_state_t *state, struct zcbor_string *result)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((zcbor_tag_expect(state, 37)
	&& (zcbor_bstr_decode(state, (&(*result))))
	&& ((((*result).len >= 16)
	&& ((*result).len <= 16)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool decode_repeated_dev_auth_device_id(
		zcbor_state_t *state, struct dev_auth_device_id *result)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (2))))
	&& (decode_uuid(state, (&(*result).dev_auth_device_id)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool decode_repeated_dev_auth_first_auth(
		zcbor_state_t *state, struct dev_auth_first_auth *result)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (3))))
	&& (zcbor_uint32_decode(state, (&(*result).dev_auth_first_auth)))
	&& ((((((*result).dev_auth_first_auth <= UINT32_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool decode_repeated_dev_auth_auth_time(
		zcbor_state_t *state, struct dev_auth_auth_time *result)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (4))))
	&& (zcbor_uint32_decode(state, (&(*result).dev_auth_auth_time)))
	&& ((((((*result).dev_auth_auth_time <= UINT32_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool decode_repeated_dev_auth_auth_count(
		zcbor_state_t *state, struct dev_auth_auth_count *result)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (5))))
	&& (zcbor_uint32_decode(state, (&(*result).dev_auth_auth_count)))
	&& ((((((*result).dev_auth_auth_count <= UINT32_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool decode_repeated_dev_auth_challenge(
		zcbor_state_t *state, struct dev_auth_challenge *result)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (6))))
	&& (zcbor_bstr_decode(state, (&(*result).dev_auth_challenge)))
	&& ((((*result).dev_auth_challenge.len >= 32)
	&& ((*result).dev_auth_challenge.len <= 32)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool decode_repeated_dev_auth_signature(
		zcbor_state_t *state, struct dev_auth_signature *result)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (7))))
	&& (zcbor_bstr_decode(state, (&(*result).dev_auth_signature)))
	&& ((((*result).dev_auth_signature.len >= 64)
	&& ((*result).dev_auth_signature.len <= 64)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool decode_dev_auth(
		zcbor_state_t *state, struct dev_auth *result)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_decode(state) && (((((zcbor_uint32_expect(state, (1))))
	&& (decode_dev_auth_step_type(state, (&(*result).dev_auth_step))))
	&& zcbor_present_decode(&((*result).dev_auth_device_id_present), (zcbor_decoder_t *)decode_repeated_dev_auth_device_id, state, (&(*result).dev_auth_device_id))
	&& zcbor_present_decode(&((*result).dev_auth_first_auth_present), (zcbor_decoder_t *)decode_repeated_dev_auth_first_auth, state, (&(*result).dev_auth_first_auth))
	&& zcbor_present_decode(&((*result).dev_auth_auth_time_present), (zcbor_decoder_t *)decode_repeated_dev_auth_auth_time, state, (&(*result).dev_auth_auth_time))
	&& zcbor_present_decode(&((*result).dev_auth_auth_count_present), (zcbor_decoder_t *)decode_repeated_dev_auth_auth_count, state, (&(*result).dev_auth_auth_count))
	&& zcbor_present_decode(&((*result).dev_auth_challenge_present), (zcbor_decoder_t *)decode_repeated_dev_auth_challenge, state, (&(*result).dev_auth_challenge))
	&& zcbor_present_decode(&((*result).dev_auth_signature_present), (zcbor_decoder_t *)decode_repeated_dev_auth_signature, state, (&(*result).dev_auth_signature))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_map_end_decode(state))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}



int cbor_decode_dev_auth(
		const uint8_t *payload, size_t payload_len,
		struct dev_auth *result,
		size_t *payload_len_out)
{
	zcbor_state_t states[4];

	return zcbor_entry_function(payload, payload_len, (void *)result, payload_len_out, states,
		(zcbor_decoder_t *)decode_dev_auth, sizeof(states) / sizeof(zcbor_state_t), 1);
}
