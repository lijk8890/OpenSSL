#include "sm2.h"
#include "../../ssl/ssl_locl.h"

int sm2_compute_key(                                                    \
    const EC_GROUP *group, const char *id, int id_len,                  \
    unsigned char *self_tmp_prvkey, int self_tmp_prvkey_len,            \
    unsigned char *self_tmp_pubkey, int self_tmp_pubkey_len,            \
    unsigned char *self_enc_prvkey, int self_enc_prvkey_len,            \
    unsigned char *self_enc_pubkey, int self_enc_pubkey_len,            \
    unsigned char *peer_tmp_pubkey, int peer_tmp_pubkey_len,            \
    unsigned char *peer_enc_pubkey, int peer_enc_pubkey_len,            \
    unsigned char *session_key, int session_key_len,                    \
    int is_server                                                       \
    )
{
    int w = 0;
    BN_CTX *bn_ctx = BN_CTX_new();
    BIGNUM *order = BN_new();
    BIGNUM *two_pow_w = BN_new();
    BIGNUM *x1bar = BN_new();
    BIGNUM *x2bar = BN_new();
    BIGNUM *t = BN_new();
    BIGNUM *h = BN_new();
    BIGNUM *self_tmp_bn = BN_new();                         //本端临时私钥 rA or rB
    BIGNUM *self_enc_bn = BN_new();                         //本端加密证书私钥 dA or dB
    EC_POINT *self_tmp_point = EC_POINT_new(group);         //本端临时公钥 RA or RB - x1bar
    EC_POINT *peer_tmp_point = EC_POINT_new(group);         //对端临时公钥 RB or RA - x2bar
    EC_POINT *peer_enc_point = EC_POINT_new(group);         //对端加密证书公钥 PB or PA
    EC_POINT *point = EC_POINT_new(group);                  //目标椭圆曲线点 U or V
    unsigned char pxyzab[128+1] = {0};

    if(bn_ctx == NULL || order == NULL || two_pow_w == NULL || x1bar == NULL || x2bar == NULL || t == NULL || h == NULL \
        || self_tmp_bn == NULL || self_enc_bn == NULL || self_tmp_point == NULL || peer_tmp_point == NULL || peer_enc_point == NULL || point == NULL)
    {
        fprintf(stderr, "%s %s:%u - sm2_compute_key failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    BN_CTX_start(bn_ctx);

    if(!BN_bin2bn(self_tmp_prvkey, self_tmp_prvkey_len, self_tmp_bn))
    {
        fprintf(stderr, "%s %s:%u - BN_bin2bn failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    } 
    if(!BN_bin2bn(self_enc_prvkey, self_enc_prvkey_len, self_enc_bn))
    {
        fprintf(stderr, "%s %s:%u - BN_bin2bn failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    if(!EC_POINT_oct2point(group, self_tmp_point, self_tmp_pubkey, self_tmp_pubkey_len, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_oct2point failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(!EC_POINT_oct2point(group, peer_tmp_point, peer_tmp_pubkey, peer_tmp_pubkey_len, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_oct2point failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(!EC_POINT_oct2point(group, peer_enc_point, peer_enc_pubkey, peer_enc_pubkey_len, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_oct2point failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    if(!EC_POINT_is_on_curve(group, peer_tmp_point, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_is_on_curve failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    if(!EC_GROUP_get_order(group, order, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_GROUP_get_order failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(!EC_GROUP_get_cofactor(group, h, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_GROUP_get_cofactor failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    /*
     * w = ceil(keybits / 2) - 1
     * x = 2^w + (x and (2^w - 1)) = 2^w + (x mod 2^w)
     * t = (d + x * r) mod n
     * t = (h * t) mod n
     */
    w = (BN_num_bits(order) + 1) / 2 - 1;                   //向上取整 w = 127
    if(!BN_lshift(two_pow_w, BN_value_one(), w))            //two_pow_w = 2^w
    {
        fprintf(stderr, "%s %s:%u - BN_lshift failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    if(EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field)
    {
        if(!EC_POINT_get_affine_coordinates_GFp(group, self_tmp_point, x1bar, NULL, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_get_affine_coordinates_GFp failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
        if(!EC_POINT_get_affine_coordinates_GFp(group, peer_tmp_point, x2bar, NULL, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_get_affine_coordinates_GFp failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
    }
    else
    {
        if(!EC_POINT_get_affine_coordinates_GF2m(group, self_tmp_point, x1bar, NULL, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_get_affine_coordinates_GF2m failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
        if(!EC_POINT_get_affine_coordinates_GF2m(group, peer_tmp_point, x2bar, NULL, bn_ctx))
        {
            fprintf(stderr, "%s %s:%u - EC_POINT_get_affine_coordinates_GF2m failed\n", __FUNCTION__, __FILE__, __LINE__);
            goto ErrP;
        }
    }

    //Caculate self x1bar
    if(!BN_nnmod(x1bar, x1bar, two_pow_w, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - BN_nnmod failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(!BN_add(x1bar, x1bar, two_pow_w))
    {
        fprintf(stderr, "%s %s:%u - BN_nnmod failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    //Caculate peer x2bar
    if(!BN_nnmod(x2bar, x2bar, two_pow_w, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - BN_nnmod failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(!BN_add(x2bar, x2bar, two_pow_w))
    {
        fprintf(stderr, "%s %s:%u - BN_nnmod failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    //Caculate t
    //t = (d + xbar * r) mod n
    if(!BN_mod_mul(t, x1bar, self_tmp_bn, order, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - BN_mod_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(!BN_mod_add(t, t, self_enc_bn, order, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - BN_mod_add failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    //Caculate U or V
    //[h * t](P + xbar * R) = (x, y)
    if(!BN_mul(t, t, h, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - BN_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(!EC_POINT_mul(group, point, NULL, peer_tmp_point, x2bar, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(!EC_POINT_add(group, point, point, peer_enc_point, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_add failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(!EC_POINT_mul(group, point, NULL, point, t, bn_ctx))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_mul failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    if(EC_POINT_is_at_infinity(group, point))
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_is_at_infinity failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }
    if(EC_POINT_point2oct(group, point, POINT_CONVERSION_UNCOMPRESSED, &pxyzab[0], 65, bn_ctx) != 65)
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_point2oct failed\n", __FUNCTION__, __FILE__, __LINE__);
        goto ErrP;
    }

    if(is_server)
    {
        //Caculate ZA
        get_z(id, id_len, self_enc_pubkey, self_enc_pubkey_len, &pxyzab[65]);
        //Caculate ZB
        get_z(id, id_len, peer_enc_pubkey, peer_enc_pubkey_len, &pxyzab[97]);
    }
    else
    {
        //Caculate ZA
        get_z(id, id_len, peer_enc_pubkey, peer_enc_pubkey_len, &pxyzab[65]);
        //Caculate ZB
        get_z(id, id_len, self_enc_pubkey, self_enc_pubkey_len, &pxyzab[97]);
    }
    kdf_sm3(&pxyzab[1], 128, session_key, session_key_len);

    BN_CTX_end(bn_ctx);
    if(point) EC_POINT_free(point);
    if(peer_enc_point) EC_POINT_free(peer_enc_point);
    if(peer_tmp_point) EC_POINT_free(peer_tmp_point);
    if(self_tmp_point) EC_POINT_free(self_tmp_point);
    if(self_enc_bn) BN_free(self_enc_bn);
    if(self_tmp_bn) BN_free(self_tmp_bn);
    if(h) BN_free(h);
    if(t) BN_free(t);
    if(x2bar) BN_free(x2bar);
    if(x1bar) BN_free(x1bar);
    if(two_pow_w) BN_free(two_pow_w);
    if(order) BN_free(order);
    if(bn_ctx) BN_CTX_free(bn_ctx);
    return session_key_len;
ErrP:
    BN_CTX_end(bn_ctx);
    if(point) EC_POINT_free(point);
    if(peer_enc_point) EC_POINT_free(peer_enc_point);
    if(peer_tmp_point) EC_POINT_free(peer_tmp_point);
    if(self_tmp_point) EC_POINT_free(self_tmp_point);
    if(self_enc_bn) BN_free(self_enc_bn);
    if(self_tmp_bn) BN_free(self_tmp_bn);
    if(h) BN_free(h);
    if(t) BN_free(t);
    if(x2bar) BN_free(x2bar);
    if(x1bar) BN_free(x1bar);
    if(two_pow_w) BN_free(two_pow_w);
    if(order) BN_free(order);
    if(bn_ctx) BN_CTX_free(bn_ctx);
    return 0;
}

int SM2_compute_key(SSL *s, const EC_KEY *ecdh, const EC_POINT *point, unsigned char *out, int outlen)
{
    int ret = 0;
    int self_tmp_prvkey_len = 0;                            //本端临时私钥长度

    unsigned char self_tmp_prvkey[32] = {0};                //本端临时私钥
    unsigned char self_tmp_pubkey[65] = {0};                //本端临时公钥

    unsigned char self_enc_prvkey[32] = {0};                //本端加密证书私钥
    unsigned char self_enc_pubkey[65] = {0};                //本端加密证书公钥

    unsigned char peer_tmp_pubkey[65] = {0};                //对端临时公钥
    unsigned char peer_enc_pubkey[65] = {0};                //对端加密证书公钥

    const char *id = "1234567812345678";
    const EC_GROUP *group = EC_KEY_get0_group(ecdh);

    if(s->cert->pkeys[SSL_PKEY_ECC_ENC].x509 == NULL || s->cert->pkeys[SSL_PKEY_ECC_ENC].x509->cert_info->key->public_key->length != 65 \
        || s->session->peer == NULL || s->session->peer->cert_info->key->public_key->length != 65)
    {
        fprintf(stderr, "%s %s:%u - SM2_compute_key failed\n", __FUNCTION__, __FILE__, __LINE__);
        return 0;
    }

    self_tmp_prvkey_len = BN_bn2bin(EC_KEY_get0_private_key(ecdh), self_tmp_prvkey);
    if(self_tmp_prvkey_len <= 0)
    {
        fprintf(stderr, "%s %s:%u - BN_bn2bin failed\n", __FUNCTION__, __FILE__, __LINE__);
        return 0;
    }
    ret = EC_POINT_point2oct(group, EC_KEY_get0_public_key(ecdh), POINT_CONVERSION_UNCOMPRESSED, self_tmp_pubkey, 65, NULL);
    if(ret != 65)
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_point2oct failed\n", __FUNCTION__, __FILE__, __LINE__);
        return 0;
    }

    ret = BN_bn2bin(EC_KEY_get0_private_key(s->cert->pkeys[SSL_PKEY_ECC_ENC].privatekey->pkey.ec), self_enc_prvkey);
    if(ret != 32)
    {
        fprintf(stderr, "%s %s:%u - BN_bn2bin failed\n", __FUNCTION__, __FILE__, __LINE__);
        return 0;
    }
    memcpy(self_enc_pubkey, s->cert->pkeys[SSL_PKEY_ECC_ENC].x509->cert_info->key->public_key->data, 65);

    ret = EC_POINT_point2oct(group, point, POINT_CONVERSION_UNCOMPRESSED, peer_tmp_pubkey, 65, NULL);
    if(ret != 65)
    {
        fprintf(stderr, "%s %s:%u - EC_POINT_point2oct failed\n", __FUNCTION__, __FILE__, __LINE__);
        return 0;
    }
    memcpy(peer_enc_pubkey, s->session->peer->cert_info->key->public_key->data, 65);

    return sm2_compute_key(group, id, 16, self_tmp_prvkey, self_tmp_prvkey_len, self_tmp_pubkey, 65, \
        self_enc_prvkey, 32, self_enc_pubkey, 65, peer_tmp_pubkey, 65, peer_enc_pubkey, 65, out, outlen, s->server);
}
