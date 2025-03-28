/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CRYPTO_TESTS_CHECK_CONFIG_H__
#define __CRYPTO_TESTS_CHECK_CONFIG_H__

#ifdef MBEDTLS_PSA_CRYPTO_CONFIG_FILE
#include MBEDTLS_PSA_CRYPTO_CONFIG_FILE

#if defined(TFM_CRYPTO_TEST_ALG_CBC) && \
    (!defined(PSA_WANT_ALG_CBC_NO_PADDING) && !defined(PSA_WANT_ALG_CBC_PKCS7))
#error "TFM_CRYPTO_TEST_ALG_CBC enabled, but CBC mode is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_CCM) && !defined(PSA_WANT_ALG_CCM)
#error "TFM_CRYPTO_TEST_ALG_CCM enabled, but CCM mode is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_CFB) && !defined(PSA_WANT_ALG_CFB)
#error "TFM_CRYPTO_TEST_ALG_CFB enabled, but CFB mode is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_ECB) && !defined(PSA_WANT_ALG_ECB_NO_PADDING)
#error "TFM_CRYPTO_TEST_ALG_ECB enabled, but ECB mode is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_CTR) && !defined(PSA_WANT_ALG_CTR)
#error "TFM_CRYPTO_TEST_ALG_CTR enabled, but CTR mode is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_OFB) && !defined(PSA_WANT_ALG_OFB)
#error "TFM_CRYPTO_TEST_ALG_OFB enabled, but OFB mode is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_GCM) && !defined(PSA_WANT_ALG_GCM)
#error "TFM_CRYPTO_TEST_ALG_GCM enabled, but GCM mode is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_SHA_224) && !defined(PSA_WANT_ALG_SHA_224)
#error "TFM_CRYPTO_TEST_ALG_SHA_224 enabled, but SHA-224 algorithm is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_SHA_384) && !defined(PSA_WANT_ALG_SHA_384)
#error "TFM_CRYPTO_TEST_ALG_SHA_384 enabled, but SHA-384 algorithm is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_SHA_512) && !defined(PSA_WANT_ALG_SHA_512)
#error "TFM_CRYPTO_TEST_ALG_SHA_512 enabled, but SHA-512 algorithm is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_HKDF) && !defined(PSA_WANT_ALG_HKDF)
#error "TFM_CRYPTO_TEST_ALG_HKDF enabled, but HKDF key derivation algorithm is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ECDH) && !defined(PSA_WANT_ALG_ECDH)
#error "TFM_CRYPTO_TEST_ALG_ECDH enabled, but ECDH key agreement algorithm is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_CHACHA20) && \
    (!defined(PSA_WANT_KEY_TYPE_CHACHA20) || !defined(PSA_WANT_ALG_STREAM_CIPHER))
#error "TFM_CRYPTO_TEST_ALG_CHACHA20 enabled, but ChaCha20 stream cipher algorithm is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305) && \
    (!defined(PSA_WANT_ALG_CHACHA20_POLY1305) || !defined(PSA_WANT_KEY_TYPE_CHACHA20))
#error "TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305 enabled, but ChaCha20-Poly1305 AEAD algorithm is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_RSASSA_PSS_VERIFICATION) && \
    !defined(PSA_WANT_ALG_RSA_PSS)
#error "TFM_CRYPTO_TEST_ALG_RSASSA_PSS_VERIFICATION enabled, but RSA-PSS algorithm is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_DETERMINISTIC_ECDSA) && \
    !defined(PSA_WANT_ALG_DETERMINISTIC_ECDSA)
#error "TFM_CRYPTO_TEST_ALG_DETERMINISTIC_ECDSA enabled, but DETERMINSTIC_ECDSA algorithm is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_ALG_ECDSA) && \
    !defined(PSA_WANT_ALG_ECDSA)
#error "TFM_CRYPTO_TEST_ALG_ECDSA enabled, but ECDSA algorithm is not defined!"
#endif

#if defined(TFM_CRYPTO_TEST_WP_SECP384_R1) && \
    !(defined(PSA_WANT_ALG_ECDSA) && defined(PSA_WANT_ECC_SECP_R1_384))
#if !defined(PSA_WANT_ALG_ECDSA)
#error "TFM_CRYPTO_TEST_WP_SECP384_R1 requires PSA_WANT_ALG_ECDSA!"
#endif
#if !defined(PSA_WANT_ECC_SECP_R1_384)
#error "TFM_CRYPTO_TEST_WP_SECP384_R1 requires PSA_WANT_ECC_SECP_R1_384"
#endif
#endif /* TFM_CRYPTO_TEST_WP_SECP384_R1 && !(PSA_WANT_ALG_ECDSA && PSA_WANT_ECC_SECP_R1_384) */

#if defined(PSA_WANT_ALG_SHA_1)
#undef TFM_CRYPTO_TEST_UNSUPPORTED_ALG
#warning "SHA_1 algorithm is defined, disable TFM_CRYPTO_TEST_UNSUPPORTED_ALG!"
#endif

#endif /* MBEDTLS_PSA_CRYPTO_CONFIG_FILE */

#endif /* __CRYPTO_TESTS_CHECK_CONFIG_H__ */
