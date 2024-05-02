/*
 * Generated using zcbor version 0.8.1
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef EIP4527_TYPES_H__
#define EIP4527_TYPES_H__

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

struct eth_sign_request_request_id {

  struct zcbor_string eth_sign_request_request_id;

};



struct sign_data_type_r {

  enum {

    sign_data_type_eth_transaction_data_m_c = 1,

    sign_data_type_eth_typed_data_m_c = 2,

    sign_data_type_eth_raw_bytes_m_c = 3,

    sign_data_type_eth_typed_transaction_m_c = 4,

  } sign_data_type_choice;

};



struct eth_sign_request_chain_id {

  int32_t eth_sign_request_chain_id;

};



struct path_component {

  uint32_t path_component_child_index_m;

  bool path_component_is_hardened_m;

};



struct crypto_keypath_source_fingerprint {

  uint32_t crypto_keypath_source_fingerprint;

};



struct crypto_keypath_depth {

  uint32_t crypto_keypath_depth;

};



struct crypto_keypath {

  struct path_component crypto_keypath_components_path_component_m[10];

  size_t crypto_keypath_components_path_component_m_count;

  struct crypto_keypath_source_fingerprint crypto_keypath_source_fingerprint;

  bool crypto_keypath_source_fingerprint_present;

  struct crypto_keypath_depth crypto_keypath_depth;

  bool crypto_keypath_depth_present;

};



struct eth_sign_request_address {

  struct zcbor_string eth_sign_request_address;

};



struct eth_sign_request_request_origin {

  struct zcbor_string eth_sign_request_request_origin;

};



struct eth_sign_request {

  struct eth_sign_request_request_id eth_sign_request_request_id;

  bool eth_sign_request_request_id_present;

  struct zcbor_string eth_sign_request_sign_data;

  struct sign_data_type_r eth_sign_request_data_type;

  struct eth_sign_request_chain_id eth_sign_request_chain_id;

  bool eth_sign_request_chain_id_present;

  struct crypto_keypath eth_sign_request_derivation_path;

  struct eth_sign_request_address eth_sign_request_address;

  bool eth_sign_request_address_present;

  struct eth_sign_request_request_origin eth_sign_request_request_origin;

  bool eth_sign_request_request_origin_present;

};

struct eth_signature_request_id {

  struct zcbor_string eth_signature_request_id;

};



struct eth_signature_signature_origin {

  struct zcbor_string eth_signature_signature_origin;

};



struct eth_signature {

  struct eth_signature_request_id eth_signature_request_id;

  bool eth_signature_request_id_present;

  struct zcbor_string eth_signature_signature;

  struct eth_signature_signature_origin eth_signature_signature_origin;

  bool eth_signature_signature_origin_present;

};



struct coininfo {

  uint32_t coininfo_coin_type;

  int32_t coininfo_network;

};



struct hd_key_use_info {

  struct coininfo hd_key_use_info;

};



struct hd_key {

  bool hd_key_is_master;

  bool hd_key_is_private;

  struct zcbor_string hd_key_key_data;

  struct zcbor_string hd_key_chain_code;

  struct hd_key_use_info hd_key_use_info;

  bool hd_key_use_info_present;

  struct crypto_keypath hd_key_origin;

  uint32_t hd_key_parent_fingerprint;

  struct zcbor_string hd_key_name;

  struct zcbor_string hd_key_source;

};



struct crypto_multi_accounts_device {

  struct zcbor_string crypto_multi_accounts_device;

};



struct crypto_multi_accounts_device_id {

  struct zcbor_string crypto_multi_accounts_device_id;

};



struct crypto_multi_accounts_version {

  struct zcbor_string crypto_multi_accounts_version;

};



struct crypto_multi_accounts {

  uint32_t crypto_multi_accounts_master_fingerprint;

  struct hd_key crypto_multi_accounts_keys_hd_key_m[10];

  size_t crypto_multi_accounts_keys_hd_key_m_count;

  struct crypto_multi_accounts_device crypto_multi_accounts_device;

  bool crypto_multi_accounts_device_present;

  struct crypto_multi_accounts_device_id crypto_multi_accounts_device_id;

  bool crypto_multi_accounts_device_id_present;

  struct crypto_multi_accounts_version crypto_multi_accounts_version;

  bool crypto_multi_accounts_version_present;

};


#ifdef __cplusplus
}
#endif

#endif /* EIP4527_TYPES_H__ */
