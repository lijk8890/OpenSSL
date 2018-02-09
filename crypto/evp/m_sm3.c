#include <openssl/evp.h>
#include <openssl/sm2.h>
#include <openssl/sm3.h>

static int init(EVP_MD_CTX *ctx)
{
    return sm3_init(ctx->md_data);
}

static int update(EVP_MD_CTX *ctx, const void *in, size_t inlen)
{
    return sm3_update(ctx->md_data, in, inlen);
}

static int final(EVP_MD_CTX *ctx, unsigned char *md)
{
    return sm3_final(md, ctx->md_data);
}

static const EVP_MD sm3_md = {
    NID_sm3,
    NID_sm3withsm2,
    SM3_DIGEST_LENGTH,
    EVP_MD_FLAG_PKEY_DIGEST,
    init,
    update,
    final,
    NULL,
    NULL,
    (evp_sign_method*)SM2_sign,
    (evp_sign_method*)SM2_verify,
    {EVP_PKEY_EC, 0, 0, 0},
    SM3_BLOCK_SIZE,
    sizeof(EVP_MD*) + sizeof(SM3_CTX),
};

const EVP_MD *EVP_sm3(void)
{
    return(&sm3_md);
}
