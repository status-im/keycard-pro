/*
 * Generated using zcbor version 0.6.0
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_decode.h"
#include "auth_decode.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif

static bool decode_dev_auth_step_type(zcbor_state_t *state, struct dev_auth_step_type_ *result);
static bool decode_uuid(zcbor_state_t *state, struct zcbor_string *result);
static bool decode_repeated_dev_auth_device_id(zcbor_state_t *state, struct dev_auth_device_id *result);
static bool decode_repeated_dev_auth_first_auth(zcbor_state_t *state, struct dev_auth_first_auth *result);
static bool decode_repeated_dev_auth_auth_count(zcbor_state_t *state, struct dev_auth_auth_count *result);
static bool decode_repeated_dev_auth_challenge(zcbor_state_t *state, struct dev_auth_challenge *result);
static bool decode_repeated_dev_auth_signature(zcbor_state_t *state, struct dev_auth_signature *result);
static bool decode_dev_auth(zcbor_state_t *state, struct dev_auth *result);


static bool decode_dev_auth_step_type(
		zcbor_state_t *state, struct dev_auth_step_type_ *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = (((((zcbor_int_decode(state, &(*result)._dev_auth_step_type_choice, sizeof((*result)._dev_auth_step_type_choice)))) && ((((((*result)._dev_auth_step_type_choice == _dev_auth_step_type__dev_auth_init) && ((1)))
	|| (((*result)._dev_auth_step_type_choice == _dev_auth_step_type__dev_auth_kpro) && ((1)))
	|| (((*result)._dev_auth_step_type_choice == _dev_auth_step_type__dev_auth_server) && ((1)))) || (zcbor_error(state, ZCBOR_ERR_WRONG_VALUE), false))))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_uuid(
		zcbor_state_t *state, struct zcbor_string *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((zcbor_tag_expect(state, 37)
	&& (zcbor_bstr_decode(state, (&(*result))))
	&& ((((*result).len >= 16)
	&& ((*result).len <= 16)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_repeated_dev_auth_device_id(
		zcbor_state_t *state, struct dev_auth_device_id *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (2))))
	&& (decode_uuid(state, (&(*result)._dev_auth_device_id)))
	&& ((((((*result)._dev_auth_device_id.len >= 16)
	&& ((*result)._dev_auth_device_id.len <= 16)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_repeated_dev_auth_first_auth(
		zcbor_state_t *state, struct dev_auth_first_auth *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (3))))
	&& (zcbor_uint32_decode(state, (&(*result)._dev_auth_first_auth)))
	&& ((((((*result)._dev_auth_first_auth <= 4294967295)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_repeated_dev_auth_auth_count(
		zcbor_state_t *state, struct dev_auth_auth_count *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (4))))
	&& (zcbor_int32_decode(state, (&(*result)._dev_auth_auth_count)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_repeated_dev_auth_challenge(
		zcbor_state_t *state, struct dev_auth_challenge *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (5))))
	&& (zcbor_bstr_decode(state, (&(*result)._dev_auth_challenge)))
	&& ((((*result)._dev_auth_challenge.len >= 32)
	&& ((*result)._dev_auth_challenge.len <= 32)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_repeated_dev_auth_signature(
		zcbor_state_t *state, struct dev_auth_signature *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (6))))
	&& (zcbor_bstr_decode(state, (&(*result)._dev_auth_signature)))
	&& ((((*result)._dev_auth_signature.len >= 64)
	&& ((*result)._dev_auth_signature.len <= 64)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_dev_auth(
		zcbor_state_t *state, struct dev_auth *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_decode(state) && (((((zcbor_uint32_expect(state, (1))))
	&& (decode_dev_auth_step_type(state, (&(*result)._dev_auth_step))))
	&& zcbor_present_decode(&((*result)._dev_auth_device_id_present), (zcbor_decoder_t *)decode_repeated_dev_auth_device_id, state, (&(*result)._dev_auth_device_id))
	&& zcbor_present_decode(&((*result)._dev_auth_first_auth_present), (zcbor_decoder_t *)decode_repeated_dev_auth_first_auth, state, (&(*result)._dev_auth_first_auth))
	&& zcbor_present_decode(&((*result)._dev_auth_auth_count_present), (zcbor_decoder_t *)decode_repeated_dev_auth_auth_count, state, (&(*result)._dev_auth_auth_count))
	&& zcbor_present_decode(&((*result)._dev_auth_challenge_present), (zcbor_decoder_t *)decode_repeated_dev_auth_challenge, state, (&(*result)._dev_auth_challenge))
	&& zcbor_present_decode(&((*result)._dev_auth_signature_present), (zcbor_decoder_t *)decode_repeated_dev_auth_signature, state, (&(*result)._dev_auth_signature))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_map_end_decode(state))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}



int cbor_decode_dev_auth(
		const uint8_t *payload, size_t payload_len,
		struct dev_auth *result,
		size_t *payload_len_out)
{
	zcbor_state_t states[4];

	zcbor_new_state(states, sizeof(states) / sizeof(zcbor_state_t), payload, payload_len, 1);

	bool ret = decode_dev_auth(states, result);

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
