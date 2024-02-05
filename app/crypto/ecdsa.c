#include <string.h>

#include "ecdsa.h"
#include "hal.h"
#include "memzero.h"

static int ec_uncompress_point(const uint8_t* point, uint8_t odd, uint8_t out[64]) {
  return 1;
}

static const uint8_t* ec_uncompress_key(const uint8_t* pub_key, uint8_t out[64]) {
  if (pub_key[0] == 0x04) {
    return &pub_key[1];
  } else {
    if (ec_uncompress_point(&pub_key[1], (pub_key[0] & 0x1), out) != 0) {
      return NULL;
    }

    return out;
  }
}

int ecdsa_sign(const ecdsa_curve *curve, const uint8_t *priv_key, const uint8_t *digest, uint8_t *sig) {
  uint8_t k[ECDSA_256_ELEMENT_SIZE];
  hal_rng_next(k, ECDSA_256_ELEMENT_SIZE);
  return hal_ecdsa_sign(curve, priv_key, digest, k, sig) == HAL_SUCCESS ? 0 : 1;
}

int ecdsa_verify(const ecdsa_curve *curve, const uint8_t *pub_key, const uint8_t *sig, const uint8_t *digest) {
  uint8_t buf[ECDSA_256_ELEMENT_SIZE * 2];
  pub_key = ec_uncompress_key(pub_key, buf);

  if (pub_key == NULL) {
    return 1;
  }

  return hal_ecdsa_verify(curve, pub_key, sig, digest) == HAL_SUCCESS ? 0 : 1;
}

int ecdsa_get_public_key65(const ecdsa_curve *curve, const uint8_t *priv_key, uint8_t *pub_key) {
  if (hal_ec_point_multiply(curve, priv_key, curve->G, &pub_key[1]) != HAL_SUCCESS) {
    return 1;
  }

  pub_key[0] = 0x04;
  return 0;
}

int ecdsa_recover_pub_from_sig(const ecdsa_curve *curve, uint8_t *pub_key, const uint8_t *sig, const uint8_t *digest, int recid) {
  return 1;
}

int ecdh_multiply(const ecdsa_curve *curve, const uint8_t *priv_key, const uint8_t *pub_key, uint8_t *session_key) {
  uint8_t buf[ECDSA_256_ELEMENT_SIZE * 2];
  pub_key = ec_uncompress_key(pub_key, buf);

  if (pub_key == NULL) {
    return 1;
  }

  if (hal_ec_point_multiply(curve, priv_key, &pub_key[1], &session_key[1]) != HAL_SUCCESS) {
    return 1;
  }

  session_key[0] = 0x04;
  return 0;
}

int ecdsa_sig_to_der(const uint8_t *sig, uint8_t *der) {
  int i = 0;
  uint8_t *p = der, *len = NULL, *len1 = NULL, *len2 = NULL;
  *p = 0x30;
  p++;  // sequence
  *p = 0x00;
  len = p;
  p++;  // len(sequence)

  *p = 0x02;
  p++;  // integer
  *p = 0x00;
  len1 = p;
  p++;  // len(integer)

  // process R
  i = 0;
  while (i < 31 && sig[i] == 0) {
    i++;
  }                      // skip leading zeroes
  if (sig[i] >= 0x80) {  // put zero in output if MSB set
    *p = 0x00;
    p++;
    *len1 = *len1 + 1;
  }
  while (i < 32) {  // copy bytes to output
    *p = sig[i];
    p++;
    *len1 = *len1 + 1;
    i++;
  }

  *p = 0x02;
  p++;  // integer
  *p = 0x00;
  len2 = p;
  p++;  // len(integer)

  // process S
  i = 32;
  while (i < 63 && sig[i] == 0) {
    i++;
  }                      // skip leading zeroes
  if (sig[i] >= 0x80) {  // put zero in output if MSB set
    *p = 0x00;
    p++;
    *len2 = *len2 + 1;
  }
  while (i < 64) {  // copy bytes to output
    *p = sig[i];
    p++;
    *len2 = *len2 + 1;
    i++;
  }

  *len = *len1 + *len2 + 4;
  return *len + 2;
}

// Parse a DER-encoded signature. We don't check whether the encoded integers
// satisfy DER requirements regarding leading zeros.
int ecdsa_sig_from_der(const uint8_t *der, size_t der_len, uint8_t sig[64]) {
  memzero(sig, 64);

  // Check sequence header.
  if (der_len < 2 || der[1] > 70 || der[0] != 0x30 || der[1] > der_len) {
    return 1;
  }

  der_len = der[1] + 2;

  // Read two DER-encoded integers.
  size_t pos = 2;
  for (int i = 0; i < 2; ++i) {
    // Check integer header.
    if (der_len < pos + 2 || der[pos] != 0x02) {
      return 1;
    }

    // Locate the integer.
    size_t int_len = der[pos + 1];
    pos += 2;
    if (pos + int_len > der_len) {
      return 1;
    }

    // Skip a possible leading zero.
    if (int_len != 0 && der[pos] == 0) {
      int_len--;
      pos++;
    }

    // Copy the integer to the output, making sure it fits.
    if (int_len > 32) {
      return 1;
    }
    memcpy(sig + 32 * (i + 1) - int_len, der + pos, int_len);

    // Move on to the next one.
    pos += int_len;
  }

  // Check that there are no trailing elements in the sequence.
  if (pos != der_len) {
    return 1;
  }

  return 0;
}
