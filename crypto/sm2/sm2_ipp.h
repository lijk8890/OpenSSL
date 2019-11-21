#ifdef OPENSSL_WITH_INTEL
#ifndef __IPP_SM2_H__
#define __IPP_SM2_H__

#include "ipp.h"
#include "ippcp.h"

#ifdef __cplusplus
extern "C"
{
#endif

int ipp_sm2_keypair(unsigned char prvkey[32], unsigned char pubkey[65]);

int ipp_sm2_sign(unsigned char *dgst, int dgstlen, unsigned char *sig, int *siglen, unsigned char *prvkey, int keylen);

int ipp_sm2_verify(unsigned char *dgst, int dgstlen, unsigned char *sig, int siglen, unsigned char *pubkey, int keylen);

int ipp_sm2_encrypt(unsigned char *in, int inlen, unsigned char *out, unsigned char *pubkey, int keylen);

int ipp_sm2_decrypt(unsigned char *in, int inlen, unsigned char *out, unsigned char *prvkey, int keylen);

int ipp_sm2_compute_key(                                                \
    const char *self_id, int self_id_len,                               \
    unsigned char *self_tmp_prvkey, int self_tmp_prvkey_len,            \
    unsigned char *self_tmp_pubkey, int self_tmp_pubkey_len,            \
    unsigned char *self_enc_prvkey, int self_enc_prvkey_len,            \
    unsigned char *self_enc_pubkey, int self_enc_pubkey_len,            \
    const char *peer_id, int peer_id_len,                               \
    unsigned char *peer_tmp_pubkey, int peer_tmp_pubkey_len,            \
    unsigned char *peer_enc_pubkey, int peer_enc_pubkey_len,            \
    unsigned char *session_key, int session_key_len,                    \
    int is_server                                                       \
    );

int ipp_kdf_sm3(unsigned char *in, int inlen, unsigned char *out, int outlen);

int ipp_get_z(const char *id, int id_len, unsigned char *pubkey, int pubkey_len, unsigned char *md);

// xbar = 2^w + (x and (2^w - 1)) = 2^w + (x mod 2^w)
int ipp_get_x(IppsBigNumState *x, IppsBigNumState *xbar);

void ipp_print_bn(IppsBigNumState *bn);

void ipp_print_point(IppsECCPPointState *point, IppsECCPState *eccp);

#ifdef __cplusplus
}
#endif

#endif
#endif
