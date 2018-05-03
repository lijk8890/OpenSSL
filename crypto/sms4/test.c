#include <stdio.h>
#include <unistd.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

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

static int evp_cipher_encrypt(const EVP_CIPHER *cipher, unsigned char *key, unsigned char *iv, int enc, unsigned char *in, int inlen, unsigned char *out, int *outlen)
{
    int ret;
    int outl;
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);

    *outlen = 0;
    ret = EVP_CipherInit_ex(&ctx, cipher, NULL, key, iv, enc);
    if(ret != 1)
    {
        fprintf(stderr, "%s %s:%u - EVP_CipherInit_ex failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    ret = EVP_CipherUpdate(&ctx, out, &outl, in, inlen);
    if(ret != 1)
    {
        fprintf(stderr, "%s %s:%u - EVP_CipherUpdate failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    *outlen += outl;

    ret = EVP_CipherFinal_ex(&ctx, out + *outlen, &outl);
    if(ret != 1)
    {
        fprintf(stderr, "%s %s:%u - EVP_CipherFinal_ex failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    *outlen += outl;

    EVP_CIPHER_CTX_cleanup(&ctx);
    return 1;
ErrP:
    EVP_CIPHER_CTX_cleanup(&ctx);
    return 0;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int inlen = 32;
    unsigned char in[2048] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    int outlen = 0;
    unsigned char out[2048] = {0};

    unsigned char iv[16] = {0};
    unsigned char key[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };

do{
    // 加密
    ret = evp_cipher_encrypt(EVP_sms4_cbc(), key, iv, 1, in, inlen, out, &outlen);
    if(ret != 1)
    {
        fprintf(stderr, "%s %s:%u - SMS4 Encrypt failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    fprintf(stdout, "%s %s:%u - SMS4 Encrypt succeed:\n", __FUNCTION__, __FILE__, __LINE__);
    PRINT_HEX(out, outlen);

    // 解密
    ret = evp_cipher_encrypt(EVP_sms4_cbc(), key, iv, 0, out, outlen, in, &inlen);
    if(ret != 1)
    {
        fprintf(stderr, "%s %s:%u - SMS4 Decrypt failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    fprintf(stdout, "%s %s:%u - SMS4 Decrypt succeed:\n", __FUNCTION__, __FILE__, __LINE__);
    PRINT_HEX(in, inlen);

    usleep(100);
}while(0);

    return 0;
ErrP:
    return -1;
}
