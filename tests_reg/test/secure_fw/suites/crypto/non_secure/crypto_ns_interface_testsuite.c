/*
 * Copyright (c) 2018-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "config_tfm.h"
#include "test_framework_helpers.h"
#include "../crypto_tests_common.h"

/* List of tests */
static void tfm_crypto_test_1001(struct test_result_t *ret);
#ifdef TFM_CRYPTO_TEST_ALG_CBC
static void tfm_crypto_test_1002(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_CBC */
#ifdef TFM_CRYPTO_TEST_ALG_CFB
static void tfm_crypto_test_1003(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_CFB */
#ifdef TFM_CRYPTO_TEST_ALG_CTR
static void tfm_crypto_test_1005(struct test_result_t *ret);
static void tfm_crypto_test_1006(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_CTR */
static void tfm_crypto_test_1007(struct test_result_t *ret);
static void tfm_crypto_test_1008(struct test_result_t *ret);
#ifdef TFM_CRYPTO_TEST_ALG_CFB
static void tfm_crypto_test_1009(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_CFB */
#ifdef TFM_CRYPTO_TEST_UNSUPPORTED_ALG
static void tfm_crypto_test_1010(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_UNSUPPORTED_ALG */
#if defined(TFM_CRYPTO_TEST_ALG_SHA_224) && !defined(CC3XX_RUNTIME_ENABLED)
static void tfm_crypto_test_1011(struct test_result_t *ret);
#endif
static void tfm_crypto_test_1012(struct test_result_t *ret);
#ifdef TFM_CRYPTO_TEST_ALG_SHA_384
static void tfm_crypto_test_1013(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_SHA_384 */
#ifdef TFM_CRYPTO_TEST_ALG_SHA_512
static void tfm_crypto_test_1014(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_SHA_512 */
#ifdef TFM_CRYPTO_TEST_UNSUPPORTED_ALG
static void tfm_crypto_test_1019(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_UNSUPPORTED_ALG */
static void tfm_crypto_test_1020(struct test_result_t *ret);
#ifdef TFM_CRYPTO_TEST_ALG_SHA_384
static void tfm_crypto_test_1021(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_SHA_384 */
#ifdef TFM_CRYPTO_TEST_ALG_SHA_512
static void tfm_crypto_test_1022(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_SHA_512 */
#if defined(TFM_CRYPTO_TEST_ALG_SHA_224) && !defined(CC3XX_RUNTIME_ENABLED)
static void tfm_crypto_test_1024(struct test_result_t *ret);
#endif
#ifdef TFM_CRYPTO_TEST_ALG_CCM
static void tfm_crypto_test_1030(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_CCM */
#ifdef TFM_CRYPTO_TEST_ALG_GCM
static void tfm_crypto_test_1031(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_GCM */
static void tfm_crypto_test_1032(struct test_result_t *ret);
static void tfm_crypto_test_1033(struct test_result_t *ret);
#ifdef TFM_INTERNAL_TRUSTED_STORAGE_SERVICE
static void tfm_crypto_test_1034(struct test_result_t *ret);
#endif /* TFM_INTERNAL_TRUSTED_STORAGE_SERVICE */
#ifdef TFM_CRYPTO_TEST_ALG_CCM
static void tfm_crypto_test_1035(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_CCM */
static void tfm_crypto_test_1036(struct test_result_t *ret);
static void tfm_crypto_test_1037(struct test_result_t *ret);
#ifdef TFM_CRYPTO_TEST_HKDF
static void tfm_crypto_test_1038(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_HKDF */
#ifdef TFM_CRYPTO_TEST_ECDH
static void tfm_crypto_test_1039(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ECDH */
#if defined(TFM_CRYPTO_TEST_ALG_OFB) && !defined(CC3XX_RUNTIME_ENABLED)
static void tfm_crypto_test_1040(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_OFB && !CC3XX_RUNTIME_ENABLED */
#ifdef TFM_CRYPTO_TEST_ALG_ECB
static void tfm_crypto_test_1041(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_ECB */
#if CRYPTO_ASYM_ENCRYPT_MODULE_ENABLED
static void tfm_crypto_test_1042(struct test_result_t *ret);
static void tfm_crypto_test_1043(struct test_result_t *ret);
#endif /* CRYPTO_ASYM_ENCRYPT_MODULE_ENABLED */
#if CRYPTO_ASYM_SIGN_MODULE_ENABLED
#if TFM_CRYPTO_TEST_ALG_DETERMINISTIC_ECDSA
static void tfm_crypto_test_1044(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_DETERMINISTIC_ECDSA */
#if TFM_CRYPTO_TEST_ALG_ECDSA
static void tfm_crypto_test_1053(struct test_result_t *ret);
static void tfm_crypto_test_1056(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_ECDSA */
#endif /* CRYPTO_ASYM_SIGN_MODULE_ENABLED */
#ifdef TFM_CRYPTO_TEST_ALG_CBC
static void tfm_crypto_test_1045(struct test_result_t *ret);
static void tfm_crypto_test_1046(struct test_result_t *ret);
static void tfm_crypto_test_1047(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_CBC */
#ifdef TFM_CRYPTO_TEST_CHACHA20
static void tfm_crypto_test_1048(struct test_result_t *ret);
static void tfm_crypto_test_1051(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_CHACHA20 */
#ifdef TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305
static void tfm_crypto_test_1049(struct test_result_t *ret);
static void tfm_crypto_test_1052(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305 */
#if defined(TFM_CRYPTO_TEST_ALG_RSASSA_PSS_VERIFICATION) && !defined(CC3XX_RUNTIME_ENABLED)
static void tfm_crypto_test_1050(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_RSASSA_PSS_VERIFICATION */
#if defined(TFM_CRYPTO_TEST_SINGLE_PART_FUNCS)
#if defined(TFM_CRYPTO_TEST_ALG_GCM) || defined(TFM_CRYPTO_TEST_ALG_CCM)
static void tfm_crypto_test_1054(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_ALG_GCM || TFM_CRYPTO_TEST_ALG_CCM */
#endif /* TFM_CRYPTO_TEST_SINGLE_PART_FUNCS */
#if defined(TFM_CRYPTO_TEST_WP_SECP384_R1)
static void tfm_crypto_test_1055(struct test_result_t *ret);
#endif /* TFM_CRYPTO_TEST_WP_SECP384_R1 */

static struct test_t crypto_tests[] = {
    {&tfm_crypto_test_1001, "TFM_NS_CRYPTO_TEST_1001",
     "Non Secure Key management interface"},
#ifdef TFM_CRYPTO_TEST_ALG_CBC
    {&tfm_crypto_test_1002, "TFM_NS_CRYPTO_TEST_1002",
     "Non Secure Symmetric encryption (AES-128-CBC) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_CBC */
#ifdef TFM_CRYPTO_TEST_ALG_CFB
    {&tfm_crypto_test_1003, "TFM_NS_CRYPTO_TEST_1003",
     "Non Secure Symmetric encryption (AES-128-CFB) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_CFB */
#ifdef TFM_CRYPTO_TEST_ALG_CTR
    {&tfm_crypto_test_1005, "TFM_NS_CRYPTO_TEST_1005",
     "Non Secure Symmetric encryption (AES-128-CTR) interface"},
    {&tfm_crypto_test_1006, "TFM_NS_CRYPTO_TEST_1006",
     "Non Secure Symmetric encryption (AES-256-CTR) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_CTR */
    {&tfm_crypto_test_1007, "TFM_NS_CRYPTO_TEST_1007",
     "Non Secure Symmetric encryption invalid cipher"},
    {&tfm_crypto_test_1008, "TFM_NS_CRYPTO_TEST_1008",
     "Non Secure Symmetric encryption invalid cipher (AES-152)"},
#ifdef TFM_CRYPTO_TEST_ALG_CFB
    {&tfm_crypto_test_1009, "TFM_NS_CRYPTO_TEST_1009",
     "Non Secure Symmetric encryption invalid cipher (HMAC-128-CFB)"},
#endif /* TFM_CRYPTO_TEST_ALG_CFB */
#ifdef TFM_CRYPTO_TEST_UNSUPPORTED_ALG
    {&tfm_crypto_test_1010, "TFM_NS_CRYPTO_TEST_1010",
     "Non Secure Unsupported Hash (SHA-1) interface"},
#endif /* TFM_CRYPTO_TEST_UNSUPPORTED_ALG */
#if defined(TFM_CRYPTO_TEST_ALG_SHA_224) && !defined(CC3XX_RUNTIME_ENABLED)
    {&tfm_crypto_test_1011, "TFM_NS_CRYPTO_TEST_1011",
     "Non Secure Hash (SHA-224) interface"},
#endif
    {&tfm_crypto_test_1012, "TFM_NS_CRYPTO_TEST_1012",
     "Non Secure Hash (SHA-256) interface"},
#ifdef TFM_CRYPTO_TEST_ALG_SHA_384
    {&tfm_crypto_test_1013, "TFM_NS_CRYPTO_TEST_1013",
     "Non Secure Hash (SHA-384) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_SHA_384 */
#ifdef TFM_CRYPTO_TEST_ALG_SHA_512
    {&tfm_crypto_test_1014, "TFM_NS_CRYPTO_TEST_1014",
     "Non Secure Hash (SHA-512) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_SHA_512 */
#ifdef TFM_CRYPTO_TEST_UNSUPPORTED_ALG
    {&tfm_crypto_test_1019, "TFM_NS_CRYPTO_TEST_1019",
     "Non Secure Unsupported HMAC (SHA-1) interface"},
#endif /* TFM_CRYPTO_TEST_UNSUPPORTED_ALG */
    {&tfm_crypto_test_1020, "TFM_NS_CRYPTO_TEST_1020",
     "Non Secure HMAC (SHA-256) interface"},
#ifdef TFM_CRYPTO_TEST_ALG_SHA_384
    {&tfm_crypto_test_1021, "TFM_NS_CRYPTO_TEST_1021",
     "Non Secure HMAC (SHA-384) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_SHA_384 */
#ifdef TFM_CRYPTO_TEST_ALG_SHA_512
    {&tfm_crypto_test_1022, "TFM_NS_CRYPTO_TEST_1022",
     "Non Secure HMAC (SHA-512) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_SHA_512 */
#if defined(TFM_CRYPTO_TEST_ALG_SHA_224) && !defined(CC3XX_RUNTIME_ENABLED)
    {&tfm_crypto_test_1024, "TFM_NS_CRYPTO_TEST_1024",
     "Non Secure HMAC with long key (SHA-224) interface"},
#endif
#ifdef TFM_CRYPTO_TEST_ALG_CCM
    {&tfm_crypto_test_1030, "TFM_NS_CRYPTO_TEST_1030",
     "Non Secure AEAD (AES-128-CCM) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_CCM */
#ifdef TFM_CRYPTO_TEST_ALG_GCM
    {&tfm_crypto_test_1031, "TFM_NS_CRYPTO_TEST_1031",
     "Non Secure AEAD (AES-128-GCM) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_GCM */
    {&tfm_crypto_test_1032, "TFM_NS_CRYPTO_TEST_1032",
     "Non Secure key policy interface"},
    {&tfm_crypto_test_1033, "TFM_NS_CRYPTO_TEST_1033",
     "Non Secure key policy check permissions"},
#ifdef TFM_INTERNAL_TRUSTED_STORAGE_SERVICE
    {&tfm_crypto_test_1034, "TFM_NS_CRYPTO_TEST_1034",
     "Non Secure persistent key interface"},
#endif /* TFM_INTERNAL_TRUSTED_STORAGE_SERVICE */
#ifdef TFM_CRYPTO_TEST_ALG_CCM
    {&tfm_crypto_test_1035, "TFM_NS_CRYPTO_TEST_1035",
     "Non Secure AEAD interface with truncated auth tag (AES-128-CCM-8)"},
#endif /* TFM_CRYPTO_TEST_ALG_CCM */
    {&tfm_crypto_test_1036, "TFM_NS_CRYPTO_TEST_1036",
     "Non Secure TLS 1.2 PRF key derivation"},
    {&tfm_crypto_test_1037, "TFM_NS_CRYPTO_TEST_1037",
     "Non Secure TLS-1.2 PSK-to-MasterSecret key derivation"},
#ifdef TFM_CRYPTO_TEST_HKDF
    {&tfm_crypto_test_1038, "TFM_NS_CRYPTO_TEST_1038",
     "Non Secure HKDF key derivation"},
#endif /* TFM_CRYPTO_TEST_HKDF */
#ifdef TFM_CRYPTO_TEST_ECDH
    {&tfm_crypto_test_1039, "TFM_NS_CRYPTO_TEST_1039",
     "Non Secure ECDH key agreement"},
#endif /* TFM_CRYPTO_TEST_ECDH */
#if defined(TFM_CRYPTO_TEST_ALG_OFB) && !defined(CC3XX_RUNTIME_ENABLED)
    {&tfm_crypto_test_1040, "TFM_NS_CRYPTO_TEST_1040",
     "Non Secure Symmetric encryption (AES-128-OFB) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_OFB && !CC3XX_RUNTIME_ENABLED */
#ifdef TFM_CRYPTO_TEST_ALG_ECB
    {&tfm_crypto_test_1041, "TFM_NS_CRYPTO_TEST_1041",
     "Non Secure Symmetric encryption (AES-128-ECB) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_ECB */
#if CRYPTO_ASYM_ENCRYPT_MODULE_ENABLED
    {&tfm_crypto_test_1042, "TFM_NS_CRYPTO_TEST_1042",
     "Non Secure Asymmetric encryption interface (RSA-OAEP)"},
    {&tfm_crypto_test_1043, "TFM_NS_CRYPTO_TEST_1043",
     "Non Secure Asymmetric encryption interface (RSA-PKCS1V15)"},
#endif /* CRYPTO_ASYM_ENCRYPT_MODULE_ENABLED */
#if CRYPTO_ASYM_SIGN_MODULE_ENABLED
#if TFM_CRYPTO_TEST_ALG_DETERMINISTIC_ECDSA
    {&tfm_crypto_test_1044, "TFM_NS_CRYPTO_TEST_1044",
     "Non Secure Sign and verify message interface (DETERMINISTIC_ECDSA-SECP256R1-SHA256)"},
#endif /* TFM_CRYPTO_TEST_ALG_DETERMINISTIC_ECDSA */
#if TFM_CRYPTO_TEST_ALG_ECDSA
    {&tfm_crypto_test_1053, "TFM_NS_CRYPTO_TEST_1053",
     "Non Secure Sign and verify hash interface (ECDSA-SECP256R1-SHA256)"},
    {&tfm_crypto_test_1056, "TFM_NS_CRYPTO_TEST_1056",
     "Non Secure Sign and verify hash interface (ECDSA-SECP384R1-SHA384)"},
#endif /* TFM_CRYPTO_TEST_ALG_ECDSA */
#endif /* CRYPTO_ASYM_SIGN_MODULE_ENABLED */
#ifdef TFM_CRYPTO_TEST_ALG_CBC
    {&tfm_crypto_test_1045, "TFM_NS_CRYPTO_TEST_1045",
     "Non Secure Symmetric encryption (AES-128-CBC-PKCS7) interface"},
    {&tfm_crypto_test_1046, "TFM_NS_CRYPTO_TEST_1046",
     "Non Secure Symmetric encryption (AES-128-CBC-PKCS7) interface, shorter"},
    {&tfm_crypto_test_1047, "TFM_NS_CRYPTO_TEST_1047",
     "Non Secure Symmetric encryption (AES-128-CBC-PKCS7) interface, longer"},
#endif /* TFM_CRYPTO_TEST_ALG_CBC */
#ifdef TFM_CRYPTO_TEST_CHACHA20
    {&tfm_crypto_test_1048, "TFM_NS_CRYPTO_TEST_1048",
     "Non Secure Symmetric encryption (CHACHA20) interface"},
#endif /* TFM_CRYPTO_TEST_CHACHA20 */
#ifdef TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305
    {&tfm_crypto_test_1049, "TFM_NS_CRYPTO_TEST_1049",
     "Non Secure AEAD (CHACHA20-POLY1305) interface"},
#endif /* TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305 */
#if defined(TFM_CRYPTO_TEST_ALG_RSASSA_PSS_VERIFICATION) && !defined(CC3XX_RUNTIME_ENABLED)
    {&tfm_crypto_test_1050, "TFM_NS_CRYPTO_TEST_1050",
     "Non Secure RSASSA-PSS signature verification (RSASSA-PSS-SHA256)"},
#endif /* TFM_CRYPTO_TEST_ALG_RSASSA_PSS_VERIFICATION */
#ifdef TFM_CRYPTO_TEST_CHACHA20
    {&tfm_crypto_test_1051, "TFM_NS_CRYPTO_TEST_1051",
     "Non Secure RFC7539 verification on Chacha20"},
#endif /* TFM_CRYPTO_TEST_CHACHA20 */
#ifdef TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305
    {&tfm_crypto_test_1052, "TFM_NS_CRYPTO_TEST_1052",
     "Non Secure RFC7539 verification on Chacha20-Poly1305"},
#endif /* TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305 */
#if defined(TFM_CRYPTO_TEST_SINGLE_PART_FUNCS)
#if defined(TFM_CRYPTO_TEST_ALG_GCM) || defined(TFM_CRYPTO_TEST_ALG_CCM)
    {&tfm_crypto_test_1054, "TFM_NS_CRYPTO_TEST_1054",
     "Non Secure authenticator based on AEAD"},
#endif /* TFM_CRYPTO_TEST_ALG_GCM || TFM_CRYPTO_TEST_ALG_CCM */
#endif /* TFM_CRYPTO_TEST_SINGLE_PART_FUNCS */
#if defined(TFM_CRYPTO_TEST_WP_SECP384_R1)
    {&tfm_crypto_test_1055, "TFM_NS_CRYPTO_TEST_1055",
     "Non Secure SECP384R1 ECDSA Verification Wycheproof tests"},
#endif /* TFM_CRYPTO_TEST_WP_SECP384_R1 */
};

void register_testsuite_ns_crypto_interface(struct test_suite_t *p_test_suite)
{
    uint32_t list_size = (sizeof(crypto_tests) / sizeof(crypto_tests[0]));

    set_testsuite("Crypto non-secure interface test (TFM_NS_CRYPTO_TEST_1XXX)",
                  crypto_tests, list_size, p_test_suite);
}

/**
 * \brief Non-Secure interface test for Crypto
 *
 * \details The scope of this set of tests is to functionally verify
 *          the interfaces specified by psa/crypto.h are working
 *          as expected. This is not meant to cover all possible
 *          scenarios and corner cases.
 *
 */
static void tfm_crypto_test_1001(struct test_result_t *ret)
{
    psa_key_interface_test(PSA_KEY_TYPE_AES, ret);
}

#ifdef TFM_CRYPTO_TEST_ALG_CBC
static void tfm_crypto_test_1002(struct test_result_t *ret)
{
    psa_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_CBC_NO_PADDING,
                    test_key_128, BIT_SIZE_TEST_KEY, ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_CBC */

#ifdef TFM_CRYPTO_TEST_ALG_CFB
static void tfm_crypto_test_1003(struct test_result_t *ret)
{
    psa_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_CFB,
                    test_key_128, BIT_SIZE_TEST_KEY, ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_CFB */

#ifdef TFM_CRYPTO_TEST_ALG_CTR
static void tfm_crypto_test_1005(struct test_result_t *ret)
{
    psa_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_CTR,
                    test_key_128, BIT_SIZE_TEST_KEY, ret);
}

static void tfm_crypto_test_1006(struct test_result_t *ret)
{
    psa_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_CTR,
                    test_key_256, BIT_SIZE_TEST_LONG_KEY, ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_CTR */

static void tfm_crypto_test_1007(struct test_result_t *ret)
{
    psa_invalid_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_HMAC(PSA_ALG_SHA_256),
                            16, ret);
}

static void tfm_crypto_test_1008(struct test_result_t *ret)
{
    psa_invalid_key_length_test(ret);
}

#ifdef TFM_CRYPTO_TEST_ALG_CFB
static void tfm_crypto_test_1009(struct test_result_t *ret)
{
    /* HMAC is not a block cipher */
    psa_invalid_cipher_test(PSA_KEY_TYPE_HMAC, PSA_ALG_CFB, 16, ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_CFB */

#ifdef TFM_CRYPTO_TEST_UNSUPPORTED_ALG
static void tfm_crypto_test_1010(struct test_result_t *ret)
{
    psa_unsupported_hash_test(PSA_ALG_SHA_1, ret);
}
#endif /* TFM_CRYPTO_TEST_UNSUPPORTED_ALG */

#if defined(TFM_CRYPTO_TEST_ALG_SHA_224) && !defined(CC3XX_RUNTIME_ENABLED)
static void tfm_crypto_test_1011(struct test_result_t *ret)
{
    psa_hash_test(PSA_ALG_SHA_224, ret);
}
#endif

static void tfm_crypto_test_1012(struct test_result_t *ret)
{
    psa_hash_test(PSA_ALG_SHA_256, ret);
}

#ifdef TFM_CRYPTO_TEST_ALG_SHA_384
static void tfm_crypto_test_1013(struct test_result_t *ret)
{
    psa_hash_test(PSA_ALG_SHA_384, ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_SHA_384 */

#ifdef TFM_CRYPTO_TEST_ALG_SHA_512
static void tfm_crypto_test_1014(struct test_result_t *ret)
{
    psa_hash_test(PSA_ALG_SHA_512, ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_SHA_512 */

#ifdef TFM_CRYPTO_TEST_UNSUPPORTED_ALG
static void tfm_crypto_test_1019(struct test_result_t *ret)
{
    psa_unsupported_mac_test(PSA_KEY_TYPE_HMAC, PSA_ALG_HMAC(PSA_ALG_SHA_1),
                             ret);
}
#endif /* TFM_CRYPTO_TEST_UNSUPPORTED_ALG */

static void tfm_crypto_test_1020(struct test_result_t *ret)
{
    psa_mac_test(PSA_ALG_HMAC(PSA_ALG_SHA_256),
                 test_key_128, BIT_SIZE_TEST_KEY, ret);
}

#ifdef TFM_CRYPTO_TEST_ALG_SHA_384
static void tfm_crypto_test_1021(struct test_result_t *ret)
{
    psa_mac_test(PSA_ALG_HMAC(PSA_ALG_SHA_384),
                 test_key_128, BIT_SIZE_TEST_KEY, ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_SHA_384 */

#ifdef TFM_CRYPTO_TEST_ALG_SHA_512
static void tfm_crypto_test_1022(struct test_result_t *ret)
{
    psa_mac_test(PSA_ALG_HMAC(PSA_ALG_SHA_512),
                 test_key_128, BIT_SIZE_TEST_KEY, ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_SHA_512 */

#if defined(TFM_CRYPTO_TEST_ALG_SHA_224) && !defined(CC3XX_RUNTIME_ENABLED)
static void tfm_crypto_test_1024(struct test_result_t *ret)
{
    psa_mac_test(PSA_ALG_HMAC(PSA_ALG_SHA_224),
                 test_key_256, BIT_SIZE_TEST_LONG_KEY, ret);
}
#endif

#ifdef TFM_CRYPTO_TEST_ALG_CCM
static void tfm_crypto_test_1030(struct test_result_t *ret)
{
    psa_aead_test(PSA_KEY_TYPE_AES, PSA_ALG_CCM,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK,
                  PSA_TEST_IOVEC_NOT_INLINED,
                  ret);

#ifdef CC3XX_RUNTIME_ENABLED
    psa_aead_test(PSA_KEY_TYPE_AES, PSA_ALG_CCM,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK / 3,
                  PSA_TEST_IOVEC_NOT_INLINED,
                  ret);

    psa_aead_test(PSA_KEY_TYPE_AES, PSA_ALG_CCM,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK / 4,
                  PSA_TEST_IOVEC_NOT_INLINED,
                  ret);
#endif /* CC3XX_RUNTIME_ENABLED */

    psa_aead_test(PSA_KEY_TYPE_AES, PSA_ALG_CCM,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK,
                  PSA_TEST_IOVEC_INLINED,
                  ret);

#ifdef CC3XX_RUNTIME_ENABLED
    psa_aead_test(PSA_KEY_TYPE_AES, PSA_ALG_CCM,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK / 3,
                  PSA_TEST_IOVEC_INLINED,
                  ret);

    psa_aead_test(PSA_KEY_TYPE_AES, PSA_ALG_CCM,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK / 4,
                  PSA_TEST_IOVEC_INLINED,
                  ret);
#endif /* CC3XX_RUNTIME_ENABLED */
}
#endif /* TFM_CRYPTO_TEST_ALG_CCM */

#ifdef TFM_CRYPTO_TEST_ALG_GCM
static void tfm_crypto_test_1031(struct test_result_t *ret)
{
    psa_aead_test(PSA_KEY_TYPE_AES, PSA_ALG_GCM,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK,
                  PSA_TEST_IOVEC_NOT_INLINED,
                  ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_GCM */

static void tfm_crypto_test_1032(struct test_result_t *ret)
{
    psa_policy_key_interface_test(ret);
}

static void tfm_crypto_test_1033(struct test_result_t *ret)
{
    psa_policy_invalid_policy_usage_test(ret);
}

#ifdef TFM_INTERNAL_TRUSTED_STORAGE_SERVICE
static void tfm_crypto_test_1034(struct test_result_t *ret)
{
    psa_persistent_key_test(1, ret);
}
#endif /* TFM_INTERNAL_TRUSTED_STORAGE_SERVICE */

#ifdef TFM_CRYPTO_TEST_ALG_CCM
static void tfm_crypto_test_1035(struct test_result_t *ret)
{
    psa_algorithm_t alg = PSA_ALG_AEAD_WITH_SHORTENED_TAG(
            PSA_ALG_CCM, TRUNCATED_AUTH_TAG_LEN);

    psa_aead_test(PSA_KEY_TYPE_AES, alg,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK,
                  PSA_TEST_IOVEC_NOT_INLINED,
                  ret);

#ifdef CC3XX_RUNTIME_ENABLED
    psa_aead_test(PSA_KEY_TYPE_AES, alg,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK / 3,
                  PSA_TEST_IOVEC_NOT_INLINED,
                  ret);

    psa_aead_test(PSA_KEY_TYPE_AES, alg,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK / 4,
                  PSA_TEST_IOVEC_NOT_INLINED,
                  ret);
#endif /* CC3XX_RUNTIME_ENABLED */

    psa_aead_test(PSA_KEY_TYPE_AES, alg,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK,
                  PSA_TEST_IOVEC_INLINED,
                  ret);

#ifdef CC3XX_RUNTIME_ENABLED
    psa_aead_test(PSA_KEY_TYPE_AES, alg,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK / 3,
                  PSA_TEST_IOVEC_INLINED,
                  ret);

    psa_aead_test(PSA_KEY_TYPE_AES, alg,
                  test_key_128, BIT_SIZE_TEST_KEY,
                  BYTE_SIZE_CHUNK / 4,
                  PSA_TEST_IOVEC_INLINED,
                  ret);
#endif /* CC3XX_RUNTIME_ENABLED */
}
#endif /* TFM_CRYPTO_TEST_ALG_CCM */

static void tfm_crypto_test_1036(struct test_result_t *ret)
{
    psa_key_derivation_test(PSA_ALG_TLS12_PRF(PSA_ALG_SHA_256), ret);
}

static void tfm_crypto_test_1037(struct test_result_t *ret)
{
    psa_key_derivation_test(PSA_ALG_TLS12_PSK_TO_MS(PSA_ALG_SHA_256), ret);
}

#ifdef TFM_CRYPTO_TEST_HKDF
static void tfm_crypto_test_1038(struct test_result_t *ret)
{
    psa_key_derivation_test(PSA_ALG_HKDF(PSA_ALG_SHA_256), ret);
}
#endif /* TFM_CRYPTO_TEST_HKDF */

#ifdef TFM_CRYPTO_TEST_ECDH
static void tfm_crypto_test_1039(struct test_result_t *ret)
{
    psa_key_agreement_test(PSA_ALG_ECDH, ret);
}
#endif /* TFM_CRYPTO_TEST_ECDH */

#if defined(TFM_CRYPTO_TEST_ALG_OFB) && !defined(CC3XX_RUNTIME_ENABLED)
static void tfm_crypto_test_1040(struct test_result_t *ret)
{
    psa_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_OFB,
                    test_key_128, BIT_SIZE_TEST_KEY, ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_OFB && !CC3XX_RUNTIME_ENABLED */

#ifdef TFM_CRYPTO_TEST_ALG_ECB
static void tfm_crypto_test_1041(struct test_result_t *ret)
{
    psa_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_ECB_NO_PADDING,
                    test_key_128, BIT_SIZE_TEST_KEY, ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_ECB */

#if CRYPTO_ASYM_ENCRYPT_MODULE_ENABLED
static void tfm_crypto_test_1042(struct test_result_t *ret)
{
    psa_asymmetric_encryption_test(PSA_ALG_RSA_OAEP(PSA_ALG_SHA_256), ret);
}

static void tfm_crypto_test_1043(struct test_result_t *ret)
{
    psa_asymmetric_encryption_test(PSA_ALG_RSA_PKCS1V15_CRYPT, ret);
}
#endif /* CRYPTO_ASYM_ENCRYPT_MODULE_ENABLED */
#if CRYPTO_ASYM_SIGN_MODULE_ENABLED
#if TFM_CRYPTO_TEST_ALG_DETERMINISTIC_ECDSA
static void tfm_crypto_test_1044(struct test_result_t *ret)
{
    psa_sign_verify_message_test(
        PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_DETERMINISTIC_ECDSA */
#if TFM_CRYPTO_TEST_ALG_ECDSA
static void tfm_crypto_test_1053(struct test_result_t *ret)
{
    psa_sign_verify_hash_test(PSA_ALG_ECDSA(PSA_ALG_SHA_256), 0, ret);
}

static void tfm_crypto_test_1056(struct test_result_t *ret)
{
#if defined(PSA_WANT_ECC_SECP_R1_384) && defined(CC3XX_RUNTIME_ENABLED)
    psa_sign_verify_hash_test(PSA_ALG_ECDSA(PSA_ALG_SHA_384), 1, ret);

    psa_verify_hash_test(PSA_ALG_ECDSA(PSA_ALG_SHA_384), 1 /* Unused */, ret);
#else
    TEST_LOG("P384 is unsupported. Skipping...");
    ret->val = 0;
#endif
}
#endif /* TFM_CRYPTO_TEST_ALG_ECDSA */
#endif /* CRYPTO_ASYM_SIGN_MODULE_ENABLED */

#ifdef TFM_CRYPTO_TEST_ALG_CBC
static void tfm_crypto_test_1045(struct test_result_t *ret)
{
    psa_cipher_test(PSA_KEY_TYPE_AES, PSA_ALG_CBC_PKCS7,
                    test_key_128, BIT_SIZE_TEST_KEY, ret);
}

static void tfm_crypto_test_1046(struct test_result_t *ret)
{
    psa_cipher_padded_modes_test(PSA_KEY_TYPE_AES, PSA_ALG_CBC_PKCS7, 12, ret);
}

static void tfm_crypto_test_1047(struct test_result_t *ret)
{
    psa_cipher_padded_modes_test(PSA_KEY_TYPE_AES, PSA_ALG_CBC_PKCS7, 20, ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_CBC */

#ifdef TFM_CRYPTO_TEST_CHACHA20
static void tfm_crypto_test_1048(struct test_result_t *ret)
{
    psa_cipher_test(PSA_KEY_TYPE_CHACHA20, PSA_ALG_STREAM_CIPHER,
                    test_key_256, BIT_SIZE_TEST_LONG_KEY, ret);
}

static void tfm_crypto_test_1051(struct test_result_t *ret)
{
    psa_cipher_rfc7539_test(ret);
}
#endif /* TFM_CRYPTO_TEST_CHACHA20 */

#ifdef TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305
static void tfm_crypto_test_1049(struct test_result_t *ret)
{
    psa_aead_test(PSA_KEY_TYPE_CHACHA20, PSA_ALG_CHACHA20_POLY1305,
                  test_key_256, BIT_SIZE_TEST_LONG_KEY,
                  BYTE_SIZE_CHUNK, ret);
}

static void tfm_crypto_test_1052(struct test_result_t *ret)
{
    psa_aead_rfc7539_test(ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305 */

#if defined(TFM_CRYPTO_TEST_ALG_RSASSA_PSS_VERIFICATION) && !defined(CC3XX_RUNTIME_ENABLED)
static void tfm_crypto_test_1050(struct test_result_t *ret)
{
    psa_verify_rsassa_pss_test(ret);
}
#endif /* TFM_CRYPTO_TEST_ALG_RSASSA_PSS_VERIFICATION */

#if defined(TFM_CRYPTO_TEST_SINGLE_PART_FUNCS)
#if defined(TFM_CRYPTO_TEST_ALG_GCM) || defined(TFM_CRYPTO_TEST_ALG_CCM)
static void tfm_crypto_test_1054(struct test_result_t *ret)
{
    if (!psa_aead_as_authenticator_test()) {
        ret->val = TEST_PASSED;
    } else {
        ret->val = TEST_FAILED;
    }
}
#endif /* TFM_CRYPTO_TEST_ALG_GCM || TFM_CRYPTO_TEST_ALG_CCM */
#endif /* TFM_CRYPTO_TEST_SINGLE_PART_FUNCS */

#if defined(TFM_CRYPTO_TEST_WP_SECP384_R1)
static void tfm_crypto_test_1055(struct test_result_t *ret)
{
    wp_ec_test_runner(
        ret,
        PSA_ALG_ECDSA(PSA_ALG_SHA_384),
        PSA_KEY_USAGE_VERIFY_HASH,
        PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
}
#endif /* TFM_CRYPTO_TEST_WP_SECP384_R1 */
