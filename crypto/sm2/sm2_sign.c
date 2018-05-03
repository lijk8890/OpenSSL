#include "sm2.h"
#include "../ecdsa/ecdsa.h"
#include "../objects/objects.h"

int SM2_sign(int type, const unsigned char *dgst, int dgstlen, unsigned char *sig, int *siglen, EC_KEY *ec)
{
    const EC_GROUP *group = EC_KEY_get0_group(ec);
    const BIGNUM *prvkey = EC_KEY_get0_private_key(ec);
    ECDSA_SIG *sm2sign = ECDSA_SIG_new();
    BN_CTX *bn_ctx = BN_CTX_new();
    BIGNUM *n = BN_new();
    BIGNUM *e = BN_new();
    BIGNUM *k = BN_new();
    BIGNUM *x1 = BN_new();
    BIGNUM *tmp = BN_new();
    EC_POINT *kPG = EC_POINT_new(group);        // (x1, y1)

    if(sm2sign == NULL || bn_ctx == NULL || n == NULL || e == NULL || k == NULL \
        || x1 == NULL || tmp == NULL || kPG == NULL)
    {
        fprintf(stderr, "%s %s:%u - SM2_sign failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    BN_CTX_start(bn_ctx);

    /* n */
    if(!EC_GROUP_get_order(group, n, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_GROUP_get_order failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    /* e */
    if(!BN_bin2bn(dgst, dgstlen, e))
    {
        fprintf(stderr, "%s %s:%u - BN_bin2bn failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    do{
        /* k in [1, n-1] */
        do{
            BN_rand_range(k, n);
        }while(BN_is_zero(k));

        /* x1 */
        if(!EC_POINT_mul(group, kPG, k, NULL, NULL, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
        if(EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field)
        {
            if(!EC_POINT_get_affine_coordinates_GFp(group, kPG, x1, NULL, bn_ctx))
            {
                fprintf(stderr, "%s %s:%u - EC_POINT_get_affine_coordinates_GFp failed\n", __FUNCTION__, __FILE__, __LINE__);
                goto ErrP;
            }
        }
        else
        {
            if(!EC_POINT_get_affine_coordinates_GF2m(group, kPG, x1, NULL, bn_ctx))
            {
                fprintf(stderr, "%s %s:%u - EC_POINT_get_affine_coordinates_GF2m failed\n", __FUNCTION__, __FILE__, __LINE__);
                goto ErrP;
            }
        }

        /* r = (e + x1) mod n */
        if(!BN_mod_add(sm2sign->r, e, x1, n, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - BN_mod_add failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
        /* (r + k) mod n != 0 */
        if(!BN_mod_add(tmp, sm2sign->r, k, n, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - BN_mod_add failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
        if(BN_is_zero(sm2sign->r) || BN_is_zero(tmp))
            continue;
        
        /* s = ((1 + d) ^ -1 * (k - r * d)) mod n */
        BN_one(tmp);
        if(!BN_mod_add(sm2sign->s, tmp, prvkey, n, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - BN_mod_add failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
        if(!BN_mod_inverse(sm2sign->s, sm2sign->s, n, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - BN_mod_inverse failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }

        if(!BN_mod_mul(tmp, sm2sign->r, prvkey, n, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - BN_mod_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
        if(!BN_mod_sub(tmp, k, tmp, n, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - BN_mod_sub failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }

        if(!BN_mod_mul(sm2sign->s, sm2sign->s, tmp, n, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - BN_mod_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
    }while(BN_is_zero(sm2sign->s));

    *siglen = i2d_ECDSA_SIG(sm2sign, &sig);

    BN_CTX_end(bn_ctx);
    if(kPG) EC_POINT_free(kPG);
    if(tmp) BN_free(tmp);
    if(x1) BN_free(x1);
    if(k) BN_free(k);
    if(e) BN_free(e);
    if(n) BN_free(n);
    if(bn_ctx) BN_CTX_free(bn_ctx);
    if(sm2sign) ECDSA_SIG_free(sm2sign);
    return 1;
ErrP:
    if(kPG) EC_POINT_free(kPG);
    if(tmp) BN_free(tmp);
    if(x1) BN_free(x1);
    if(k) BN_free(k);
    if(e) BN_free(e);
    if(n) BN_free(n);
    if(bn_ctx) BN_CTX_free(bn_ctx);
    if(sm2sign) ECDSA_SIG_free(sm2sign);
    BN_CTX_end(bn_ctx);
    return 0;
}

int SM2_verify(int type, const unsigned char *dgst, int dgstlen, const unsigned char *sig, int siglen, EC_KEY *ec)
{
    const EC_GROUP *group = EC_KEY_get0_group(ec);
    const EC_POINT *pubkey = EC_KEY_get0_public_key(ec);
    ECDSA_SIG *sm2sign = NULL;
    BN_CTX *bn_ctx = BN_CTX_new();
    BIGNUM *n = BN_new();
    BIGNUM *e = BN_new();
    BIGNUM *t = BN_new();
    BIGNUM *x1 = BN_new();
    BIGNUM *tmp = BN_new();
    EC_POINT *kPG = EC_POINT_new(group);        // (x1, y1)

    sm2sign = d2i_ECDSA_SIG(NULL, &sig, siglen);
    if(sm2sign == NULL)
    {
        fprintf(stderr, "%s %s:%u - d2i_ECDSA_SIG failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    if(sm2sign == NULL || bn_ctx == NULL || n == NULL || e == NULL || t == NULL \
        || x1 == NULL || tmp == NULL || kPG == NULL)
    {
        fprintf(stderr, "%s %s:%u - SM2_verify failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    BN_CTX_start(bn_ctx);

    /* n */
    if(!EC_GROUP_get_order(group, n, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_GROUP_get_order failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    /* e */
    if(!BN_bin2bn(dgst, dgstlen, e))
    {
        fprintf(stderr, "%s %s:%u - BN_bin2bn failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    /* r, s in [1, n-1] */
    if(BN_is_zero(sm2sign->r) || BN_is_negative(sm2sign->r) || BN_ucmp(sm2sign->r, n) >= 0 \
        || BN_is_zero(sm2sign->s) || BN_is_negative(sm2sign->s) || BN_ucmp(sm2sign->s, n) >= 0)
    {
        fprintf(stderr, "%s %s:%u - BN_is_zero or BN_is_negative or BN_ucmp failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    /* t = (r + s) mod n */
    if(!BN_mod_add(t, sm2sign->r, sm2sign->s, n, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - BN_mod_add failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(BN_is_zero(t))
    {
        fprintf(stderr, "%s %s:%u - BN_is_zero failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    /* s * G + t * P = (x1, y1) */
    if(!EC_POINT_mul(group, kPG, sm2sign->s, pubkey, t, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if (EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field)
    {
        if(!EC_POINT_get_affine_coordinates_GFp(group, kPG, x1, NULL, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_get_affine_coordinates_GFp failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
    }
    else
    { 
        if (!EC_POINT_get_affine_coordinates_GF2m(group, kPG, x1, NULL, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_get_affine_coordinates_GF2m failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
    }

    /* R = (e + x1) mod n */
    if(!BN_mod_add(tmp, e, x1, n, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - BN_mod_add failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(BN_ucmp(tmp, sm2sign->r) != 0)
    {
        fprintf(stderr, "%s %s:%u - BN_ucmp failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    BN_CTX_end(bn_ctx);
    if(kPG) EC_POINT_free(kPG);
    if(tmp) BN_free(tmp);
    if(x1) BN_free(x1);
    if(t) BN_free(t);
    if(e) BN_free(e);
    if(n) BN_free(n);
    if(bn_ctx) BN_CTX_free(bn_ctx);
    if(sm2sign) ECDSA_SIG_free(sm2sign);
    return 1;
ErrP:
    BN_CTX_end(bn_ctx);
    if(kPG) EC_POINT_free(kPG);
    if(tmp) BN_free(tmp);
    if(x1) BN_free(x1);
    if(t) BN_free(t);
    if(e) BN_free(e);
    if(n) BN_free(n);
    if(bn_ctx) BN_CTX_free(bn_ctx);
    if(sm2sign) ECDSA_SIG_free(sm2sign);
    return 0;
}
