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
#include "eip4527_decode.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif

static bool decode_uuid(zcbor_state_t *state, struct zcbor_string *result);
static bool decode_repeated_eth_sign_request_request_id(zcbor_state_t *state, struct eth_sign_request_request_id *result);
static bool decode_sign_data_type(zcbor_state_t *state, struct sign_data_type_ *result);
static bool decode_repeated_eth_sign_request_chain_id(zcbor_state_t *state, struct eth_sign_request_chain_id *result);
static bool decode_path_component(zcbor_state_t *state, struct path_component *result);
static bool decode_repeated_crypto_keypath_source_fingerprint(zcbor_state_t *state, struct crypto_keypath_source_fingerprint *result);
static bool decode_repeated_crypto_keypath_depth(zcbor_state_t *state, struct crypto_keypath_depth *result);
static bool decode_crypto_keypath(zcbor_state_t *state, struct crypto_keypath *result);
static bool decode_repeated_eth_sign_request_address(zcbor_state_t *state, struct eth_sign_request_address *result);
static bool decode_repeated_eth_sign_request_request_origin(zcbor_state_t *state, struct eth_sign_request_request_origin *result);
static bool decode_eth_sign_request(zcbor_state_t *state, struct eth_sign_request *result);


static bool decode_uuid(
		zcbor_state_t *state, struct zcbor_string *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((zcbor_tag_expect(state, 37)
	&& (zcbor_bstr_decode(state, (&(*result))))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_repeated_eth_sign_request_request_id(
		zcbor_state_t *state, struct eth_sign_request_request_id *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (1))))
	&& (decode_uuid(state, (&(*result)._eth_sign_request_request_id)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_sign_data_type(
		zcbor_state_t *state, struct sign_data_type_ *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = (((((zcbor_int_decode(state, &(*result)._sign_data_type_choice, sizeof((*result)._sign_data_type_choice)))) && ((((((*result)._sign_data_type_choice == _sign_data_type__eth_transaction_data) && ((1)))
	|| (((*result)._sign_data_type_choice == _sign_data_type__eth_typed_data) && ((1)))
	|| (((*result)._sign_data_type_choice == _sign_data_type__eth_raw_bytes) && ((1)))
	|| (((*result)._sign_data_type_choice == _sign_data_type__eth_typed_transaction) && ((1)))) || (zcbor_error(state, ZCBOR_ERR_WRONG_VALUE), false))))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_repeated_eth_sign_request_chain_id(
		zcbor_state_t *state, struct eth_sign_request_chain_id *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (4))))
	&& (zcbor_int32_decode(state, (&(*result)._eth_sign_request_chain_id)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_path_component(
		zcbor_state_t *state, struct path_component *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = (((((zcbor_uint32_decode(state, (&(*result)._path_component__child_index)))
	&& ((((((((((*result)._path_component__child_index <= 4294967295)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false)))
	&& ((zcbor_bool_decode(state, (&(*result)._path_component__is_hardened)))))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_repeated_crypto_keypath_source_fingerprint(
		zcbor_state_t *state, struct crypto_keypath_source_fingerprint *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (2))))
	&& (zcbor_uint32_decode(state, (&(*result)._crypto_keypath_source_fingerprint)))
	&& ((((((*result)._crypto_keypath_source_fingerprint <= 4294967295)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_repeated_crypto_keypath_depth(
		zcbor_state_t *state, struct crypto_keypath_depth *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (3))))
	&& (zcbor_uint32_decode(state, (&(*result)._crypto_keypath_depth)))
	&& ((((((*result)._crypto_keypath_depth <= 255)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_crypto_keypath(
		zcbor_state_t *state, struct crypto_keypath *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_decode(state) && (((((zcbor_uint32_expect(state, (1))))
	&& (zcbor_list_start_decode(state) && ((zcbor_multi_decode(0, 10, &(*result)._crypto_keypath_components__path_component_count, (zcbor_decoder_t *)decode_path_component, state, (&(*result)._crypto_keypath_components__path_component), sizeof(struct path_component))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state)))
	&& zcbor_present_decode(&((*result)._crypto_keypath_source_fingerprint_present), (zcbor_decoder_t *)decode_repeated_crypto_keypath_source_fingerprint, state, (&(*result)._crypto_keypath_source_fingerprint))
	&& zcbor_present_decode(&((*result)._crypto_keypath_depth_present), (zcbor_decoder_t *)decode_repeated_crypto_keypath_depth, state, (&(*result)._crypto_keypath_depth))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_map_end_decode(state))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_repeated_eth_sign_request_address(
		zcbor_state_t *state, struct eth_sign_request_address *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (6))))
	&& (zcbor_bstr_decode(state, (&(*result)._eth_sign_request_address)))
	&& ((((((*result)._eth_sign_request_address.len >= 20)
	&& ((*result)._eth_sign_request_address.len <= 20)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_repeated_eth_sign_request_request_origin(
		zcbor_state_t *state, struct eth_sign_request_request_origin *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_expect(state, (7))))
	&& (zcbor_tstr_decode(state, (&(*result)._eth_sign_request_request_origin)))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}

static bool decode_eth_sign_request(
		zcbor_state_t *state, struct eth_sign_request *result)
{
	zcbor_print("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_decode(state) && ((zcbor_present_decode(&((*result)._eth_sign_request_request_id_present), (zcbor_decoder_t *)decode_repeated_eth_sign_request_request_id, state, (&(*result)._eth_sign_request_request_id))
	&& (((zcbor_uint32_expect(state, (2))))
	&& (zcbor_bstr_decode(state, (&(*result)._eth_sign_request_sign_data))))
	&& (((zcbor_uint32_expect(state, (3))))
	&& (decode_sign_data_type(state, (&(*result)._eth_sign_request_data_type))))
	&& zcbor_present_decode(&((*result)._eth_sign_request_chain_id_present), (zcbor_decoder_t *)decode_repeated_eth_sign_request_chain_id, state, (&(*result)._eth_sign_request_chain_id))
	&& (((zcbor_uint32_expect(state, (5))))
	&& zcbor_tag_expect(state, 304)
	&& (decode_crypto_keypath(state, (&(*result)._eth_sign_request_derivation_path))))
	&& zcbor_present_decode(&((*result)._eth_sign_request_address_present), (zcbor_decoder_t *)decode_repeated_eth_sign_request_address, state, (&(*result)._eth_sign_request_address))
	&& zcbor_present_decode(&((*result)._eth_sign_request_request_origin_present), (zcbor_decoder_t *)decode_repeated_eth_sign_request_request_origin, state, (&(*result)._eth_sign_request_request_origin))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_map_end_decode(state))));

	if (!tmp_result)
		zcbor_trace();

	return tmp_result;
}



int cbor_decode_eth_sign_request(
		const uint8_t *payload, size_t payload_len,
		struct eth_sign_request *result,
		size_t *payload_len_out)
{
	zcbor_state_t states[5];

	zcbor_new_state(states, sizeof(states) / sizeof(zcbor_state_t), payload, payload_len, 1);

	bool ret = decode_eth_sign_request(states, result);

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
