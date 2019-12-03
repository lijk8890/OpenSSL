/**
 * @author lijk@infosec.com.cn
 * @version 0.0.1
 * @date 2018-2-9 17:42:55
**/
#ifndef __SM2_ASN1_H__
#define __SM2_ASN1_H__

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>

// SM2加密
typedef struct SM2Cipher_st {
    ASN1_INTEGER *xCoordinate;                                              // x分量
    ASN1_INTEGER *yCoordinate;                                              // y分量
    ASN1_OCTET_STRING *hash;                                                // 杂凑值
    ASN1_OCTET_STRING *cipherText;                                          // 密文
} SM2Cipher;
DECLARE_ASN1_FUNCTIONS(SM2Cipher);

// SM2签名
typedef struct SM2Signature_st {
    ASN1_INTEGER *r;
    ASN1_INTEGER *s;
} SM2Signature;
DECLARE_ASN1_FUNCTIONS(SM2Signature);

// SM2信封
typedef struct AlgorithmIdentifier_st {
    ASN1_OBJECT *algorithm;
    ASN1_TYPE *parameter;
}AlgorithmIdentifier;
DECLARE_ASN1_FUNCTIONS(AlgorithmIdentifier);

typedef struct SM2EnvelopedKey_st {
    AlgorithmIdentifier *symAlgID;                                          // 对称密码算法标识
    SM2Cipher *symEncryptedKey;                                             // 对称密钥密文
    ASN1_BIT_STRING *sm2PublicKey;                                          // SM2公钥
    ASN1_BIT_STRING *sm2EncryptedPrivateKey;                                // SM2私钥密文
}SM2EnvelopedKey;
DECLARE_ASN1_FUNCTIONS(SM2EnvelopedKey);

// SM2加密ASN.1编码
int sm2cipher_encode(unsigned char *xCoordinate, int xCoordinateLen, unsigned char *yCoordinate, int yCoordinateLen, unsigned char *hash, int hashLen, \
    unsigned char *cipherText, int cipherTextLen, unsigned char *outData, int *outLen);

// SM2加密ASN.1解码
int sm2cipher_decode(unsigned char *inData, int inLen, unsigned char *xCoordinate, int *xCoordinateLen, unsigned char *yCoordinate, int *yCoordinateLen, \
    unsigned char *hash, int *hashLen, unsigned char *cipherText, int *cipherTextLen);

// SM2签名ASN.1编码
int sm2signature_encode(unsigned char *r, int rLen, unsigned char *s, int sLen, unsigned char *outData, int *outLen);

// SM2签名ASN.1解码
int sm2signature_decode(unsigned char *inData, int inLen, unsigned char *r, int *rLen, unsigned char *s, int *sLen);

// SM2信封ASN.1编码
int sm2envelopedkey_encode(unsigned char *symAlgID, int symAlgIDLen, unsigned char *symEncryptedKey, int symEncryptedKeyLen, \
    unsigned char *sm2PublicKey, int sm2PublicKeyLen, unsigned char *sm2EncryptedPrivateKey, int sm2EncryptedPrivateKeyLen, unsigned char *outData, int *outLen);

// SM2信封ASN.1解码
int sm2envelopedkey_decode(unsigned char *inData, int inLen, unsigned char *symAlgID, int *symAlgIDLen, \
    unsigned char *symEncryptedKey, int *symEncryptedKeyLen, unsigned char *sm2PublicKey, int *sm2PublicKeyLen, unsigned char *sm2EncryptedPrivateKey, int *sm2EncryptedPrivateKeyLen);

// OID十六进制转字符串
int oid_hex2txt(unsigned char *hex, int hexLen, char *txt, int txtLen);

// OID字符串转十六进制
int oid_txt2hex(char *txt, int txtLen, unsigned char *hex, int hexLen);

SM2EnvelopedKey* _d2i_SM2EnvelopedKey(unsigned char *in, int inlen);

int _i2d_SM2EnvelopedKey(SM2EnvelopedKey *env, unsigned char **out);

#endif
