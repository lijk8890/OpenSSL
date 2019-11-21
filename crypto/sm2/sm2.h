#ifndef __SM2_H__
#define __SM2_H__

#include <openssl/ec.h>
#include <openssl/sm3.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct SM2Cipher_st {
    ASN1_INTEGER *xCoordinate;          // x分量
    ASN1_INTEGER *yCoordinate;          // y分量
    ASN1_OCTET_STRING *hash;            // 杂凑值
    ASN1_OCTET_STRING *cipherText;      // 密文
} SM2Cipher;
DECLARE_ASN1_FUNCTIONS(SM2Cipher);

int openssl_sm2_sign(int type, const unsigned char *dgst, int dgstlen, unsigned char *sig, int *siglen, EC_KEY *ec);

int openssl_sm2_verify(int type, const unsigned char *dgst, int dgstlen, const unsigned char *sig, int siglen, EC_KEY *ec);

int openssl_sm2_encrypt(int type, const unsigned char *in, int inlen, unsigned char *out, EC_KEY *ec);

int openssl_sm2_decrypt(int type, const unsigned char *in, int inlen, unsigned char *out, EC_KEY *ec);

int openssl_sm2_compute_key(                                            \
    const EC_GROUP *group, const char *self_id, int self_id_len,        \
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

int SM2_sign(int type, const unsigned char *dgst, int dgstlen, unsigned char *sig, int *siglen, EC_KEY *ec);

int SM2_verify(int type, const unsigned char *dgst, int dgstlen, const unsigned char *sig, int siglen, EC_KEY *ec);

int SM2_encrypt(int type, const unsigned char *in, int inlen, unsigned char *out, EC_KEY *ec);

int SM2_decrypt(int type, const unsigned char *in, int inlen, unsigned char *out, EC_KEY *ec);

int SM2_compute_key(SSL *s, const EC_KEY *ecdh, const EC_POINT *point, unsigned char *out);

int kdf_sm3(unsigned char *in, int inlen, unsigned char *out, int outlen);

int get_z(const char *id, int id_len, unsigned char *pubkey, int pubkey_len, unsigned char md[SM3_DIGEST_LENGTH]);

void print_bn(BIGNUM *bn);

void print_point(const EC_GROUP *group, EC_POINT *point);

// unsigned char prvkey[32];
int get_prvkey_from_ec_key(EC_KEY *ec_key, unsigned char out[32]);

// unsigned char pubkey[64+1];
int get_pubkey_from_ec_key(EC_KEY *ec_key, unsigned char out[65]);

#ifdef __cplusplus
}
#endif

#endif
