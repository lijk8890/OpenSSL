#include "sm2.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_be16(v) ((v << 8) | (v >> 8))
#define cpu_to_be32(v) ((cpu_to_be16(v) << 16) | cpu_to_be16(v >> 16))
#else
#define cpu_to_be16(v) (v)
#define cpu_to_be32(v) (v)
#endif

// a
static unsigned char sm2a[] = {
    0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC
};

// b
static unsigned char sm2b[] = {
    0x28, 0xE9, 0xFA, 0x9E, 0x9D, 0x9F, 0x5E, 0x34, 0x4D, 0x5A, 0x9E, 0x4B, 0xCF, 0x65, 0x09, 0xA7,
    0xF3, 0x97, 0x89, 0xF5, 0x15, 0xAB, 0x8F, 0x92, 0xDD, 0xBC, 0xBD, 0x41, 0x4D, 0x94, 0x0E, 0x93
};

// Gx
static unsigned char sm2Gx[] = {
    0x32, 0xC4, 0xAE, 0x2C, 0x1F, 0x19, 0x81, 0x19, 0x5F, 0x99, 0x04, 0x46, 0x6A, 0x39, 0xC9, 0x94,
    0x8F, 0xE3, 0x0B, 0xBF, 0xF2, 0x66, 0x0B, 0xE1, 0x71, 0x5A, 0x45, 0x89, 0x33, 0x4C, 0x74, 0xC7
};

// Gy
static unsigned char sm2Gy[] = {
    0xBC, 0x37, 0x36, 0xA2, 0xF4, 0xF6, 0x77, 0x9C, 0x59, 0xBD, 0xCE, 0xE3, 0x6B, 0x69, 0x21, 0x53,
    0xD0, 0xA9, 0x87, 0x7C, 0xC6, 0x2A, 0x47, 0x40, 0x02, 0xDF, 0x32, 0xE5, 0x21, 0x39, 0xF0, 0xA0
};

int kdf_sm3(unsigned char *in, int inlen, unsigned char *out, int outlen)
{
    unsigned int counter = 1;
    unsigned int counter_be;
    unsigned char md[SM3_DIGEST_LENGTH];
    sm3_ctx_t ctx;

    while(outlen > 0)
    {
        counter_be = cpu_to_be32(counter);
        counter++;

        sm3_init(&ctx);
        sm3_update(&ctx, in, inlen);
        sm3_update(&ctx, (unsigned char*)&counter_be, 4);
        sm3_final(&ctx, md);

        memcpy(out, md, outlen < SM3_DIGEST_LENGTH ? outlen : SM3_DIGEST_LENGTH);
        outlen -= SM3_DIGEST_LENGTH;
        out += SM3_DIGEST_LENGTH;
    }

    return 1;
}

int get_z(const char *id, int id_len, unsigned char *pubkey, int pubkey_len, unsigned char md[SM3_DIGEST_LENGTH])
{
    int entl;
    unsigned char b[2];
    sm3_ctx_t ctx;

    entl = id_len * 8;
    b[0] = (entl >> 8) % 256;
    b[1] = (entl >> 0) % 256;

    sm3_init(&ctx);
    sm3_update(&ctx, b, 2);
    sm3_update(&ctx, (unsigned char*)id, id_len);
    sm3_update(&ctx, sm2a, 32);
    sm3_update(&ctx, sm2b, 32);
    sm3_update(&ctx, sm2Gx, 32);
    sm3_update(&ctx, sm2Gy, 32);
    if(pubkey_len == 65 && pubkey[0] == 0x04)
        sm3_update(&ctx, &pubkey[1], 64);
    else
        sm3_update(&ctx, &pubkey[0], 64);
    sm3_final(&ctx, md);

    return 1;
}

void print_bn(BIGNUM *bn)
{
    int loop = 0;
    int length = 0;
    unsigned char buffer[128] = {0};
    length = BN_bn2bin(bn, buffer);
    for(loop = 0; loop < length; loop++)
        fprintf(stdout, "0x%02hhx%c", buffer[loop], (loop + 1) % 16 == 0 ? '\n' : ' ');
    fprintf(stdout, "\n");
}

void print_point(const EC_GROUP *group, EC_POINT *point)
{
    int loop = 0;
    int length = 0;
    unsigned char buffer[256+1] = {0};
    length = EC_POINT_point2oct(group, point, POINT_CONVERSION_UNCOMPRESSED, buffer, 256+1, NULL);
    for(loop = 1; loop < length; loop++)
        fprintf(stdout, "0x%02hhx%c", buffer[loop], (loop + 0) % 16 == 0 ? '\n' : ' ');
    fprintf(stdout, "\n");
}

// unsigned char prvkey[32];
int get_prvkey_from_ec_key(EC_KEY *ec_key, unsigned char *out)
{
    return BN_bn2bin(EC_KEY_get0_private_key(ec_key), out);
}

// unsigned char pubkey[64+1];
int get_pubkey_from_ec_key(EC_KEY *ec_key, unsigned char *out, int len)
{
    return EC_POINT_point2oct(EC_KEY_get0_group(ec_key), EC_KEY_get0_public_key(ec_key), POINT_CONVERSION_UNCOMPRESSED, out, len, NULL);
}
