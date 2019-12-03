#ifdef OPENSSL_WITH_INTEL
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "sm3_ipp.h"

#ifdef _WIN32
#pragma comment(lib, "ippcpmt.lib")
#pragma comment(lib, "ippcoremt.lib")
#endif

IppsSM3State* ipp_sm3_new()
{
    int size = 0;
    IppsSM3State *ctx = NULL;
    IppStatus status = ippsSM3GetSize(&size);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        return NULL;
    }
    ctx = (IppsSM3State*)malloc(size*sizeof(Ipp8u));
    if(ctx == NULL)
    {
        fprintf(stderr, "%s %s:%u - %d:%s\n", __FUNCTION__, __FILE__, __LINE__, errno, strerror(errno));
        return NULL;
    }
    return ctx;
}

int ipp_sm3_init(IppsSM3State *ctx)
{
    IppStatus status = ippsSM3Init(ctx);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        return 0;
    }
    return 1;
}

int ipp_sm3_update(IppsSM3State *ctx, unsigned char *msg, int len)
{
    IppStatus status = ippsSM3Update(msg, len, ctx);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        return 0;
    }
    return 1;
}

int ipp_sm3_final(IppsSM3State *ctx, unsigned char *md)
{
    IppStatus status = ippsSM3Final(md, ctx);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        return 0;
    }
    return 1;
}

void ipp_sm3_delete(IppsSM3State *ctx)
{
    if(ctx) free(ctx);
}

unsigned char* ipp_sm3(unsigned char *msg, int len, unsigned char *md)
{
    IppStatus status = ippsSM3MessageDigest(msg, len, md);
    if(status != ippStsNoErr)
    {
        fprintf(stderr, "%s %s:%u - %s\n", __FUNCTION__, __FILE__, __LINE__, ippGetStatusString(status));
        return NULL;
    }
    return md;
}
#endif
