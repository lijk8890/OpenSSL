#ifdef OPENSSL_WITH_INTEL
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "sms4_ipp.h"

#ifdef _WIN32
#pragma comment(lib, "ippcpmt.lib")
#pragma comment(lib, "ippcoremt.lib")
#endif

int ipp_sms4_ecb_encrypt(unsigned char *src, unsigned char *dst, int len, unsigned char *key)
{
    int size = 0;
    IppsSMS4Spec *ctx = NULL;
    IppStatus status = ippStsNoErr;

    status = ippsSMS4GetSize(&size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        return 0;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    {
        fprintf(stderr, "%s %s:%u - %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4EncryptECB(src, dst, len, ctx);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    if(ctx) free(ctx);
    return 1;
ErrP:
    if(ctx) free(ctx);
    return 0;
}

int ipp_sms4_ecb_decrypt(unsigned char *src, unsigned char *dst, int len, unsigned char *key)
{
    int size = 0;
    IppsSMS4Spec *ctx = NULL;
    IppStatus status = ippStsNoErr;

    status = ippsSMS4GetSize(&size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        return 0;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    {
        fprintf(stderr, "%s %s:%u - %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4DecryptECB(src, dst, len, ctx);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    if(ctx) free(ctx);
    return 1;
ErrP:
    if(ctx) free(ctx);
    return 0;
}

int ipp_sms4_cbc_encrypt(unsigned char *src, unsigned char *dst, int len, unsigned char *key, unsigned char *iv)
{
    int size = 0;
    IppsSMS4Spec *ctx = NULL;
    IppStatus status = ippStsNoErr;

    status = ippsSMS4GetSize(&size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        return 0;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    {
        fprintf(stderr, "%s %s:%u - %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4EncryptCBC(src, dst, len, ctx, iv);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    if(ctx) free(ctx);
    return 1;
ErrP:
    if(ctx) free(ctx);
    return 0;
}

int ipp_sms4_cbc_decrypt(unsigned char *src, unsigned char *dst, int len, unsigned char *key, unsigned char *iv)
{
    int size = 0;
    IppsSMS4Spec *ctx = NULL;
    IppStatus status = ippStsNoErr;

    status = ippsSMS4GetSize(&size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        return 0;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    {
        fprintf(stderr, "%s %s:%u - %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4DecryptCBC(src, dst, len, ctx, iv);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    if(ctx) free(ctx);
    return 1;
ErrP:
    if(ctx) free(ctx);
    return 0;
}


int ipp_sms4_ecb_encrypt_auto_padding(unsigned char *src, unsigned char *dst, int len, unsigned char *key)
{
    int i = 0;
    int padding = 0;
    int length = 0;
    Ipp8u *buffer = NULL;

    int size = 0;
    IppsSMS4Spec *ctx = NULL;
    IppStatus status = ippStsNoErr;

    padding = SMS4_BLOCK_SIZE - (len % SMS4_BLOCK_SIZE);
    length = len + padding;

    buffer = (Ipp8u*)malloc(length*sizeof(Ipp8u));
    if(buffer == NULL)
    {
        fprintf(stderr, "%s %s:%u - %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }
    memcpy(buffer, src, len);

    for(i = 0; i < padding; i ++)
        buffer[len + i] = padding;

    status = ippsSMS4GetSize(&size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    {
        fprintf(stderr, "%s %s:%u - %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        goto ErrP;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4EncryptECB(buffer, dst, length, ctx);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    if(buffer) free(buffer);
    if(ctx) free(ctx);
    return length;
ErrP:
    if(buffer) free(buffer);
    if(ctx) free(ctx);
    return 0;
}

int ipp_sms4_ecb_decrypt_auto_padding(unsigned char *src, unsigned char *dst, int len, unsigned char *key)
{
    int padding = 0;
    int length = 0;

    int size = 0;
    IppsSMS4Spec *ctx = NULL;
    IppStatus status = ippStsNoErr;

    status = ippsSMS4GetSize(&size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        return 0;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    {
        fprintf(stderr, "%s %s:%u - %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4DecryptECB(src, dst, len, ctx);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    padding = dst[len - 1];
    length = len - padding;

    if(ctx) free(ctx);
    return length;
ErrP:
    if(ctx) free(ctx);
    return 0;
}

int ipp_sms4_cbc_encrypt_auto_padding(unsigned char *src, unsigned char *dst, int len, unsigned char *key, unsigned char *iv)
{
    int i = 0;
    int padding = 0;
    int length = 0;
    Ipp8u *buffer = NULL;

    int size = 0;
    IppsSMS4Spec *ctx = NULL;
    IppStatus status = ippStsNoErr;

    padding = SMS4_BLOCK_SIZE - (len % SMS4_BLOCK_SIZE);
    length = len + padding;

    buffer = (Ipp8u*)malloc(length*sizeof(Ipp8u));
    if(buffer == NULL)
    {
        fprintf(stderr, "%s %s:%u - %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }
    memcpy(buffer, src, len);

    for(i = 0; i < padding; i ++)
        buffer[len + i] = padding;

    status = ippsSMS4GetSize(&size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    {
        fprintf(stderr, "%s %s:%u - %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        goto ErrP;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4EncryptCBC(buffer, dst, length, ctx, iv);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    if(buffer) free(buffer);
    if(ctx) free(ctx);
    return length;
ErrP:
    if(buffer) free(buffer);
    if(ctx) free(ctx);
    return 0;
}

int ipp_sms4_cbc_decrypt_auto_padding(unsigned char *src, unsigned char *dst, int len, unsigned char *key, unsigned char *iv)
{
    int padding = 0;
    int length = 0;

    int size = 0;
    IppsSMS4Spec *ctx = NULL;
    IppStatus status = ippStsNoErr;

    status = ippsSMS4GetSize(&size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        return 0;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    {
        fprintf(stderr, "%s %s:%u - %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        return 0;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4DecryptCBC(src, dst, len, ctx, iv);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    padding = dst[len - 1];
    length = len - padding;

    if(ctx) free(ctx);
    return length;
ErrP:
    if(ctx) free(ctx);
    return 0;
}
#endif
