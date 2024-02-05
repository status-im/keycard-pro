#ifndef __ECDSA_H__
#define __ECDSA_H__

#include <stdint.h>
#include <stddef.h>

#define ECDSA_256_ELEMENT_SIZE 32

#ifndef SOFT_ECDSA
typedef struct {
  uint8_t prime[ECDSA_256_ELEMENT_SIZE];
  uint8_t G[ECDSA_256_ELEMENT_SIZE * 2];
  uint8_t order[ECDSA_256_ELEMENT_SIZE];
  uint8_t a[ECDSA_256_ELEMENT_SIZE];
  uint8_t a_sign;
  uint8_t b[ECDSA_256_ELEMENT_SIZE];
} ecdsa_curve;
#endif

int ecdsa_sign(const ecdsa_curve *curve, const uint8_t *priv_key, const uint8_t *digest, uint8_t *sig);
int ecdsa_verify(const ecdsa_curve *curve, const uint8_t *pub_key, const uint8_t *sig, const uint8_t *digest);
int ecdsa_get_public_key65(const ecdsa_curve *curve, const uint8_t *priv_key, uint8_t *pub_key);
int ecdsa_recover_pub_from_sig(const ecdsa_curve *curve, uint8_t *pub_key, const uint8_t *sig, const uint8_t *digest, int recid);
int ecdsa_sig_to_der(const uint8_t *sig, uint8_t *der);
int ecdsa_sig_from_der(const uint8_t *der, size_t der_len, uint8_t sig[64]);

int ecdh_multiply(const ecdsa_curve *curve, const uint8_t *priv_key, const uint8_t *pub_key, uint8_t *session_key);
#endif
