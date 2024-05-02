/*
 * Generated using zcbor version 0.8.1
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_encode.h"
#include "auth_encode.h"
#include "zcbor_print.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif

static bool encode_dev_auth_step_type(zcbor_state_t *state, const struct dev_auth_step_type_r *input);
static bool encode_uuid(zcbor_state_t *state, const struct zcbor_string *input);
static bool encode_repeated_dev_auth_device_id(zcbor_state_t *state, const struct dev_auth_device_id *input);
static bool encode_repeated_dev_auth_first_auth(zcbor_state_t *state, const struct dev_auth_first_auth *input);
static bool encode_repeated_dev_auth_auth_time(zcbor_state_t *state, const struct dev_auth_auth_time *input);
static bool encode_repeated_dev_auth_auth_count(zcbor_state_t *state, const struct dev_auth_auth_count *input);
static bool encode_repeated_dev_auth_challenge(zcbor_state_t *state, const struct dev_auth_challenge *input);
static bool encode_repeated_dev_auth_signature(zcbor_state_t *state, const struct dev_auth_signature *input);
static bool encode_dev_auth(zcbor_state_t *state, const struct dev_auth *input);


static bool encode_dev_auth_step_type(
		zcbor_state_t *state, const struct dev_auth_step_type_r *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((((*input).dev_auth_step_type_choice == dev_auth_step_type_dev_auth_init_m_c) ? ((zcbor_uint32_put(state, (1))))
	: (((*input).dev_auth_step_type_choice == dev_auth_step_type_dev_auth_device_m_c) ? ((zcbor_uint32_put(state, (2))))
	: (((*input).dev_auth_step_type_choice == dev_auth_step_type_dev_auth_server_m_c) ? ((zcbor_uint32_put(state, (3))))
	: false)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_uuid(
		zcbor_state_t *state, const struct zcbor_string *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((zcbor_tag_put(state, 37)
	&& ((((*input).len >= 16)
	&& ((*input).len <= 16)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input))))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_dev_auth_device_id(
		zcbor_state_t *state, const struct dev_auth_device_id *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (2))))
	&& (encode_uuid(state, (&(*input).dev_auth_device_id)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_dev_auth_first_auth(
		zcbor_state_t *state, const struct dev_auth_first_auth *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (3))))
	&& ((((((*input).dev_auth_first_auth <= UINT32_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input).dev_auth_first_auth)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_dev_auth_auth_time(
		zcbor_state_t *state, const struct dev_auth_auth_time *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (4))))
	&& ((((((*input).dev_auth_auth_time <= UINT32_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input).dev_auth_auth_time)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_dev_auth_auth_count(
		zcbor_state_t *state, const struct dev_auth_auth_count *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (5))))
	&& ((((((*input).dev_auth_auth_count <= UINT32_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input).dev_auth_auth_count)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_dev_auth_challenge(
		zcbor_state_t *state, const struct dev_auth_challenge *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (6))))
	&& ((((*input).dev_auth_challenge.len >= 32)
	&& ((*input).dev_auth_challenge.len <= 32)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input).dev_auth_challenge)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_dev_auth_signature(
		zcbor_state_t *state, const struct dev_auth_signature *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (7))))
	&& ((((*input).dev_auth_signature.len >= 64)
	&& ((*input).dev_auth_signature.len <= 64)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input).dev_auth_signature)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_dev_auth(
		zcbor_state_t *state, const struct dev_auth *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_encode(state, 7) && (((((zcbor_uint32_put(state, (1))))
	&& (encode_dev_auth_step_type(state, (&(*input).dev_auth_step))))
	&& (!(*input).dev_auth_device_id_present || encode_repeated_dev_auth_device_id(state, (&(*input).dev_auth_device_id)))
	&& (!(*input).dev_auth_first_auth_present || encode_repeated_dev_auth_first_auth(state, (&(*input).dev_auth_first_auth)))
	&& (!(*input).dev_auth_auth_time_present || encode_repeated_dev_auth_auth_time(state, (&(*input).dev_auth_auth_time)))
	&& (!(*input).dev_auth_auth_count_present || encode_repeated_dev_auth_auth_count(state, (&(*input).dev_auth_auth_count)))
	&& (!(*input).dev_auth_challenge_present || encode_repeated_dev_auth_challenge(state, (&(*input).dev_auth_challenge)))
	&& (!(*input).dev_auth_signature_present || encode_repeated_dev_auth_signature(state, (&(*input).dev_auth_signature)))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 7))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}



int cbor_encode_dev_auth(
		uint8_t *payload, size_t payload_len,
		const struct dev_auth *input,
		size_t *payload_len_out)
{
	zcbor_state_t states[4];

	return zcbor_entry_function(payload, payload_len, (void *)input, payload_len_out, states,
		(zcbor_decoder_t *)encode_dev_auth, sizeof(states) / sizeof(zcbor_state_t), 1);
}
