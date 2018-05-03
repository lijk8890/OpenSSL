#ifdef OPENSSL_WITH_INTEL
#ifndef __SM3_IPP_H__
#define __SM3_IPP_H__

#include "../../ipp/include/ipp.h"
#include "../../ipp/include/ippcp.h"

#define SM3_DIGEST_LENGTH 32
#define SM3_BLOCK_SIZE 64

#ifdef __cplusplus
extern "C"
{
#endif

IppsSM3State* ipp_sm3_new(void);

int ipp_sm3_init(IppsSM3State *ctx);

int ipp_sm3_update(IppsSM3State *ctx, unsigned char *msg, int len);

int ipp_sm3_final(IppsSM3State *ctx, unsigned char *md);

void ipp_sm3_delete(IppsSM3State *ctx);

unsigned char* ipp_sm3(unsigned char *msg, int len, unsigned char *md);

#ifdef __cplusplus
}
#endif

#endif
#endif
