#ifndef __SM2_H__
#define __SM2_H__

#include <openssl/ec.h>

int SM2_sign(int type, const unsigned char *dgst, int dgstlen, unsigned char *sig, int *siglen, EC_KEY *ec);

int SM2_verify(int type, const unsigned char *dgst, int dgstlen, const unsigned char *sig, int siglen, EC_KEY *ec);

int SM2_encrypt(const EVP_MD *kdf_md, const EVP_MD *mac_md, point_conversion_form_t point_form, const unsigned char *in, int inlen, unsigned char *out, int *outlen, EC_KEY *ec);

int SM2_decrypt(const EVP_MD *kdf_md, const EVP_MD *mac_md, point_conversion_form_t point_form, const unsigned char *in, int inlen, unsigned char *out, int *outlen, EC_KEY *ec);

int SM2_compute_key(unsigned char *out, int outlen, const EC_POINT *point, EC_KEY *ec);

#endif
