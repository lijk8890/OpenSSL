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

static int evp_digest_encrypt(const EVP_MD *digest, unsigned char *in, int inlen, unsigned char *md, int *mdlen)
{
    int ret;
    EVP_MD_CTX ctx;
    EVP_MD_CTX_init(&ctx);

    ret = EVP_DigestInit_ex(&ctx, digest, NULL);
    if(ret != 1)
    {
        fprintf(stderr, "%s %s:%u - EVP_DigestInit_ex failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    ret = EVP_DigestUpdate(&ctx, in, inlen);
    if(ret != 1)
    {
        fprintf(stderr, "%s %s:%u - EVP_DigestUpdate failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    ret = EVP_DigestFinal_ex(&ctx, md, (unsigned int*)mdlen);
    if(ret != 1)
    {
        fprintf(stderr, "%s %s:%u - EVP_DigestFinal_ex failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    EVP_MD_CTX_cleanup(&ctx);
    return 1;
ErrP:
    EVP_MD_CTX_cleanup(&ctx);
    return 0;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int inlen = 10;
    unsigned char in[] = "0123456789";
    int mdlen = 0;
    unsigned char md[32] = {0};

do{
    // 哈希
    ret = evp_digest_encrypt(EVP_sm3(), in, inlen, md, &mdlen);
    if(ret != 1)
    {
        fprintf(stderr, "%s %s:%u - SM3 Digest failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    fprintf(stdout, "%s %s:%u - SM3 Digest succeed:\n", __FUNCTION__, __FILE__, __LINE__);
    PRINT_HEX(md, mdlen);

    usleep(100);
}while(0);

    return 0;
ErrP:
    return -1;
}
