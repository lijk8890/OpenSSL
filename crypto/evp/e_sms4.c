#include <openssl/evp.h>
#include <openssl/sms4.h>

typedef struct {

} EVP_SMS4_KEY;

static int sms4_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key, const unsigned char *iv, int enc)
{
    return 0;
}

static int sms4_cbc_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t len)
{
    return 0;
}

static int sms4_ecb_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t len)
{
    return 0;
}

static const EVP_CIPHER sms4 = {
    NID_sms4,
    SMS4_BLOCK_SIZE, SMS4_KEY_LENGTH, SMS4_IV_LENGTH,
    EVP_CIPH_FLAG_DEFAULT_ASN1 | EVP_CIPH_CBC_MODE,
    sms4_init_key,
    sms4_cbc_cipher,
    NULL,
    sizeof(EVP_SMS4_KEY),
    NULL, NULL, NULL, NULL
};

static const EVP_CIPHER sms4_cbc = {
    NID_sms4_cbc,
    SMS4_BLOCK_SIZE, SMS4_KEY_LENGTH, SMS4_IV_LENGTH,
    EVP_CIPH_FLAG_DEFAULT_ASN1 | EVP_CIPH_CBC_MODE,
    sms4_init_key,
    sms4_cbc_cipher,
    NULL,
    sizeof(EVP_SMS4_KEY),
    NULL, NULL, NULL, NULL
};

static const EVP_CIPHER sms4_ecb = {
    NID_sms4_ecb,
    SMS4_BLOCK_SIZE, SMS4_KEY_LENGTH, 0,
    EVP_CIPH_FLAG_FIPS | EVP_CIPH_FLAG_DEFAULT_ASN1 | EVP_CIPH_ECB_MODE,
    sms4_init_key,
    sms4_ecb_cipher,
    NULL,
    sizeof(EVP_SMS4_KEY),
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
