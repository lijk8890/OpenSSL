#include "sm2.h"
#include "../asn1/asn1.h"
#include "../asn1/asn1t.h"

typedef struct SM2Cipher_st {
    ASN1_INTEGER *xCoordinate;          // x分量
    ASN1_INTEGER *yCoordinate;          // y分量
    ASN1_OCTET_STRING *hash;            // 杂凑值
    ASN1_OCTET_STRING *cipherText;      // 密文
} SM2Cipher;

ASN1_SEQUENCE(SM2Cipher) = {
    ASN1_SIMPLE(SM2Cipher, xCoordinate, ASN1_INTEGER),
    ASN1_SIMPLE(SM2Cipher, yCoordinate, ASN1_INTEGER),
    ASN1_SIMPLE(SM2Cipher, hash, ASN1_OCTET_STRING),
    ASN1_SIMPLE(SM2Cipher, cipherText, ASN1_OCTET_STRING),
} ASN1_SEQUENCE_END(SM2Cipher);
DECLARE_ASN1_FUNCTIONS(SM2Cipher);
IMPLEMENT_ASN1_FUNCTIONS(SM2Cipher);

int SM2_encrypt(int type, const unsigned char *in, int inlen, unsigned char *out, int *outlen, EC_KEY *ec)
{
    return 0;
}

int SM2_decrypt(int type, const unsigned char *in, int inlen, unsigned char *out, int *outlen, EC_KEY *ec)
{
    return 0;
}
