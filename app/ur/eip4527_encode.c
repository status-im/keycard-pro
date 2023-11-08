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
#include "eip4527_encode.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif

static bool encode_uuid(zcbor_state_t *state, const struct zcbor_string *input);

static bool encode_repeated_eth_signature_request_id(zcbor_state_t *state, const struct eth_signature_request_id *input);

static bool encode_repeated_eth_signature_signature_origin(zcbor_state_t *state, const struct eth_signature_signature_origin *input);

static bool encode_repeated_hd_key_chain_code(zcbor_state_t *state, const struct hd_key_chain_code *input);

static bool encode_path_component(zcbor_state_t *state, const struct path_component *input);

static bool encode_repeated_crypto_keypath_source_fingerprint(zcbor_state_t *state, const struct crypto_keypath_source_fingerprint *input);

static bool encode_repeated_crypto_keypath_depth(zcbor_state_t *state, const struct crypto_keypath_depth *input);

static bool encode_crypto_keypath(zcbor_state_t *state, const struct crypto_keypath *input);

static bool encode_repeated_hd_key_origin(zcbor_state_t *state, const struct hd_key_origin *input);

static bool encode_repeated_hd_key_name(zcbor_state_t *state, const struct hd_key_name *input);

static bool encode_repeated_hd_key_source(zcbor_state_t *state, const struct hd_key_source *input);

static bool encode_hd_key(zcbor_state_t *state, const struct hd_key *input);

static bool encode_eth_signature(zcbor_state_t *state, const struct eth_signature *input);


static bool encode_uuid(
		zcbor_state_t *state, const struct zcbor_string *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((zcbor_tag_encode(state, 37)
	&& (zcbor_bstr_encode(state, (&(*input))))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_repeated_eth_signature_request_id(
		zcbor_state_t *state, const struct eth_signature_request_id *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (1))))
	&& (encode_uuid(state, (&(*input)._eth_signature_request_id)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_repeated_eth_signature_signature_origin(
		zcbor_state_t *state, const struct eth_signature_signature_origin *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (3))))
	&& (zcbor_tstr_encode(state, (&(*input)._eth_signature_signature_origin)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_repeated_hd_key_chain_code(
		zcbor_state_t *state, const struct hd_key_chain_code *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (4))))
	&& ((((((*input)._hd_key_chain_code.len >= 32)
	&& ((*input)._hd_key_chain_code.len <= 32)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input)._hd_key_chain_code)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_path_component(
		zcbor_state_t *state, const struct path_component *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((((((((((((*input)._path_component__child_index <= 4294967295)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input)._path_component__child_index))))
	&& ((zcbor_bool_encode(state, (&(*input)._path_component__is_hardened)))))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_repeated_crypto_keypath_source_fingerprint(
		zcbor_state_t *state, const struct crypto_keypath_source_fingerprint *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (2))))
	&& ((((((*input)._crypto_keypath_source_fingerprint <= 4294967295)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input)._crypto_keypath_source_fingerprint)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_repeated_crypto_keypath_depth(
		zcbor_state_t *state, const struct crypto_keypath_depth *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (3))))
	&& ((((((*input)._crypto_keypath_depth <= 255)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input)._crypto_keypath_depth)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_crypto_keypath(
		zcbor_state_t *state, const struct crypto_keypath *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_encode(state, 3) && (((((zcbor_uint32_put(state, (1))))
	&& (zcbor_list_start_encode(state, 20) && ((zcbor_multi_encode_minmax(0, 10, &(*input)._crypto_keypath_components__path_component_count, (zcbor_encoder_t *)encode_path_component, state, (&(*input)._crypto_keypath_components__path_component), sizeof(struct path_component))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 20)))
	&& zcbor_present_encode(&((*input)._crypto_keypath_source_fingerprint_present), (zcbor_encoder_t *)encode_repeated_crypto_keypath_source_fingerprint, state, (&(*input)._crypto_keypath_source_fingerprint))
	&& zcbor_present_encode(&((*input)._crypto_keypath_depth_present), (zcbor_encoder_t *)encode_repeated_crypto_keypath_depth, state, (&(*input)._crypto_keypath_depth))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 3))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_repeated_hd_key_origin(
		zcbor_state_t *state, const struct hd_key_origin *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (6))))
	&& zcbor_tag_encode(state, 304)
	&& (encode_crypto_keypath(state, (&(*input)._hd_key_origin)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_repeated_hd_key_name(
		zcbor_state_t *state, const struct hd_key_name *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (9))))
	&& (zcbor_tstr_encode(state, (&(*input)._hd_key_name)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_repeated_hd_key_source(
		zcbor_state_t *state, const struct hd_key_source *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (10))))
	&& (zcbor_tstr_encode(state, (&(*input)._hd_key_source)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_hd_key(
		zcbor_state_t *state, const struct hd_key *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_encode(state, 5) && (((((zcbor_uint32_put(state, (3))))
	&& ((((((*input)._hd_key_key_data.len >= 33)
	&& ((*input)._hd_key_key_data.len <= 33)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input)._hd_key_key_data))))
	&& zcbor_present_encode(&((*input)._hd_key_chain_code_present), (zcbor_encoder_t *)encode_repeated_hd_key_chain_code, state, (&(*input)._hd_key_chain_code))
	&& zcbor_present_encode(&((*input)._hd_key_origin_present), (zcbor_encoder_t *)encode_repeated_hd_key_origin, state, (&(*input)._hd_key_origin))
	&& zcbor_present_encode(&((*input)._hd_key_name_present), (zcbor_encoder_t *)encode_repeated_hd_key_name, state, (&(*input)._hd_key_name))
	&& zcbor_present_encode(&((*input)._hd_key_source_present), (zcbor_encoder_t *)encode_repeated_hd_key_source, state, (&(*input)._hd_key_source))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 5))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}


static bool encode_eth_signature(
		zcbor_state_t *state, const struct eth_signature *input)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_encode(state, 3) && ((zcbor_present_encode(&((*input)._eth_signature_request_id_present), (zcbor_encoder_t *)encode_repeated_eth_signature_request_id, state, (&(*input)._eth_signature_request_id))
	&& (((zcbor_uint32_put(state, (2))))
	&& (zcbor_bstr_encode(state, (&(*input)._eth_signature_signature))))
	&& zcbor_present_encode(&((*input)._eth_signature_signature_origin_present), (zcbor_encoder_t *)encode_repeated_eth_signature_signature_origin, state, (&(*input)._eth_signature_signature_origin))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 3))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}



int cbor_encode_eth_signature(
		uint8_t *payload, size_t payload_len,
		const struct eth_signature *input,
		size_t *payload_len_out)
{
	zcbor_state_t states[3];

	zcbor_new_state(states, sizeof(states) / sizeof(zcbor_state_t), payload, payload_len, 1);

	bool ret = encode_eth_signature(states, input);

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



int cbor_encode_hd_key(
		uint8_t *payload, size_t payload_len,
		const struct hd_key *input,
		size_t *payload_len_out)
{
	zcbor_state_t states[5];

	zcbor_new_state(states, sizeof(states) / sizeof(zcbor_state_t), payload, payload_len, 1);

	bool ret = encode_hd_key(states, input);

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
