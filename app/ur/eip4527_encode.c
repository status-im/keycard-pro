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
#include "eip4527_encode.h"
#include "zcbor_print.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif

static bool encode_uuid(zcbor_state_t *state, const struct zcbor_string *input);
static bool encode_repeated_eth_signature_request_id(zcbor_state_t *state, const struct eth_signature_request_id *input);
static bool encode_repeated_eth_signature_signature_origin(zcbor_state_t *state, const struct eth_signature_signature_origin *input);
static bool encode_repeated_eth_sign_request_request_id(zcbor_state_t *state, const struct eth_sign_request_request_id *input);
static bool encode_sign_data_type(zcbor_state_t *state, const struct sign_data_type_r *input);
static bool encode_repeated_eth_sign_request_chain_id(zcbor_state_t *state, const struct eth_sign_request_chain_id *input);
static bool encode_path_component(zcbor_state_t *state, const struct path_component *input);
static bool encode_repeated_crypto_keypath_source_fingerprint(zcbor_state_t *state, const struct crypto_keypath_source_fingerprint *input);
static bool encode_repeated_crypto_keypath_depth(zcbor_state_t *state, const struct crypto_keypath_depth *input);
static bool encode_crypto_keypath(zcbor_state_t *state, const struct crypto_keypath *input);
static bool encode_repeated_eth_sign_request_address(zcbor_state_t *state, const struct eth_sign_request_address *input);
static bool encode_repeated_eth_sign_request_request_origin(zcbor_state_t *state, const struct eth_sign_request_request_origin *input);
static bool encode_coininfo(zcbor_state_t *state, const struct coininfo *input);
static bool encode_repeated_hd_key_use_info(zcbor_state_t *state, const struct hd_key_use_info *input);
static bool encode_repeated_hd_key_source(zcbor_state_t *state, const struct hd_key_source *input);
static bool encode_repeated_crypto_multi_accounts_device(zcbor_state_t *state, const struct crypto_multi_accounts_device *input);
static bool encode_repeated_crypto_multi_accounts_device_id(zcbor_state_t *state, const struct crypto_multi_accounts_device_id *input);
static bool encode_repeated_crypto_multi_accounts_version(zcbor_state_t *state, const struct crypto_multi_accounts_version *input);
static bool encode_crypto_multi_accounts(zcbor_state_t *state, const struct crypto_multi_accounts *input);
static bool encode_hd_key(zcbor_state_t *state, const struct hd_key *input);
static bool encode_eth_signature(zcbor_state_t *state, const struct eth_signature *input);
static bool encode_eth_sign_request(zcbor_state_t *state, const struct eth_sign_request *input);


static bool encode_uuid(
		zcbor_state_t *state, const struct zcbor_string *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((zcbor_tag_put(state, 37)
	&& (zcbor_bstr_encode(state, (&(*input))))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_eth_signature_request_id(
		zcbor_state_t *state, const struct eth_signature_request_id *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (1))))
	&& (encode_uuid(state, (&(*input).eth_signature_request_id)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_eth_signature_signature_origin(
		zcbor_state_t *state, const struct eth_signature_signature_origin *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (3))))
	&& (zcbor_tstr_encode(state, (&(*input).eth_signature_signature_origin)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_eth_sign_request_request_id(
		zcbor_state_t *state, const struct eth_sign_request_request_id *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (1))))
	&& (encode_uuid(state, (&(*input).eth_sign_request_request_id)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_sign_data_type(
		zcbor_state_t *state, const struct sign_data_type_r *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((((*input).sign_data_type_choice == sign_data_type_eth_transaction_data_m_c) ? ((zcbor_uint32_put(state, (1))))
	: (((*input).sign_data_type_choice == sign_data_type_eth_typed_data_m_c) ? ((zcbor_uint32_put(state, (2))))
	: (((*input).sign_data_type_choice == sign_data_type_eth_raw_bytes_m_c) ? ((zcbor_uint32_put(state, (3))))
	: (((*input).sign_data_type_choice == sign_data_type_eth_typed_transaction_m_c) ? ((zcbor_uint32_put(state, (4))))
	: false))))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_eth_sign_request_chain_id(
		zcbor_state_t *state, const struct eth_sign_request_chain_id *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (4))))
	&& (zcbor_int32_encode(state, (&(*input).eth_sign_request_chain_id)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_path_component(
		zcbor_state_t *state, const struct path_component *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((((((((((((*input).path_component_child_index_m <= UINT32_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input).path_component_child_index_m))))
	&& ((zcbor_bool_encode(state, (&(*input).path_component_is_hardened_m)))))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_crypto_keypath_source_fingerprint(
		zcbor_state_t *state, const struct crypto_keypath_source_fingerprint *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (2))))
	&& ((((((*input).crypto_keypath_source_fingerprint <= UINT32_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input).crypto_keypath_source_fingerprint)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_crypto_keypath_depth(
		zcbor_state_t *state, const struct crypto_keypath_depth *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (3))))
	&& ((((((*input).crypto_keypath_depth <= UINT8_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input).crypto_keypath_depth)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_crypto_keypath(
		zcbor_state_t *state, const struct crypto_keypath *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_encode(state, 3) && (((((zcbor_uint32_put(state, (1))))
	&& (zcbor_list_start_encode(state, 20) && ((zcbor_multi_encode_minmax(0, 10, &(*input).crypto_keypath_components_path_component_m_count, (zcbor_encoder_t *)encode_path_component, state, (&(*input).crypto_keypath_components_path_component_m), sizeof(struct path_component))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 20)))
	&& (!(*input).crypto_keypath_source_fingerprint_present || encode_repeated_crypto_keypath_source_fingerprint(state, (&(*input).crypto_keypath_source_fingerprint)))
	&& (!(*input).crypto_keypath_depth_present || encode_repeated_crypto_keypath_depth(state, (&(*input).crypto_keypath_depth)))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 3))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_eth_sign_request_address(
		zcbor_state_t *state, const struct eth_sign_request_address *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (6))))
	&& ((((((*input).eth_sign_request_address.len >= 20)
	&& ((*input).eth_sign_request_address.len <= 20)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input).eth_sign_request_address)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_eth_sign_request_request_origin(
		zcbor_state_t *state, const struct eth_sign_request_request_origin *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (7))))
	&& (zcbor_tstr_encode(state, (&(*input).eth_sign_request_request_origin)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_coininfo(
		zcbor_state_t *state, const struct coininfo *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_encode(state, 2) && (((((zcbor_uint32_put(state, (1))))
	&& ((((((((*input).coininfo_coin_type <= UINT32_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input).coininfo_coin_type))))
	&& (((zcbor_uint32_put(state, (2))))
	&& (zcbor_int32_encode(state, (&(*input).coininfo_network))))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 2))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_hd_key_use_info(
		zcbor_state_t *state, const struct hd_key_use_info *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (5))))
	&& zcbor_tag_put(state, 305)
	&& (encode_coininfo(state, (&(*input).hd_key_use_info)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_hd_key_source(
		zcbor_state_t *state, const struct hd_key_source *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (10))))
	&& (zcbor_tstr_encode(state, (&(*input).hd_key_source)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_crypto_multi_accounts_device(
		zcbor_state_t *state, const struct crypto_multi_accounts_device *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (3))))
	&& (zcbor_tstr_encode(state, (&(*input).crypto_multi_accounts_device)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_crypto_multi_accounts_device_id(
		zcbor_state_t *state, const struct crypto_multi_accounts_device_id *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (4))))
	&& (zcbor_tstr_encode(state, (&(*input).crypto_multi_accounts_device_id)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_repeated_crypto_multi_accounts_version(
		zcbor_state_t *state, const struct crypto_multi_accounts_version *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = ((((zcbor_uint32_put(state, (5))))
	&& (zcbor_tstr_encode(state, (&(*input).crypto_multi_accounts_version)))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}
static bool encode_crypto_multi_accounts_key(
    zcbor_state_t *state, const struct hd_key *input)
{
  zcbor_tag_put(state, 303);
  return encode_hd_key(state, input);
}

static bool encode_crypto_multi_accounts(
		zcbor_state_t *state, const struct crypto_multi_accounts *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_encode(state, 5) && (((((zcbor_uint32_put(state, (1))))
	&& ((((((*input).crypto_multi_accounts_master_fingerprint <= UINT32_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input).crypto_multi_accounts_master_fingerprint))))
	&& (((zcbor_uint32_put(state, (2))))
	&& (zcbor_list_start_encode(state, 10) && ((zcbor_multi_encode_minmax(0, 10, &(*input).crypto_multi_accounts_keys_hd_key_m_count, (zcbor_encoder_t *)encode_crypto_multi_accounts_key, state, (&(*input).crypto_multi_accounts_keys_hd_key_m), sizeof(struct hd_key))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 10)))
	&& (!(*input).crypto_multi_accounts_device_present || encode_repeated_crypto_multi_accounts_device(state, (&(*input).crypto_multi_accounts_device)))
	&& (!(*input).crypto_multi_accounts_device_id_present || encode_repeated_crypto_multi_accounts_device_id(state, (&(*input).crypto_multi_accounts_device_id)))
	&& (!(*input).crypto_multi_accounts_version_present || encode_repeated_crypto_multi_accounts_version(state, (&(*input).crypto_multi_accounts_version)))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 5))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_hd_key(
		zcbor_state_t *state, const struct hd_key *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_encode(state, 9) && (((((zcbor_uint32_put(state, (1))))
	&& (zcbor_bool_encode(state, (&(*input).hd_key_is_master))))
	&& (((zcbor_uint32_put(state, (2))))
	&& (zcbor_bool_encode(state, (&(*input).hd_key_is_private))))
	&& (((zcbor_uint32_put(state, (3))))
	&& ((((((*input).hd_key_key_data.len >= 33)
	&& ((*input).hd_key_key_data.len <= 33)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input).hd_key_key_data))))
	&& (((zcbor_uint32_put(state, (4))))
	&& ((((((*input).hd_key_chain_code.len >= 32)
	&& ((*input).hd_key_chain_code.len <= 32)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input).hd_key_chain_code))))
	&& (!(*input).hd_key_use_info_present || encode_repeated_hd_key_use_info(state, (&(*input).hd_key_use_info)))
	&& (((zcbor_uint32_put(state, (6))))
	&& zcbor_tag_put(state, 304)
	&& (encode_crypto_keypath(state, (&(*input).hd_key_origin))))
	&& (((zcbor_uint32_put(state, (8))))
	&& ((((((*input).hd_key_parent_fingerprint <= UINT32_MAX)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_uint32_encode(state, (&(*input).hd_key_parent_fingerprint))))
	&& (((zcbor_uint32_put(state, (9))))
	&& (zcbor_tstr_encode(state, (&(*input).hd_key_name))))
	&& (!(*input).hd_key_source_present || encode_repeated_hd_key_source(state, (&(*input).hd_key_source)))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 9))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_eth_signature(
		zcbor_state_t *state, const struct eth_signature *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_encode(state, 3) && (((!(*input).eth_signature_request_id_present || encode_repeated_eth_signature_request_id(state, (&(*input).eth_signature_request_id)))
	&& (((zcbor_uint32_put(state, (2))))
	&& (zcbor_bstr_encode(state, (&(*input).eth_signature_signature))))
	&& (!(*input).eth_signature_signature_origin_present || encode_repeated_eth_signature_signature_origin(state, (&(*input).eth_signature_signature_origin)))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 3))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}

static bool encode_eth_sign_request(
		zcbor_state_t *state, const struct eth_sign_request *input)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((zcbor_map_start_encode(state, 7) && (((!(*input).eth_sign_request_request_id_present || encode_repeated_eth_sign_request_request_id(state, (&(*input).eth_sign_request_request_id)))
	&& (((zcbor_uint32_put(state, (2))))
	&& (zcbor_bstr_encode(state, (&(*input).eth_sign_request_sign_data))))
	&& (((zcbor_uint32_put(state, (3))))
	&& (encode_sign_data_type(state, (&(*input).eth_sign_request_data_type))))
	&& (!(*input).eth_sign_request_chain_id_present || encode_repeated_eth_sign_request_chain_id(state, (&(*input).eth_sign_request_chain_id)))
	&& (((zcbor_uint32_put(state, (5))))
	&& zcbor_tag_put(state, 304)
	&& (encode_crypto_keypath(state, (&(*input).eth_sign_request_derivation_path))))
	&& (!(*input).eth_sign_request_address_present || encode_repeated_eth_sign_request_address(state, (&(*input).eth_sign_request_address)))
	&& (!(*input).eth_sign_request_request_origin_present || encode_repeated_eth_sign_request_request_origin(state, (&(*input).eth_sign_request_request_origin)))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 7))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}



int cbor_encode_eth_sign_request(
		uint8_t *payload, size_t payload_len,
		const struct eth_sign_request *input,
		size_t *payload_len_out)
{
	zcbor_state_t states[5];

	return zcbor_entry_function(payload, payload_len, (void *)input, payload_len_out, states,
		(zcbor_decoder_t *)encode_eth_sign_request, sizeof(states) / sizeof(zcbor_state_t), 1);
}


int cbor_encode_eth_signature(
		uint8_t *payload, size_t payload_len,
		const struct eth_signature *input,
		size_t *payload_len_out)
{
	zcbor_state_t states[3];

	return zcbor_entry_function(payload, payload_len, (void *)input, payload_len_out, states,
		(zcbor_decoder_t *)encode_eth_signature, sizeof(states) / sizeof(zcbor_state_t), 1);
}


int cbor_encode_hd_key(
		uint8_t *payload, size_t payload_len,
		const struct hd_key *input,
		size_t *payload_len_out)
{
	zcbor_state_t states[5];

	return zcbor_entry_function(payload, payload_len, (void *)input, payload_len_out, states,
		(zcbor_decoder_t *)encode_hd_key, sizeof(states) / sizeof(zcbor_state_t), 1);
}


int cbor_encode_crypto_multi_accounts(
		uint8_t *payload, size_t payload_len,
		const struct crypto_multi_accounts *input,
		size_t *payload_len_out)
{
	zcbor_state_t states[7];

	return zcbor_entry_function(payload, payload_len, (void *)input, payload_len_out, states,
		(zcbor_decoder_t *)encode_crypto_multi_accounts, sizeof(states) / sizeof(zcbor_state_t), 1);
}
