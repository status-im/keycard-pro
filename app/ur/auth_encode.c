/*
 * Generated using zcbor version 0.6.0
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_encode.h"
#include "auth_encode.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif

static bool encode_dev_auth_step_type(zcbor_state_t *state, const struct dev_auth_step_type_ *input);
static bool encode_uuid(zcbor_state_t *state, const struct zcbor_string *input);
static bool encode_repeated_dev_auth_device_id(zcbor_state_t *state, const struct dev_auth_device_id *input);
static bool encode_repeated_dev_auth_first_auth(zcbor_state_t *state, const struct dev_auth_first_auth *input);
static bool encode_repeated_dev_auth_auth_time(zcbor_state_t *state, const struct dev_auth_auth_time *input);
static bool encode_repeated_dev_auth_auth_count(zcbor_state_t *state, const struct dev_auth_auth_count *input);
static bool encode_repeated_dev_auth_challenge(zcbor_state_t *state, const struct dev_auth_challenge *input);
static bool encode_repeated_dev_auth_signature(zcbor_state_t *state, const struct dev_auth_signature *input);
static bool encode_dev_auth(zcbor_state_t *state, const struct dev_auth *input);


static bool encode_dev_auth_step_type(
		zcbor_state_t *state, const struct dev_auth_step_type_ *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = (((((*input)._dev_auth_step_type_choice == _dev_auth_step_type__dev_auth_init) ? ((zcbor_uint32_put(state, (1))))
	: (((*input)._dev_auth_step_type_choice == _dev_auth_step_type__dev_auth_device) ? ((zcbor_uint32_put(state, (2))))
	: (((*input)._dev_auth_step_type_choice == _dev_auth_step_type__dev_auth_server) ? ((zcbor_uint32_put(state, (3))))
	: false)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool encode_uuid(
		zcbor_state_t *state, const struct zcbor_string *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((zcbor_tag_encode(state, 37)
	&& ((((*input).len >= 16)
	&& ((*input).len <= 16)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input))))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool encode_repeated_dev_auth_device_id(
		zcbor_state_t *state, const struct dev_auth_device_id *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (2))))
	&& ((((((*input)._dev_auth_device_id.len >= 16)
	&& ((*input)._dev_auth_device_id.len <= 16)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (encode_uuid(state, (&(*input)._dev_auth_device_id)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool encode_repeated_dev_auth_first_auth(
		zcbor_state_t *state, const struct dev_auth_first_auth *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (3))))
	&& ((((((*input)._dev_auth_first_auth <= 4294967295)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input)._dev_auth_first_auth)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool encode_repeated_dev_auth_auth_time(
		zcbor_state_t *state, const struct dev_auth_auth_time *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (4))))
	&& ((((((*input)._dev_auth_auth_time <= 4294967295)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input)._dev_auth_auth_time)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool encode_repeated_dev_auth_auth_count(
		zcbor_state_t *state, const struct dev_auth_auth_count *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (5))))
	&& ((((((*input)._dev_auth_auth_count <= 4294967295)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input)._dev_auth_auth_count)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool encode_repeated_dev_auth_challenge(
		zcbor_state_t *state, const struct dev_auth_challenge *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (6))))
	&& ((((*input)._dev_auth_challenge.len >= 32)
	&& ((*input)._dev_auth_challenge.len <= 32)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input)._dev_auth_challenge)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool encode_repeated_dev_auth_signature(
		zcbor_state_t *state, const struct dev_auth_signature *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (7))))
	&& ((((*input)._dev_auth_signature.len >= 64)
	&& ((*input)._dev_auth_signature.len <= 64)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input)._dev_auth_signature)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool encode_dev_auth(
		zcbor_state_t *state, const struct dev_auth *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_encode(state, 7) && (((((zcbor_uint32_put(state, (1))))
	&& (encode_dev_auth_step_type(state, (&(*input)._dev_auth_step))))
	&& zcbor_present_encode(&((*input)._dev_auth_device_id_present), (zcbor_encoder_t *)encode_repeated_dev_auth_device_id, state, (&(*input)._dev_auth_device_id))
	&& zcbor_present_encode(&((*input)._dev_auth_first_auth_present), (zcbor_encoder_t *)encode_repeated_dev_auth_first_auth, state, (&(*input)._dev_auth_first_auth))
	&& zcbor_present_encode(&((*input)._dev_auth_auth_time_present), (zcbor_encoder_t *)encode_repeated_dev_auth_auth_time, state, (&(*input)._dev_auth_auth_time))
	&& zcbor_present_encode(&((*input)._dev_auth_auth_count_present), (zcbor_encoder_t *)encode_repeated_dev_auth_auth_count, state, (&(*input)._dev_auth_auth_count))
	&& zcbor_present_encode(&((*input)._dev_auth_challenge_present), (zcbor_encoder_t *)encode_repeated_dev_auth_challenge, state, (&(*input)._dev_auth_challenge))
	&& zcbor_present_encode(&((*input)._dev_auth_signature_present), (zcbor_encoder_t *)encode_repeated_dev_auth_signature, state, (&(*input)._dev_auth_signature))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 7))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}



int cbor_encode_dev_auth(
		uint8_t *payload, size_t payload_len,
		const struct dev_auth *input,
		size_t *payload_len_out)
{
	zcbor_state_t states[4];

	zcbor_new_state(states, sizeof(states) / sizeof(zcbor_state_t), payload, payload_len, 1);

	bool ret = encode_dev_auth(states, input);

	if (ret && (payload_len_out != NULL)) {
		*payload_len_out = MIN(payload_len,
				(size_t)states[0].payload - (size_t)payload);
	}

	if (!ret) {
		int err = zcbor_pop_error(states);

		zcbor_print("Return error: %d\r\n", err);
		return (err == ZCBOR_SUCCESS) ? ZCBOR_ERR_UNKNOWN : err;
	}
	return ZCBOR_SUCCESS;
}
