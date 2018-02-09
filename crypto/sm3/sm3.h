#ifndef __SM3_H__
#define __SM3_H__

#define SM3_DIGEST_LENGTH   32
#define SM3_BLOCK_SIZE      64

typedef struct {

} SM3_CTX;

int sm3_init(SM3_CTX *ctx);

int sm3_update(SM3_CTX *ctx, const unsigned char *data, int len);

int sm3_final(unsigned char *md, SM3_CTX *ctx);

unsigned char *SM3(const unsigned char *data, int len, unsigned char *md);

#endif
