#include "common.h"
#include "core.h"
#include "crypto/ecdsa.h"
#include "crypto/sha2.h"
#include "crypto/secp256k1.h"
#include "crypto/rand.h"
#include "hal.h"
#include "ui/ui.h"
#include "ur/ur.h"
#include "ur/auth_types.h"
#include "ur/auth_encode.h"

#define AUTH_CHALLENGE_LEN 32
#define AUTH_SIG_LEN 64

void device_auth_run() {
  struct dev_auth auth;

  // Challenge QR
  if (ui_qrscan(DEV_AUTH, &auth) != CORE_EVT_UI_OK) {
    return;
  }

  if ((auth._dev_auth_step._dev_auth_step_type_choice != _dev_auth_step_type__dev_auth_init) ||
      !auth._dev_auth_challenge_present ||
      auth._dev_auth_device_id_present ||
      auth._dev_auth_first_auth_present ||
      auth._dev_auth_auth_time_present ||
      auth._dev_auth_auth_count_present ||
      auth._dev_auth_signature_present) {
    //TODO: error
    return;
  }

  SHA256_CTX sha256;

  uint8_t uid[HAL_DEVICE_UID_LEN];
  uint8_t digest[SHA256_DIGEST_LENGTH];

  hal_device_uid(uid);

  sha256_Init(&sha256);
  sha256_Update(&sha256, uid, HAL_DEVICE_UID_LEN);
  sha256_Update(&sha256, auth._dev_auth_challenge._dev_auth_challenge.value, auth._dev_auth_challenge._dev_auth_challenge.len);
  sha256_Final(&sha256, digest);

  //TODO: fetch the private key!
  ecdsa_sign_digest(&secp256k1, NULL, digest, g_core.data.sig.plain_sig, NULL, NULL);

  // Response
  random_buffer((uint8_t*) auth._dev_auth_challenge._dev_auth_challenge.value, AUTH_CHALLENGE_LEN);

  auth._dev_auth_device_id_present = 1;
  auth._dev_auth_device_id._dev_auth_device_id.value = uid;
  auth._dev_auth_device_id._dev_auth_device_id.len = HAL_DEVICE_UID_LEN;

  auth._dev_auth_step._dev_auth_step_type_choice = _dev_auth_step_type__dev_auth_device;
  auth._dev_auth_signature_present = 1;
  auth._dev_auth_signature._dev_auth_signature.value = g_core.data.sig.plain_sig;
  auth._dev_auth_signature._dev_auth_signature.len = AUTH_SIG_LEN;

  cbor_encode_dev_auth(g_core.data.sig.cbor_sig, CBOR_SIG_MAX_LEN, &auth, &g_core.data.sig.cbor_len);

  ui_display_qr(g_core.data.sig.cbor_sig, g_core.data.sig.cbor_len, DEV_AUTH);

  sha256_Init(&sha256);
  sha256_Update(&sha256, uid, HAL_DEVICE_UID_LEN);
  sha256_Update(&sha256, auth._dev_auth_challenge._dev_auth_challenge.value, auth._dev_auth_challenge._dev_auth_challenge.len);

  // Final QR
  if (ui_qrscan(DEV_AUTH, &auth) != CORE_EVT_UI_OK) {
    return;
  }

  if ((auth._dev_auth_step._dev_auth_step_type_choice != _dev_auth_step_type__dev_auth_server) ||
      auth._dev_auth_challenge_present ||
      auth._dev_auth_device_id_present ||
      !auth._dev_auth_first_auth_present ||
      !auth._dev_auth_auth_time_present ||
      !auth._dev_auth_auth_count_present ||
      !auth._dev_auth_signature_present
      ) {
    //TODO: error
    return;
  }

  sha256_Update(&sha256, (uint8_t*) &auth._dev_auth_first_auth._dev_auth_first_auth, 4);
  sha256_Update(&sha256, (uint8_t*) &auth._dev_auth_auth_time._dev_auth_auth_time, 4);
  sha256_Update(&sha256, (uint8_t*) &auth._dev_auth_auth_count._dev_auth_auth_count, 4);
  sha256_Final(&sha256, digest);

  //TODO: fetch the public key!
  if (!ecdsa_verify_digest(&secp256k1, NULL, auth._dev_auth_signature._dev_auth_signature.value, digest)) {
    //TODO: display
  } else {
    //TODO: error
  }
}
