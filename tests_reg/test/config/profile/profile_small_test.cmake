#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

################################## Tests #######################################

set(TFM_CRYPTO_TEST_ALG_CBC                     OFF     CACHE BOOL      "Test CBC cryptography mode")
set(TFM_CRYPTO_TEST_ALG_CFB                     OFF     CACHE BOOL      "Test CFB cryptography mode")
set(TFM_CRYPTO_TEST_ALG_ECB                     OFF     CACHE BOOL      "Test ECB cryptography mode")
set(TFM_CRYPTO_TEST_ALG_CTR                     OFF     CACHE BOOL      "Test CTR cryptography mode")
set(TFM_CRYPTO_TEST_ALG_OFB                     OFF     CACHE BOOL      "Test OFB cryptography mode")
set(TFM_CRYPTO_TEST_ALG_GCM                     OFF     CACHE BOOL      "Test GCM cryptography mode")
set(TFM_CRYPTO_TEST_ALG_SHA_224                 OFF     CACHE BOOL      "Test SHA-224 cryptography algorithm")
set(TFM_CRYPTO_TEST_ALG_SHA_384                 OFF     CACHE BOOL      "Test SHA-384 cryptography algorithm")
set(TFM_CRYPTO_TEST_ALG_SHA_512                 OFF     CACHE BOOL      "Test SHA-512 cryptography algorithm")
set(TFM_CRYPTO_TEST_HKDF                        OFF     CACHE BOOL      "Test the HKDF key derivation algorithm")
set(TFM_CRYPTO_TEST_ECDH                        OFF     CACHE BOOL      "Test the ECDH key agreement algorithm")
set(TFM_CRYPTO_TEST_CHACHA20                    OFF     CACHE BOOL      "Test the ChaCha20 stream cipher")
set(TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305       OFF     CACHE BOOL      "Test ChaCha20-Poly1305 AEAD algorithm")
set(TFM_CRYPTO_TEST_SINGLE_PART_FUNCS           OFF     CACHE BOOL      "Test single-part operations in hash, MAC, AEAD and symmetric ciphers")
set(TFM_CRYPTO_TEST_ALG_RSASSA_PSS_VERIFICATION OFF     CACHE BOOL      "Test RSASSA-PSS signature verification algorithm")
set(TFM_CRYPTO_TEST_UNSUPPORTED_ALG             ON      CACHE BOOL      "Test unsupported algorithm in hash, MAC")
set(TFM_CRYPTO_TEST_ALG_DETERMINISTIC_ECDSA     OFF     CACHE BOOL      "Test Deterministic ECDSA signing/verification algorithm")
set(TFM_CRYPTO_TEST_ALG_ECDSA                   OFF     CACHE BOOL      "Test ECDSA signing/verification algorithm")
