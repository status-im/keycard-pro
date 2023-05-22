/*
 * Generated using zcbor version 0.6.0
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef EIP4527_TYPES_H__
#define EIP4527_TYPES_H__

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

struct eth_signature_request_id {

  struct zcbor_string _eth_signature_request_id;

};



struct eth_signature_signature_origin {

  struct zcbor_string _eth_signature_signature_origin;

};



struct eth_signature {

  struct eth_signature_request_id _eth_signature_request_id;

  uint_fast32_t _eth_signature_request_id_present;

  struct zcbor_string _eth_signature_signature;

  struct eth_signature_signature_origin _eth_signature_signature_origin;

  uint_fast32_t _eth_signature_signature_origin_present;

};



struct hd_key_chain_code {

  struct zcbor_string _hd_key_chain_code;

};



struct eth_sign_request_chain_id {

	int32_t _eth_sign_request_chain_id;

};



struct path_component {

  uint32_t _path_component__child_index;

  bool _path_component__is_hardened;

};



struct crypto_keypath_source_fingerprint {

  uint32_t _crypto_keypath_source_fingerprint;

};



struct crypto_keypath_depth {

  uint32_t _crypto_keypath_depth;

};



struct crypto_keypath {

  struct path_component _crypto_keypath_components__path_component[10];

  uint_fast32_t _crypto_keypath_components__path_component_count;

  struct crypto_keypath_source_fingerprint _crypto_keypath_source_fingerprint;

  uint_fast32_t _crypto_keypath_source_fingerprint_present;

  struct crypto_keypath_depth _crypto_keypath_depth;

  uint_fast32_t _crypto_keypath_depth_present;

};



struct hd_key_origin {

  struct crypto_keypath _hd_key_origin;

};



struct hd_key_name {

  struct zcbor_string _hd_key_name;

};



struct hd_key_source {

  struct zcbor_string _hd_key_source;

};



struct hd_key {

  struct zcbor_string _hd_key_key_data;

  struct hd_key_chain_code _hd_key_chain_code;

  uint_fast32_t _hd_key_chain_code_present;

  struct hd_key_origin _hd_key_origin;

  uint_fast32_t _hd_key_origin_present;

  struct hd_key_name _hd_key_name;

  uint_fast32_t _hd_key_name_present;

  struct hd_key_source _hd_key_source;

  uint_fast32_t _hd_key_source_present;

};

struct eth_sign_request_request_id {

  struct zcbor_string _eth_sign_request_request_id;

};



struct sign_data_type_ {

  enum {

    _sign_data_type__eth_transaction_data = 1,

    _sign_data_type__eth_typed_data = 2,

    _sign_data_type__eth_raw_bytes = 3,

    _sign_data_type__eth_typed_transaction = 4,

  } _sign_data_type_choice;

};



struct eth_sign_request_address {

  struct zcbor_string _eth_sign_request_address;

};



struct eth_sign_request_request_origin {

  struct zcbor_string _eth_sign_request_request_origin;

};



struct eth_sign_request {

	struct eth_sign_request_request_id _eth_sign_request_request_id;

	uint_fast32_t _eth_sign_request_request_id_present;

	struct zcbor_string _eth_sign_request_sign_data;

	struct sign_data_type_ _eth_sign_request_data_type;

	struct eth_sign_request_chain_id _eth_sign_request_chain_id;

	uint_fast32_t _eth_sign_request_chain_id_present;

	struct crypto_keypath _eth_sign_request_derivation_path;

	struct eth_sign_request_address _eth_sign_request_address;

	uint_fast32_t _eth_sign_request_address_present;

	struct eth_sign_request_request_origin _eth_sign_request_request_origin;

	uint_fast32_t _eth_sign_request_request_origin_present;

};

#endif /* EIP4527_TYPES_H__ */
