#include <errno.h>
#include <string.h>
#include "sm2.h"
#include "sm2_ipp.h"
#include "../o_str.h"
#include "../asn1/asn1.h"
#include "../asn1/asn1t.h"
#include "../objects/objects.h"

ASN1_SEQUENCE(SM2Cipher) = {
    ASN1_SIMPLE(SM2Cipher, xCoordinate, ASN1_INTEGER),
    ASN1_SIMPLE(SM2Cipher, yCoordinate, ASN1_INTEGER),
    ASN1_SIMPLE(SM2Cipher, hash, ASN1_OCTET_STRING),
    ASN1_SIMPLE(SM2Cipher, cipherText, ASN1_OCTET_STRING),
} ASN1_SEQUENCE_END(SM2Cipher);
IMPLEMENT_ASN1_FUNCTIONS(SM2Cipher);

int openssl_sm2_encrypt(int type, const unsigned char *in, int inlen, unsigned char *out, EC_KEY *ec)
{
    const EC_GROUP *group = EC_KEY_get0_group(ec);
    const EC_POINT *pubkey = EC_KEY_get0_public_key(ec);
    SM2Cipher *sm2cipher = SM2Cipher_new();
    BN_CTX *bn_ctx = BN_CTX_new();
    BIGNUM *n = BN_new();
    BIGNUM *h = BN_new();
    BIGNUM *k = BN_new();
    BIGNUM *xCoordinate = BN_new();
    BIGNUM *yCoordinate = BN_new();
    EC_POINT *hPB = EC_POINT_new(group);
    EC_POINT *kPG = EC_POINT_new(group);        // (x1, y1)
    EC_POINT *kPB = EC_POINT_new(group);        // (x2, y2)

    int i = 0;
    int outlen = 0;
    unsigned char x2y2[65] = {0};
    unsigned char hash[SM3_DIGEST_LENGTH] = {0};
    unsigned char *cipherText = (unsigned char*)OPENSSL_malloc(inlen);
    sm3_ctx_t ctx;

    if(sm2cipher == NULL || bn_ctx == NULL || n == NULL || h == NULL || k == NULL \
        || xCoordinate == NULL || yCoordinate == NULL || hPB == NULL || kPG == NULL || kPB == NULL || cipherText == NULL)
    {
        fprintf(stderr, "%s %s:%u - SM2_encrypt failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    BN_CTX_start(bn_ctx);

    if(!EC_GROUP_get_order(group, n, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_GROUP_get_order failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(!EC_GROUP_get_cofactor(group, h, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_GROUP_get_cofactor failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    /* S = [h]PB != O */
    if(!EC_POINT_mul(group, hPB, NULL, pubkey, h, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(EC_POINT_is_at_infinity(group, hPB))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_is_at_infinity failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    do{
        do{
            BN_rand_range(k, n);
        }while(BN_is_zero(k));

        /* C1 = [k]PG = (x1, y1) */
        if(!EC_POINT_mul(group, kPG, k, NULL, NULL, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
        /* [k]PB = (x2, y2) */
        if(!EC_POINT_mul(group, kPB, NULL, pubkey, k, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }

        /* t = KDF(x2 + y2, klen) */
        if(EC_POINT_point2oct(group, kPB, POINT_CONVERSION_UNCOMPRESSED, x2y2, 65, bn_ctx) != 65)
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_point2oct failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
        kdf_sm3(&x2y2[1], 64, cipherText, inlen);

        for(i = 0; i < inlen; i++) {
            if(cipherText[i] != 0)
                break;
        }
    }while(i >= inlen);

    /* C2 = M ^ t */
    for(i = 0; i < inlen; i++) {
        cipherText[i] ^= in[i];
    }

    /* C3 = Hash(x2 + M + y2) */
    sm3_init(&ctx);
    sm3_update(&ctx, &x2y2[1], 32);
    sm3_update(&ctx, in, inlen);
    sm3_update(&ctx, &x2y2[33], 32);
    sm3_final(&ctx, hash);

    /* C1 */
    if(EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field)
    {
        if(!EC_POINT_get_affine_coordinates_GFp(group, kPG, xCoordinate, yCoordinate, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_get_affine_coordinates_GFp failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
    }
    else
    {
        if(!EC_POINT_get_affine_coordinates_GF2m(group, kPG, xCoordinate, yCoordinate, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_get_affine_coordinates_GF2m failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
    }
    BN_to_ASN1_INTEGER(xCoordinate, sm2cipher->xCoordinate);
    BN_to_ASN1_INTEGER(yCoordinate, sm2cipher->yCoordinate);
    /* C3 */
    M_ASN1_OCTET_STRING_set(sm2cipher->hash, hash, SM3_DIGEST_LENGTH);
    /* C2 */
    M_ASN1_OCTET_STRING_set(sm2cipher->cipherText, cipherText, inlen);

    outlen = i2d_SM2Cipher(sm2cipher, &out);
    if(outlen <= 0)
    {
        fprintf(stderr, "%s %s:%u - i2d_SM2Cipher failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    BN_CTX_end(bn_ctx);
    if(cipherText) OPENSSL_free(cipherText);
    if(kPB) EC_POINT_free(kPB);
    if(kPG) EC_POINT_free(kPG);
    if(hPB) EC_POINT_free(hPB);
    if(yCoordinate) BN_free(yCoordinate);
    if(xCoordinate) BN_free(xCoordinate);
    if(k) BN_free(k);
    if(h) BN_free(h);
    if(n) BN_free(n);
    if(bn_ctx) BN_CTX_free(bn_ctx);
    if(sm2cipher) SM2Cipher_free(sm2cipher);
    return outlen;
ErrP:
    BN_CTX_end(bn_ctx);
    if(cipherText) OPENSSL_free(cipherText);
    if(kPB) EC_POINT_free(kPB);
    if(kPG) EC_POINT_free(kPG);
    if(hPB) EC_POINT_free(hPB);
    if(yCoordinate) BN_free(yCoordinate);
    if(xCoordinate) BN_free(xCoordinate);
    if(k) BN_free(k);
    if(h) BN_free(h);
    if(n) BN_free(n);
    if(bn_ctx) BN_CTX_free(bn_ctx);
    if(sm2cipher) SM2Cipher_free(sm2cipher);
    return 0;
}

int openssl_sm2_decrypt(int type, const unsigned char *in, int inlen, unsigned char *out, EC_KEY *ec)
{
    const EC_GROUP *group = EC_KEY_get0_group(ec);
    const BIGNUM *prvkey = EC_KEY_get0_private_key(ec);
    SM2Cipher *sm2cipher = NULL;
    BN_CTX *bn_ctx = BN_CTX_new();
    BIGNUM *n = BN_new();
    BIGNUM *h = BN_new();
    BIGNUM *xCoordinate = NULL;
    BIGNUM *yCoordinate = NULL;
    EC_POINT *hkPG = EC_POINT_new(group);
    EC_POINT *kPG = EC_POINT_new(group);        // (x1, y1)
    EC_POINT *kPB = EC_POINT_new(group);        // (x2, y2)

    int i = 0;
    int outlen = 0;
    unsigned char x2y2[65] = {0};
    unsigned char md[SM3_DIGEST_LENGTH] = {0};
    int hashLen = 0;
    unsigned char *hash = NULL;
    int cipherTextLen = 0;
    unsigned char *cipherText = NULL;
    sm3_ctx_t ctx;

    sm2cipher = d2i_SM2Cipher(NULL, &in, inlen);
    if(sm2cipher == NULL)
    {
        fprintf(stderr, "%s %s:%u - d2i_SM2Cipher failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    /* C1 */
    xCoordinate = ASN1_INTEGER_to_BN(sm2cipher->xCoordinate, NULL);
    yCoordinate = ASN1_INTEGER_to_BN(sm2cipher->yCoordinate, NULL);
    /* C3 */
    hashLen = M_ASN1_STRING_length(sm2cipher->hash);
    hash = M_ASN1_STRING_data(sm2cipher->hash);
    /* C2 */
    cipherTextLen = M_ASN1_STRING_length(sm2cipher->cipherText);
    cipherText = M_ASN1_STRING_data(sm2cipher->cipherText);

    if(sm2cipher == NULL || bn_ctx == NULL || n == NULL || h == NULL || \
        xCoordinate == NULL || yCoordinate == NULL  || hkPG == NULL || kPG == NULL || kPB == NULL)
    {
        fprintf(stderr, "%s %s:%u - SM2_decrypt failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    BN_CTX_start(bn_ctx);

    if(!EC_GROUP_get_order(group, n, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_GROUP_get_order failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(!EC_GROUP_get_cofactor(group, h, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_GROUP_get_cofactor failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    if(EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field)
    {
        if(!EC_POINT_set_affine_coordinates_GFp(group, kPG, xCoordinate, yCoordinate, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_set_affine_coordinates_GFp failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
    }
    else
    {
        if(!EC_POINT_set_affine_coordinates_GF2m(group, kPG, xCoordinate, yCoordinate, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_set_affine_coordinates_GF2m failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
    }

    /* S = [h]C1 != O */
    if(!EC_POINT_mul(group, hkPG, NULL, kPG, h, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(EC_POINT_is_at_infinity(group, hkPG))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_is_at_infinity failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    /* [d]C1 = (x2, y2) */
    if(!EC_POINT_mul(group, kPB, NULL, kPG, prvkey, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(EC_POINT_point2oct(group, kPB, POINT_CONVERSION_UNCOMPRESSED, x2y2, 65, bn_ctx) != 65)
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_point2oct failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    kdf_sm3(&x2y2[1], 64, out, cipherTextLen);

    for(i = 0; i < cipherTextLen; i++)
        out[i] ^= cipherText[i];

    sm3_init(&ctx);
    sm3_update(&ctx, &x2y2[1], 32);
    sm3_update(&ctx, out, cipherTextLen);
    sm3_update(&ctx, &x2y2[33], 32);
    sm3_final(&ctx, md);

    if(hashLen != SM3_DIGEST_LENGTH || OPENSSL_memcmp(md, hash, hashLen) != 0)
    {
        fprintf(stderr, "%s %s:%u - OPENSSL_memcmp digest failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    outlen = cipherTextLen;

    BN_CTX_end(bn_ctx);
    if(kPB) EC_POINT_free(kPB);
    if(kPG) EC_POINT_free(kPG);
    if(hkPG) EC_POINT_free(hkPG);
    if(yCoordinate) BN_free(yCoordinate);
    if(xCoordinate) BN_free(xCoordinate);
    if(h) BN_free(h);
    if(n) BN_free(n);
    if(bn_ctx) BN_CTX_free(bn_ctx);
    if(sm2cipher) SM2Cipher_free(sm2cipher);
    return outlen;
ErrP:
    BN_CTX_end(bn_ctx);
    if(kPB) EC_POINT_free(kPB);
    if(kPG) EC_POINT_free(kPG);
    if(hkPG) EC_POINT_free(hkPG);
    if(yCoordinate) BN_free(yCoordinate);
    if(xCoordinate) BN_free(xCoordinate);
    if(h) BN_free(h);
    if(n) BN_free(n);
    if(bn_ctx) BN_CTX_free(bn_ctx);
    if(sm2cipher) SM2Cipher_free(sm2cipher);
    return 0;
}

int SM2_encrypt(int type, const unsigned char *in, int inlen, unsigned char *out, EC_KEY *ec)
{
#ifdef OPENSSL_WITH_INTEL
    int length = 0;
    unsigned char pubkey[65] = {0};

    length = get_pubkey_from_ec_key(ec, pubkey);
    if(length != 65)
    {
        fprintf(stderr, "%s %s:%u - get_pubkey_from_ec_key failed %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }
    return ipp_sm2_encrypt((unsigned char*)in, inlen, out, pubkey, 65);
#else
    return openssl_sm2_encrypt(type, in, inlen, out, ec);
#endif
}

int SM2_decrypt(int type, const unsigned char *in, int inlen, unsigned char *out, EC_KEY *ec)
{
#ifdef OPENSSL_WITH_INTEL
    int length = 0;
    unsigned char prvkey[32] = {0};

    length = get_prvkey_from_ec_key(ec, prvkey);
    if(length <= 0)
    {
        fprintf(stderr, "%s %s:%u - get_prvkey_from_ec_key failed %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }
    return ipp_sm2_decrypt((unsigned char*)in, inlen, out, prvkey, length);
#else
    return openssl_sm2_decrypt(type, in, inlen, out, ec);
#endif
}
