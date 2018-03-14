#include <stdio.h>
#include <unistd.h>
#include <openssl/sm2.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#ifndef PRINT_HEX
#define PRINT_HEX(buf, len)                                                                 \
    do{                                                                                     \
        if(buf != NULL && len > 0)                                                          \
        {                                                                                   \
            int loop = 0;                                                                   \
            for(loop = 0; loop < len; loop++)                                               \
                printf("0x%02hhx%s", buf[loop], (loop+1) % 16 != 0 ? ", " : ",\n");         \
            printf("\n");                                                                   \
        }                                                                                   \
    }while(0);
#endif

int main(int argc, char *argv[])
{
    int ret = 0;
    unsigned char out[48] = {0};
    unsigned char prvkey[32] = {
        0x64, 0x06, 0xa2, 0x5a, 0xde, 0xe3, 0xe9, 0x85,
        0x56, 0x6a, 0x17, 0xf1, 0xca, 0xbd, 0xd2, 0xee,
        0xa8, 0x07, 0x21, 0xe2, 0x01, 0x81, 0x07, 0x93,
        0x27, 0x2c, 0x2f, 0xfb, 0xb5, 0x27, 0xdf, 0x5e
    };
    unsigned char pubkey[65] = {
        0x04, 0x86, 0xa3, 0x23, 0xa6, 0x82, 0x91, 0x4f,
        0x2a, 0x82, 0xe4, 0x00, 0xe8, 0x7d, 0xd0, 0x9f,
        0x2f, 0x43, 0xd0, 0x56, 0xf8, 0xd3, 0xb9, 0x87,
        0x47, 0x15, 0xca, 0x41, 0xf7, 0xc2, 0x27, 0x72,
        0x60, 0xf4, 0x5a, 0x3f, 0x9f, 0x46, 0xd3, 0x90,
        0xc3, 0x0b, 0x98, 0xf4, 0xfa, 0x12, 0x6c, 0x64,
        0x31, 0x3e, 0xa0, 0xcd, 0xad, 0x8a, 0x3d, 0x7a,
        0x5f, 0x5b, 0x86, 0xc1, 0x16, 0x4d, 0x30, 0x34,
        0x0c
    };

    int      nid = 0;
    EC_KEY  *ecdh = 0;
    const EC_GROUP *group = NULL;

    nid = OBJ_sn2nid("sm2p256v1");
    ecdh = EC_KEY_new_by_curve_name(nid);
    group = EC_KEY_get0_group(ecdh);

do{
    ret = sm2_compute_key(group, "1234567812345678", 16, prvkey, 32, pubkey, 65, prvkey, 32, pubkey, 65, pubkey, 65, pubkey, 65, out, 48, 1);
    PRINT_HEX(out, ret);

    usleep(100);
}while(0);

    EC_KEY_free(ecdh);
    return 0;
}
