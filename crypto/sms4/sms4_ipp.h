#ifdef OPENSSL_WITH_INTEL
#ifndef __SMS4_IPP_H__
#define __SMS4_IPP_H__

#include "ipp.h"
#include "ippcp.h"

#define SMS4_KEY_LENGTH 16
#define SMS4_BLOCK_SIZE 16

#ifdef __cplusplus
extern "C"
{
#endif

int ipp_sms4_ecb_encrypt(unsigned char *src, unsigned char *dst, int len, unsigned char *key);

int ipp_sms4_ecb_decrypt(unsigned char *src, unsigned char *dst, int len, unsigned char *key);

int ipp_sms4_cbc_encrypt(unsigned char *src, unsigned char *dst, int len, unsigned char *key, unsigned char *iv);

int ipp_sms4_cbc_decrypt(unsigned char *src, unsigned char *dst, int len, unsigned char *key, unsigned char *iv);


int ipp_sms4_ecb_encrypt_with_padding(unsigned char *src, unsigned char *dst, int len, unsigned char *key);

int ipp_sms4_ecb_decrypt_with_padding(unsigned char *src, unsigned char *dst, int len, unsigned char *key);

int ipp_sms4_cbc_encrypt_with_padding(unsigned char *src, unsigned char *dst, int len, unsigned char *key, unsigned char *iv);

int ipp_sms4_cbc_decrypt_with_padding(unsigned char *src, unsigned char *dst, int len, unsigned char *key, unsigned char *iv);

#ifdef __cplusplus
}
#endif

#endif
#endif
