#include <openssl/evp.h>
#include <openssl/sms4.h>

static int sms4_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key, const unsigned char *iv, int enc)
{
    if(enc)
        sms4_set_encrypt_key(ctx->cipher_data, key);
    else
        sms4_set_decrypt_key(ctx->cipher_data, key);
    return 1;
}

static int sms4_cbc_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t len)
{
    sms4_cbc_encrypt(in, out, len, ctx->cipher_data, ctx->iv, ctx->encrypt);
    return 1;
}

static int sms4_ecb_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t len)
{
    sms4_ecb_encrypt(in, out, ctx->cipher_data, ctx->encrypt);
    return 1;
}

static const EVP_CIPHER sms4 = {
    NID_sms4,
    SMS4_BLOCK_SIZE, SMS4_KEY_LENGTH, SMS4_IV_LENGTH,
    EVP_CIPH_FLAG_DEFAULT_ASN1 | EVP_CIPH_CBC_MODE,
    sms4_init_key,
    sms4_cbc_cipher,
    NULL,
    sizeof(sms4_key_t),
    NULL, NULL, NULL, NULL
};

static const EVP_CIPHER sms4_cbc = {
    NID_sms4_cbc,
    SMS4_BLOCK_SIZE, SMS4_KEY_LENGTH, SMS4_IV_LENGTH,
    EVP_CIPH_FLAG_DEFAULT_ASN1 | EVP_CIPH_CBC_MODE,
    sms4_init_key,
    sms4_cbc_cipher,
    NULL,
    sizeof(sms4_key_t),
    NULL, NULL, NULL, NULL
};

static const EVP_CIPHER sms4_ecb = {
    NID_sms4_ecb,
    SMS4_BLOCK_SIZE, SMS4_KEY_LENGTH, 0,
    EVP_CIPH_FLAG_DEFAULT_ASN1 | EVP_CIPH_ECB_MODE,
    sms4_init_key,
    sms4_ecb_cipher,
    NULL,
    sizeof(sms4_key_t),
    NULL, NULL, NULL, NULL
};

const EVP_CIPHER *EVP_sms4(void)
{
    return &sms4;
}

const EVP_CIPHER *EVP_sms4_cbc(void)
{
    return &sms4_cbc;
}

const EVP_CIPHER *EVP_sms4_ecb(void)
{
    return &sms4_ecb;
}
