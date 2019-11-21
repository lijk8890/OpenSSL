#ifdef OPENSSL_WITH_INTEL
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "../ecdsa/ecdsa.h"
#include "../asn1/asn1.h"
#include "../asn1/asn1t.h"
#include "../sm3/sm3_ipp.h"
#include "sm2.h"
#include "sm2_ipp.h"

#ifdef _WIN32
#pragma comment(lib, "ippcpmt.lib")
#pragma comment(lib, "ippcoremt.lib")
#endif

#define SM2_INTS 8
#define SM2_BITS 256

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_be16(v) ((v << 8) | (v >> 8))
#define cpu_to_be32(v) ((cpu_to_be16(v) << 16) | cpu_to_be16(v >> 16))
#else
#define cpu_to_be16(v) (v)
#define cpu_to_be32(v) (v)
#endif

// a
static Ipp8u sm2a[] = {
    0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC
};

// b
static Ipp8u sm2b[] = {
    0x28, 0xE9, 0xFA, 0x9E, 0x9D, 0x9F, 0x5E, 0x34, 0x4D, 0x5A, 0x9E, 0x4B, 0xCF, 0x65, 0x09, 0xA7,
    0xF3, 0x97, 0x89, 0xF5, 0x15, 0xAB, 0x8F, 0x92, 0xDD, 0xBC, 0xBD, 0x41, 0x4D, 0x94, 0x0E, 0x93
};

// Gx
static Ipp8u sm2Gx[] = {
    0x32, 0xC4, 0xAE, 0x2C, 0x1F, 0x19, 0x81, 0x19, 0x5F, 0x99, 0x04, 0x46, 0x6A, 0x39, 0xC9, 0x94,
    0x8F, 0xE3, 0x0B, 0xBF, 0xF2, 0x66, 0x0B, 0xE1, 0x71, 0x5A, 0x45, 0x89, 0x33, 0x4C, 0x74, 0xC7
};

// Gy
static Ipp8u sm2Gy[] = {
    0xBC, 0x37, 0x36, 0xA2, 0xF4, 0xF6, 0x77, 0x9C, 0x59, 0xBD, 0xCE, 0xE3, 0x6B, 0x69, 0x21, 0x53,
    0xD0, 0xA9, 0x87, 0x7C, 0xC6, 0x2A, 0x47, 0x40, 0x02, 0xDF, 0x32, 0xE5, 0x21, 0x39, 0xF0, 0xA0
};

// GX
static Ipp32u sm2GX[] = {
    0x334C74C7, 0x715A4589, 0xF2660BE1, 0x8FE30BBF,
    0x6A39C994, 0x5F990446, 0x1F198119, 0x32C4AE2C
};

// GY
static Ipp32u sm2GY[] = {
    0x2139F0A0, 0x02DF32E5, 0xC62A4740, 0xD0A9877C,
    0x6B692153, 0x59BDCEE3, 0xF4F6779C, 0xBC3736A2
};

// order
static Ipp32u sm2n[] = {
    0x39D54123, 0x53BBF409, 0x21C6052B, 0x7203DF6B,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE
};

// cofactor
static Ipp32u sm2h[] = {
    0x00000001
};

// power 2^w = 2^127
static Ipp32u sm2pow[] = {
    0x00000000, 0x00000000, 0x00000000, 0x80000000
};

static IppsBigNumState* newSM2_BN(int len, Ipp32u *data)
{
    int size = 0;
    IppsBigNumState *ctx = NULL;
    ippsBigNumGetSize(len, &size);
    ctx = (IppsBigNumState*)malloc(size*sizeof(Ipp8u));
    ippsBigNumInit(len, ctx);
    if(data)
        ippsSet_BN(IppsBigNumPOS, len, data, ctx);
    return ctx;
}

static IppsECCPPointState* newSM2_Point(int bit)
{
    int size = 0;
    IppsECCPPointState *ctx = NULL;
    ippsECCPPointGetSize(bit, &size);
    ctx = (IppsECCPPointState*)malloc(size*sizeof(Ipp8u));
    ippsECCPPointInit(bit, ctx);
    return ctx;
}

static IppsECCPState* newSM2_ECCP(int bit)
{
    int size = 0;
    IppsECCPState *ctx = NULL;
    ippsECCPGetSize(bit, &size);
    ctx = (IppsECCPState*)malloc(size*sizeof(Ipp8u));
    ippsECCPInit(bit, ctx);
    ippsECCPSetStd(IppECCPStdSM2, ctx);
    return ctx;
}

static Ipp32u* randSM2_Ipp32u(int len, Ipp32u *data)
{
#ifdef __linux__
    FILE *fp = fopen("/dev/urandom", "rb");
    while(len-- > 0)
    {
        int rv = fread(&data[len], sizeof(Ipp32u), 1, fp);
        if(rv <= 0)
        {
            fprintf(stderr, "%s:%d - %d:%s\n", __FILE__, __LINE__, errno, strerror(errno));
            break;
        }
    }
    if(fp) fclose(fp);
    return data;
#else
    int i = 0;
    srand((unsigned int)time(NULL));
    for(i = 0; i < len; i++)
        data[i] = (rand() << 16) + rand();
    return data;
#endif
}

// Pseudorandom Number Generation
static IppsPRNGState* newSM2_PRNG(int len)
{
    int size = 0;
    IppsPRNGState *ctx = NULL;
    Ipp32u *seed = NULL;
    IppsBigNumState *bn = NULL;
    ippsPRNGGetSize(&size);
    ctx = (IppsPRNGState*)malloc(size*sizeof(Ipp8u));
    ippsPRNGInit(len, ctx);
    seed = (Ipp32u*)malloc(len*sizeof(Ipp32u));
    bn = newSM2_BN(len, randSM2_Ipp32u(len, seed));
    ippsPRNGSetSeed(bn, ctx);
    if(bn) free(bn);
    if(seed) free(seed);
    return ctx;
}

static IppsMontState* newSM2_Mont(int len, Ipp32u *order)
{
    int size = 0;
    IppsMontState *ctx = NULL;
    ippsMontGetSize(IppsBinaryMethod, len, &size);
    ctx = (IppsMontState*)malloc(size*sizeof(Ipp8u));
    ippsMontInit(IppsBinaryMethod, len, ctx);
    ippsMontSet(order, len, ctx);
    return ctx;
}

int ipp_kdf_sm3(unsigned char *in, int inlen, unsigned char *out, int outlen)
{
    unsigned int counter = 1;
    unsigned int counter_be = 0;
    unsigned char md[SM3_DIGEST_LENGTH];
    IppsSM3State *sm3Ctx = ipp_sm3_new();

    while(outlen > 0)
    {
        counter_be = cpu_to_be32(counter);
        counter++;

        ipp_sm3_init(sm3Ctx);
        ipp_sm3_update(sm3Ctx, in, inlen);
        ipp_sm3_update(sm3Ctx, (unsigned char*)&counter_be, 4);
        ipp_sm3_final(sm3Ctx, md);

        memcpy(out, md, outlen < SM3_DIGEST_LENGTH ? outlen : SM3_DIGEST_LENGTH);
        outlen -= SM3_DIGEST_LENGTH;
        out += SM3_DIGEST_LENGTH;
    }

    if(sm3Ctx) ipp_sm3_delete(sm3Ctx);
    return 1;
}

int ipp_get_z(const char *id, int id_len, unsigned char *pubkey, int pubkey_len, unsigned char *md)
{
    int entl = 0;
    unsigned char b[2] = {0};
    IppsSM3State *sm3Ctx = ipp_sm3_new();

    entl = id_len * 8;
    b[0] = (entl >> 8) & 0xFF;
    b[1] = (entl >> 0) & 0xFF;

    ipp_sm3_init(sm3Ctx);
    ipp_sm3_update(sm3Ctx, b, 2);
    ipp_sm3_update(sm3Ctx, (unsigned char*)id, id_len);
    ipp_sm3_update(sm3Ctx, sm2a, 32);
    ipp_sm3_update(sm3Ctx, sm2b, 32);
    ipp_sm3_update(sm3Ctx, sm2Gx, 32);
    ipp_sm3_update(sm3Ctx, sm2Gy, 32);
    if(pubkey_len == 65 && pubkey[0] == 0x04)
        ipp_sm3_update(sm3Ctx, &pubkey[1], 64);
    else
        ipp_sm3_update(sm3Ctx, &pubkey[0], 64);
    ipp_sm3_final(sm3Ctx, md);

    if(sm3Ctx) ipp_sm3_delete(sm3Ctx);
    return 1;
}

// xbar = 2^w + (x and (2^w - 1)) = 2^w + (x mod 2^w)
int ipp_get_x(IppsBigNumState *x, IppsBigNumState *xbar)
{
    IppsBigNumState *power = newSM2_BN(SM2_INTS/2, sm2pow);

    ippsMod_BN(x, power, xbar);
    ippsAdd_BN(power, xbar, xbar);

    if(power) free(power);
    return 1;
}

void ipp_print_bn(IppsBigNumState *bn)
{
    int i = 0;
    Ipp8u b[32] = {0};
    ippsGetOctString_BN(&b[0], 32, bn);
    for(i = 0; i < 32; i++)
        fprintf(stdout, "0x%02hhx%c", b[i], (i + 1) % 16 == 0 ? '\n' : ' ');
    fprintf(stdout, "\n");
}

void ipp_print_point(IppsECCPPointState *point, IppsECCPState *eccp)
{
    IppsBigNumState *x = newSM2_BN(SM2_INTS, NULL);
    IppsBigNumState *y = newSM2_BN(SM2_INTS, NULL);
    ippsECCPGetPoint(x, y, point, eccp);
    ipp_print_bn(x);
    ipp_print_bn(y);
    if(x) free(x);
    if(y) free(y);
}

int ipp_sm2_keypair(unsigned char prvkey[32], unsigned char pubkey[65])
{
    IppStatus status;
    IppsECCPState *ECCP = newSM2_ECCP(SM2_BITS);
    IppsPRNGState *PRNG = newSM2_PRNG(SM2_INTS);
    IppsBigNumState *x = newSM2_BN(SM2_INTS, NULL);
    IppsBigNumState *y = newSM2_BN(SM2_INTS, NULL);
    IppsBigNumState *k = newSM2_BN(SM2_INTS, NULL);
    IppsECCPPointState *point = newSM2_Point(SM2_BITS);

    status = ippsECCPGenKeyPair(k, point, ECCP, ippsPRNGen, PRNG);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }
    ippsECCPGetPoint(x, y, point, ECCP);

    pubkey[0] = 0x04;
    ippsGetOctString_BN(&pubkey[1], 32, x);
    ippsGetOctString_BN(&pubkey[33], 32, y);

    status = ippsGetOctString_BN(&prvkey[0], 32, k);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    if(x) free(x);
    if(y) free(y);
    if(k) free(k);
    if(point) free(point);
    if(PRNG) free(PRNG);
    if(ECCP) free(ECCP);
    return 1;
ErrP:
    if(x) free(x);
    if(y) free(y);
    if(k) free(k);
    if(point) free(point);
    if(PRNG) free(PRNG);
    if(ECCP) free(ECCP);
    return 0;
}

int ipp_sm2_sign(unsigned char *dgst, int dgstlen, unsigned char *sig, int *siglen, unsigned char *prvkey, int keylen)
{
    IppStatus status = ippStsNoErr;
    IppsBigNumState *digest = newSM2_BN(SM2_INTS, NULL);        // 消息摘要
    IppsBigNumState *prvKey = newSM2_BN(SM2_INTS, NULL);        // 证书私钥
    IppsBigNumState *sigr = newSM2_BN(SM2_INTS, NULL);          // 签名R值
    IppsBigNumState *sigs = newSM2_BN(SM2_INTS, NULL);          // 签名S值
    IppsBigNumState *k = newSM2_BN(SM2_INTS, NULL);             // 伪随机数
    IppsPRNGState *prng = newSM2_PRNG(SM2_INTS);                // 伪随机数生成器
    IppsECCPState *eccp = newSM2_ECCP(SM2_BITS);                // SM2椭圆曲线
    ECDSA_SIG *sm2sign = ECDSA_SIG_new();

    Ipp8u r[32] = {0};
    Ipp8u s[32] = {0};

    ippsPRNGen_BN(k, SM2_BITS, prng);
    ippsSetOctString_BN(dgst, dgstlen, digest);

    ippsSetOctString_BN(prvkey, keylen, prvKey);
    ippsECCPSetKeyPair(prvKey, NULL, ippTrue, eccp);

    status = ippsECCPSignSM2(digest, prvKey, k, sigr, sigs, eccp);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    ippsGetOctString_BN(r, 32, sigr);
    ippsGetOctString_BN(s, 32, sigs);
    BN_bin2bn(r, 32, sm2sign->r);
    BN_bin2bn(s, 32, sm2sign->s);
    *siglen = i2d_ECDSA_SIG(sm2sign, &sig);

    if(sm2sign) ECDSA_SIG_free(sm2sign);
    if(digest) free(digest);
    if(prvKey) free(prvKey);
    if(sigr) free(sigr);
    if(sigs) free(sigs);
    if(k) free(k);
    if(prng) free(prng);
    if(eccp) free(eccp);
    return 1;
ErrP:
    if(sm2sign) ECDSA_SIG_free(sm2sign);
    if(digest) free(digest);
    if(prvKey) free(prvKey);
    if(sigr) free(sigr);
    if(sigs) free(sigs);
    if(k) free(k);
    if(prng) free(prng);
    if(eccp) free(eccp);
    return 0;
}

int ipp_sm2_verify(unsigned char *dgst, int dgstlen, unsigned char *sig, int siglen, unsigned char *pubkey, int keylen)
{
    IppStatus status = ippStsNoErr;
    IppECResult result = ippECValid;
    IppsBigNumState *digest = newSM2_BN(SM2_INTS, NULL);        // 消息摘要
    IppsECCPPointState *pubKey = newSM2_Point(SM2_BITS);        // 证书公钥
    IppsBigNumState *pubx = newSM2_BN(SM2_INTS, NULL);          // 公钥X坐标
    IppsBigNumState *puby = newSM2_BN(SM2_INTS, NULL);          // 公钥Y坐标
    IppsBigNumState *sigr = newSM2_BN(SM2_INTS, NULL);          // 签名R值
    IppsBigNumState *sigs = newSM2_BN(SM2_INTS, NULL);          // 签名S值
    IppsECCPState *eccp = newSM2_ECCP(SM2_BITS);                // SM2椭圆曲线
    ECDSA_SIG *sm2sign = NULL;

    Ipp8u rlen = 0;
    Ipp8u slen = 0;
    Ipp8u r[32] = {0};
    Ipp8u s[32] = {0};

    sm2sign = d2i_ECDSA_SIG(NULL, (const unsigned char**)&sig, siglen);
    if(sm2sign == NULL)
    {
        fprintf(stderr, "%s %s:%u - d2i_ECDSA_SIG failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    rlen = BN_bn2bin(sm2sign->r, r);
    slen = BN_bn2bin(sm2sign->s, s);
    ippsSetOctString_BN(r, rlen, sigr);
    ippsSetOctString_BN(s, slen, sigs);

    ippsSetOctString_BN(dgst, dgstlen, digest);

    if(keylen == 65 && pubkey[0] == 0x04)
    {
        ippsSetOctString_BN(&pubkey[1], 32, pubx);
        ippsSetOctString_BN(&pubkey[33], 32, puby);
    }
    else
    {
        ippsSetOctString_BN(&pubkey[0], 32, pubx);
        ippsSetOctString_BN(&pubkey[32], 32, puby);
    }
    ippsECCPSetPoint(pubx, puby, pubKey, eccp);
    ippsECCPSetKeyPair(NULL, pubKey, ippTrue, eccp);

    status = ippsECCPVerifySM2(digest, pubKey, sigr, sigs, &result, eccp);
    if(status != ippStsNoErr || result != ippECValid)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippsECCGetResultString(result));
        goto ErrP;
    }

    if(sm2sign) ECDSA_SIG_free(sm2sign);
    if(digest) free(digest);
    if(pubKey) free(pubKey);
    if(pubx) free(pubx);
    if(puby) free(puby);
    if(sigr) free(sigr);
    if(sigs) free(sigs);
    if(eccp) free(eccp);
    return 1;
ErrP:
    if(sm2sign) ECDSA_SIG_free(sm2sign);
    if(digest) free(digest);
    if(pubKey) free(pubKey);
    if(pubx) free(pubx);
    if(puby) free(puby);
    if(sigr) free(sigr);
    if(sigs) free(sigs);
    if(eccp) free(eccp);
    return 0;
}

int ipp_sm2_encrypt(unsigned char *in, int inlen, unsigned char *out, unsigned char *pubkey, int keylen)
{
    IppsECCPState *eccp = newSM2_ECCP(SM2_BITS);                // SM2椭圆曲线
    IppsPRNGState *prng = newSM2_PRNG(SM2_INTS);                // 伪随机数生成器
    IppsBigNumState *k = newSM2_BN(SM2_INTS, NULL);
    IppsBigNumState *x = newSM2_BN(SM2_INTS, sm2GX);
    IppsBigNumState *y = newSM2_BN(SM2_INTS, sm2GY);
    IppsECCPPointState *p = newSM2_Point(SM2_BITS);
    IppsECCPPointState *pk = newSM2_Point(SM2_BITS);
    BIGNUM *xCoordinate = BN_new();
    BIGNUM *yCoordinate = BN_new();
    SM2Cipher *sm2cipher = SM2Cipher_new();

    int i = 0;
    int outlen = 0;
    Ipp8u md[32] = {0};
    Ipp8u xy[64] = {0};
    Ipp8u *cipherText = NULL;
    IppsSM3State *sm3Ctx = NULL;

    ippsPRNGen_BN(k, SM2_BITS, prng);

    ippsECCPSetPoint(x, y, p, eccp);
    ippsECCPMulPointScalar(p, k, pk, eccp);         // pkG = k * pG
    ippsECCPGetPoint(x, y, pk, eccp);

    ippsGetOctString_BN(&xy[0], 32, x);
    ippsGetOctString_BN(&xy[32], 32, y);

    // C1
    BN_bin2bn(&xy[0], 32, xCoordinate);
    BN_bin2bn(&xy[32], 32, yCoordinate);

    BN_to_ASN1_INTEGER(xCoordinate, sm2cipher->xCoordinate);
    BN_to_ASN1_INTEGER(yCoordinate, sm2cipher->yCoordinate);

    if(pubkey[0] == 0x04 && keylen == 65)
    {
        ippsSetOctString_BN(&pubkey[1], 32, x);
        ippsSetOctString_BN(&pubkey[33], 32, y);
    }
    else
    {
        ippsSetOctString_BN(&pubkey[0], 32, x);
        ippsSetOctString_BN(&pubkey[32], 32, y); 
    }

    ippsECCPSetPoint(x, y, p, eccp);                // 证书公钥
    ippsECCPMulPointScalar(p, k, pk, eccp);         // pkB = k * pB
    ippsECCPGetPoint(x, y, pk, eccp);

    // pkB = k * pB = k * [ d * pG ] = d * [ k * pG ] = d * pkG
    ippsGetOctString_BN(&xy[0], 32, x);
    ippsGetOctString_BN(&xy[32], 32, y);

    sm3Ctx = ipp_sm3_new();
    ipp_sm3_init(sm3Ctx);
    ipp_sm3_update(sm3Ctx, &xy[0], 32);
    ipp_sm3_update(sm3Ctx, in, inlen);
    ipp_sm3_update(sm3Ctx, &xy[32], 32);
    ipp_sm3_final(sm3Ctx, md);
    if(sm3Ctx) ipp_sm3_delete(sm3Ctx);

    // C3
    M_ASN1_OCTET_STRING_set(sm2cipher->hash, md, 32);

    cipherText = (Ipp8u*)malloc(inlen);
    ipp_kdf_sm3(xy, 64, cipherText, inlen);

    for(i = 0; i < inlen; i++)
        cipherText[i] ^= in[i];                     // 密文数据

    // C2
    M_ASN1_OCTET_STRING_set(sm2cipher->cipherText, cipherText, inlen);

    outlen = i2d_SM2Cipher(sm2cipher, &out);
    if(outlen <= 0)
    {
        fprintf(stderr, "%s %s:%u - i2d_SM2Cipher failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    if(cipherText) free(cipherText);
    if(xCoordinate) BN_free(xCoordinate);
    if(yCoordinate) BN_free(yCoordinate);
    if(sm2cipher) SM2Cipher_free(sm2cipher);
    if(x) free(x);
    if(y) free(y);
    if(k) free(k);
    if(p) free(p);
    if(pk) free(pk);
    if(prng) free(prng);
    if(eccp) free(eccp);
    return outlen;
ErrP:
    if(cipherText) free(cipherText);
    if(xCoordinate) BN_free(xCoordinate);
    if(yCoordinate) BN_free(yCoordinate);
    if(sm2cipher) SM2Cipher_free(sm2cipher);
    if(x) free(x);
    if(y) free(y);
    if(k) free(k);
    if(p) free(p);
    if(pk) free(pk);
    if(prng) free(prng);
    if(eccp) free(eccp);
    return 0;
}

int ipp_sm2_decrypt(unsigned char *in, int inlen, unsigned char *out, unsigned char *prvkey, int keylen)
{
    IppsBigNumState *x = newSM2_BN(SM2_INTS, NULL);
    IppsBigNumState *y = newSM2_BN(SM2_INTS, NULL);
    IppsBigNumState *d = newSM2_BN(SM2_INTS, NULL);
    IppsECCPPointState *p = newSM2_Point(SM2_BITS);
    IppsECCPPointState *pk = newSM2_Point(SM2_BITS);
    IppsECCPState *eccp = newSM2_ECCP(SM2_BITS);    // SM2椭圆曲线
    BIGNUM *xCoordinate = NULL;
    BIGNUM *yCoordinate = NULL;
    SM2Cipher *sm2cipher = NULL;

    int i = 0;
    int xlen = 0;
    int ylen = 0;
    Ipp8u md[32] = {0};
    Ipp8u xy[64] = {0};
    int hashLen = 0;
    Ipp8u *hash = NULL;
    int cipherTextLen = 0;
    Ipp8u *cipherText = NULL;
    Ipp8u *plainText = NULL;
    IppsSM3State *sm3Ctx = NULL;

    sm2cipher = d2i_SM2Cipher(NULL, (const unsigned char**)&in, inlen);
    if(sm2cipher == NULL)
    {
        fprintf(stderr, "%s %s:%u - d2i_SM2Cipher failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    // C1
    xCoordinate = ASN1_INTEGER_to_BN(sm2cipher->xCoordinate, NULL);
    yCoordinate = ASN1_INTEGER_to_BN(sm2cipher->yCoordinate, NULL);
    // C3
    hashLen = M_ASN1_STRING_length(sm2cipher->hash);
    hash = M_ASN1_STRING_data(sm2cipher->hash);
    // C2
    cipherTextLen = M_ASN1_STRING_length(sm2cipher->cipherText);
    cipherText = M_ASN1_STRING_data(sm2cipher->cipherText);

    xlen = BN_bn2bin(xCoordinate, &xy[0]);
    ylen = BN_bn2bin(yCoordinate, &xy[32]);
    ippsSetOctString_BN(&xy[0], xlen, x);
    ippsSetOctString_BN(&xy[32], ylen, y);

    ippsSetOctString_BN(prvkey, keylen, d);         // 证书私钥

    ippsECCPSetPoint(x, y, p, eccp);                // pkG
    ippsECCPMulPointScalar(p, d, pk, eccp);         // pdkG = d * pkG
    ippsECCPGetPoint(x, y, pk, eccp);

    // pdkG = d * pkG = d * [ k * pG ] = k * [ d * pG ] = k * pB = pkB
    ippsGetOctString_BN(&xy[0], 32, x);
    ippsGetOctString_BN(&xy[32], 32, y);

    plainText = (Ipp8u*)malloc(cipherTextLen);
    ipp_kdf_sm3(xy, 64, plainText, cipherTextLen);

    for(i = 0; i < cipherTextLen; i++)
        plainText[i] ^= cipherText[i];              // 明文数据

    sm3Ctx = ipp_sm3_new();
    ipp_sm3_init(sm3Ctx);
    ipp_sm3_update(sm3Ctx, &xy[0], 32);
    ipp_sm3_update(sm3Ctx, plainText, cipherTextLen);
    ipp_sm3_update(sm3Ctx, &xy[32], 32);
    ipp_sm3_final(sm3Ctx, md);
    if(sm3Ctx) ipp_sm3_delete(sm3Ctx);

    if(hashLen != 32 || memcmp(hash, md, hashLen) != 0)
    {
        fprintf(stderr, "%s %s:%u - memcmp digest failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    memcpy(out, plainText, cipherTextLen);

    if(plainText) free(plainText);
    if(xCoordinate) BN_free(xCoordinate);
    if(yCoordinate) BN_free(yCoordinate);
    if(sm2cipher) SM2Cipher_free(sm2cipher);
    if(x) free(x);
    if(y) free(y);
    if(d) free(d);
    if(p) free(p);
    if(pk) free(pk);
    if(eccp) free(eccp);
    return cipherTextLen;
ErrP:
    if(plainText) free(plainText);
    if(xCoordinate) BN_free(xCoordinate);
    if(yCoordinate) BN_free(yCoordinate);
    if(sm2cipher) SM2Cipher_free(sm2cipher);
    if(x) free(x);
    if(y) free(y);
    if(d) free(d);
    if(p) free(p);
    if(pk) free(pk);
    if(eccp) free(eccp);
    return 0;
}

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
    )
{
    IppStatus status = ippStsNoErr;
    IppECResult result = ippECValid;
    IppsECCPState *eccp = newSM2_ECCP(SM2_BITS);                    // SM2椭圆曲线
    IppsBigNumState *self_tmp_bn = newSM2_BN(SM2_INTS, NULL);       // 本端临时私钥 rA or rB
    IppsBigNumState *self_enc_bn = newSM2_BN(SM2_INTS, NULL);       // 本端加密证书私钥 dA or dB
    IppsECCPPointState *self_tmp_point = newSM2_Point(SM2_BITS);    // 本端临时公钥 RA or RB - x1bar
    IppsECCPPointState *peer_tmp_point = newSM2_Point(SM2_BITS);    // 对端临时公钥 RB or RA - x2bar
    IppsECCPPointState *peer_enc_point = newSM2_Point(SM2_BITS);    // 对端加密证书公钥 PB or PA
    IppsBigNumState *x = newSM2_BN(SM2_INTS, NULL);
    IppsBigNumState *y = newSM2_BN(SM2_INTS, NULL);
    IppsBigNumState *x1bar = newSM2_BN(SM2_INTS, NULL);
    IppsBigNumState *x2bar = newSM2_BN(SM2_INTS, NULL);
    IppsBigNumState *h = newSM2_BN(SM2_INTS, sm2h);
    IppsBigNumState *n = newSM2_BN(SM2_INTS, sm2n);
    IppsMontState *mn = newSM2_Mont(SM2_INTS, sm2n);
    IppsBigNumState *r = newSM2_BN(SM2_INTS+1, NULL);
    IppsECCPPointState *point = newSM2_Point(SM2_BITS);

    Ipp8u pxyzab[128] = {0};

    // 本端临时私钥
    ippsSetOctString_BN(self_tmp_prvkey, self_tmp_prvkey_len, self_tmp_bn);
    // 本端加密证书私钥
    ippsSetOctString_BN(self_enc_prvkey, self_enc_prvkey_len, self_enc_bn);

    // 本端临时公钥
    if(self_tmp_pubkey_len == 65 && self_tmp_pubkey[0] == 0x04)
    {
        ippsSetOctString_BN(&self_tmp_pubkey[1], 32, x);
        ippsSetOctString_BN(&self_tmp_pubkey[33], 32, y);
    }
    else
    {
        ippsSetOctString_BN(&self_tmp_pubkey[0], 32, x);
        ippsSetOctString_BN(&self_tmp_pubkey[32], 32, y);
    }
    ippsECCPSetPoint(x, y, self_tmp_point, eccp);
    ipp_get_x(x, x1bar);
    // 对端临时公钥
    if(peer_tmp_pubkey_len == 65 && peer_tmp_pubkey[0] == 0x04)
    {
        ippsSetOctString_BN(&peer_tmp_pubkey[1], 32, x);
        ippsSetOctString_BN(&peer_tmp_pubkey[33], 32, y);
    }
    else
    {
        ippsSetOctString_BN(&peer_tmp_pubkey[0], 32, x);
        ippsSetOctString_BN(&peer_tmp_pubkey[32], 32, y);
    }
    ippsECCPSetPoint(x, y, peer_tmp_point, eccp);
    ipp_get_x(x, x2bar);
    // 对端加密证书公钥
    if(peer_enc_pubkey_len == 65 && peer_enc_pubkey[0] == 0x04)
    {
        ippsSetOctString_BN(&peer_enc_pubkey[1], 32, x);
        ippsSetOctString_BN(&peer_enc_pubkey[33], 32, y);
    }
    else
    {
        ippsSetOctString_BN(&peer_enc_pubkey[0], 32, x);
        ippsSetOctString_BN(&peer_enc_pubkey[32], 32, y);
    }
    ippsECCPSetPoint(x, y, peer_enc_point, eccp);

    // t = (d + xbar * r) mod n
    ippsMontForm(x1bar, mn, x1bar);
    status = ippsMontMul(x1bar, self_tmp_bn, mn, r);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }
    ippsAdd_BN(self_enc_bn, r, r);
    ippsMod_BN(r, n, r);

    // [h * t](P + xbar * R) = (x, y)
    ippsMul_BN(h, r, r);
    ippsECCPMulPointScalar(peer_tmp_point, x2bar, point, eccp);
    ippsECCPAddPoint(peer_enc_point, point, point, eccp);
    ippsECCPMulPointScalar(point, r, point, eccp);

    status = ippsECCPCheckPoint(point, &result, eccp);
    if(status != ippStsNoErr || result != ippECValid)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippsECCGetResultString(result));
        goto ErrP;
    }
    ippsECCPGetPoint(x, y, point, eccp);

    ippsGetOctString_BN(&pxyzab[0], 32, x);
    ippsGetOctString_BN(&pxyzab[32], 32, y);
    if(is_server)
    {
        ipp_get_z(self_id, self_id_len, self_enc_pubkey, self_enc_pubkey_len, &pxyzab[64]);
        ipp_get_z(peer_id, peer_id_len, peer_enc_pubkey, peer_enc_pubkey_len, &pxyzab[96]);
    }
    else
    {
        ipp_get_z(peer_id, peer_id_len, peer_enc_pubkey, peer_enc_pubkey_len, &pxyzab[64]);
        ipp_get_z(self_id, self_id_len, self_enc_pubkey, self_enc_pubkey_len, &pxyzab[96]);
    }
    ipp_kdf_sm3(pxyzab, 128, session_key, session_key_len);

    if(point) free(point);
    if(r) free(r);
    if(n) free(n);
    if(mn) free(mn);
    if(h) free(h);
    if(x1bar) free(x1bar);
    if(x2bar) free(x2bar);
    if(x) free(x);
    if(y) free(y);
    if(self_tmp_bn) free(self_tmp_bn);
    if(self_enc_bn) free(self_enc_bn);
    if(self_tmp_point) free(self_tmp_point);
    if(peer_tmp_point) free(peer_tmp_point);
    if(peer_enc_point) free(peer_enc_point);
    if(eccp) free(eccp);
    return session_key_len;
ErrP:
    if(point) free(point);
    if(r) free(r);
    if(n) free(n);
    if(mn) free(mn);
    if(h) free(h);
    if(x1bar) free(x1bar);
    if(x2bar) free(x2bar);
    if(x) free(x);
    if(y) free(y);
    if(self_tmp_bn) free(self_tmp_bn);
    if(self_enc_bn) free(self_enc_bn);
    if(self_tmp_point) free(self_tmp_point);
    if(peer_tmp_point) free(peer_tmp_point);
    if(peer_enc_point) free(peer_enc_point);
    if(eccp) free(eccp);
    return 0;
}
#endif
