#ifdef OPENSSL_WITH_INTEL
#include <stdio.h>
#include <stdlib.h>
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
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4EncryptECB(src, dst, len, ctx);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
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
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4DecryptECB(src, dst, len, ctx);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
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
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4EncryptCBC(src, dst, len, ctx, iv);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
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
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    ctx = (IppsSMS4Spec*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4Init(key, SMS4_KEY_LENGTH, ctx, size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    status = ippsSMS4DecryptCBC(src, dst, len, ctx, iv);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, ippGetStatusString(status));
        goto ErrP;
    }

    if(ctx) free(ctx);
    return 1;
ErrP:
    if(ctx) free(ctx);
    return 0;
}
#endif
