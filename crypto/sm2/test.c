#include <stdio.h>
#include <unistd.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/sm2.h>
#include <openssl/sm2_ipp.h>

#ifndef PRINT_HEX
#define PRINT_HEX(buf, len)                                                                 \
    do{                                                                                     \
        if(buf != NULL && len > 0)                                                          \
        {                                                                                   \
            int loop = 0;                                                                   \
            for(loop = 0; loop < len; loop++)                                               \
                printf("0x%02hhx%s", buf[loop], (loop+1) % 16 != 0 ? ", " : ",\n");         \
            printf("\n");                                                                   \
        }                                                                                   \
    }while(0);
#endif

int main(int argc, char *argv[])
{
    int ret = 0;
    int inlen = 0;
    int outlen = 0;
    unsigned char in[256] = "0123456789";
    unsigned char out[256] = {0};
    unsigned char prvkey[32] = {
        0x64, 0x06, 0xa2, 0x5a, 0xde, 0xe3, 0xe9, 0x85,
        0x56, 0x6a, 0x17, 0xf1, 0xca, 0xbd, 0xd2, 0xee,
        0xa8, 0x07, 0x21, 0xe2, 0x01, 0x81, 0x07, 0x93,
        0x27, 0x2c, 0x2f, 0xfb, 0xb5, 0x27, 0xdf, 0x5e
    };
    unsigned char pubkey[65] = {
        0x04, 0x86, 0xa3, 0x23, 0xa6, 0x82, 0x91, 0x4f,
        0x2a, 0x82, 0xe4, 0x00, 0xe8, 0x7d, 0xd0, 0x9f,
        0x2f, 0x43, 0xd0, 0x56, 0xf8, 0xd3, 0xb9, 0x87,
        0x47, 0x15, 0xca, 0x41, 0xf7, 0xc2, 0x27, 0x72,
        0x60, 0xf4, 0x5a, 0x3f, 0x9f, 0x46, 0xd3, 0x90,
        0xc3, 0x0b, 0x98, 0xf4, 0xfa, 0x12, 0x6c, 0x64,
        0x31, 0x3e, 0xa0, 0xcd, 0xad, 0x8a, 0x3d, 0x7a,
        0x5f, 0x5b, 0x86, 0xc1, 0x16, 0x4d, 0x30, 0x34,
        0x0c
    };
    unsigned char md[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };

    EC_KEY  *ec = 0;
    const EC_GROUP *group = NULL;
    EC_POINT *point = NULL;
    BIGNUM *bn = NULL;

    ec = EC_KEY_new_by_curve_name(NID_sm2p256v1);
    EC_KEY_set_asn1_flag(ec, OPENSSL_EC_NAMED_CURVE);
    group = EC_KEY_get0_group(ec);

    // 设置公钥
    point = EC_POINT_new(group);
    if(!EC_POINT_oct2point(group, point, pubkey, 65, NULL))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_oct2point failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto EndP;
    }
    EC_KEY_set_public_key(ec, point);

    // 设置私钥
    bn = BN_bin2bn(prvkey, 32, NULL);
    if(bn == NULL)
    {
        fprintf(stderr, "%s %s:%u - BN_bin2bn failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto EndP;
    }
    EC_KEY_set_private_key(ec, bn);

    // 校验密钥
    if(!EC_KEY_check_key(ec))
    {
        fprintf(stderr, "%s %s:%u - EC_KEY_check_key failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto EndP;
    }

do{
#ifdef OPENSSL_WITH_INTEL
    // 密钥对
    ret = ipp_sm2_keypair(prvkey, pubkey);
    if(ret <= 0)
    {
        fprintf(stderr, "%s %s:%u - ipp_sm2_keypair failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto EndP;
    }
    fprintf(stdout, "%s %s:%u - ipp_sm2_keypair succeed:\n", __FUNCTION__, __FILE__, __LINE__);
    PRINT_HEX(prvkey, 32);
    PRINT_HEX(pubkey, 65);

    // 客户端
    ret = ipp_sm2_compute_key("1234567812345678", 16, prvkey, 32, pubkey, 65, prvkey, 32, pubkey, 65, pubkey, 65, pubkey, 65, out, 48, 0);
    if(ret <= 0)
    {
        fprintf(stderr, "%s %s:%u - sm2_compute_key failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto EndP;
    }
    fprintf(stdout, "%s %s:%u - sm2_compute_key succeed:\n", __FUNCTION__, __FILE__, __LINE__);
    PRINT_HEX(out, ret);
#endif

    // 服务端
    ret = openssl_sm2_compute_key(group, "1234567812345678", 16, prvkey, 32, pubkey, 65, prvkey, 32, pubkey, 65, pubkey, 65, pubkey, 65, out, 48, 1);
    if(ret <= 0)
    {
        fprintf(stderr, "%s %s:%u - sm2_compute_key failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto EndP;
    }
    fprintf(stdout, "%s %s:%u - sm2_compute_key succeed:\n", __FUNCTION__, __FILE__, __LINE__);
    PRINT_HEX(out, ret);

    // 加密
    outlen = SM2_encrypt(NID_sm3, in, 10, out, ec);
    if(outlen <= 0)
    {
        fprintf(stderr, "%s %s:%u - SM2_encrypt failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto EndP;
    }
    fprintf(stdout, "%s %s:%u - SM2_encrypt succeed:\n", __FUNCTION__, __FILE__, __LINE__);
    PRINT_HEX(out, outlen);

    // 解密
    inlen = SM2_decrypt(NID_sm3, out, outlen, in, ec);
    if(inlen <= 0)
    {
        fprintf(stderr, "%s %s:%u - SM2_decrypt failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto EndP;
    }
    fprintf(stdout, "%s %s:%u - SM2_decrypt succeed:\n", __FUNCTION__, __FILE__, __LINE__);
    PRINT_HEX(in, inlen);

    // 签名
    ret = SM2_sign(NID_sm3, md, 32, out, &outlen, ec);
    if(ret <= 0)
    {
        fprintf(stderr, "%s %s:%u - SM2_sign failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto EndP;
    }
    fprintf(stdout, "%s %s:%u - SM2_sign succeed:\n", __FUNCTION__, __FILE__, __LINE__);
    PRINT_HEX(out, outlen);

    // 验签
    ret = SM2_verify(NID_sm3, md, 32, out, outlen, ec);
    if(ret <= 0)
    {
        fprintf(stderr, "%s %s:%u - SM2_verify failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto EndP;
    }
    fprintf(stdout, "%s %s:%u - SM2_verify succeed:\n", __FUNCTION__, __FILE__, __LINE__);
    PRINT_HEX(md, 32);

    usleep(100);
}while(0);

EndP:
    if(bn) BN_free(bn);
    if(point) EC_POINT_free(point);
    if(ec) EC_KEY_free(ec);
    return 0;
}
