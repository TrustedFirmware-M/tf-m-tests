/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <string.h>
#include <stdbool.h>
#include "crypto_tests_common.h"

void psa_key_interface_test(const psa_key_type_t key_type,
                            struct test_result_t *ret)
{
    psa_status_t status = PSA_SUCCESS;
    uint32_t i = 0;
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    const uint8_t data[] = "THIS IS MY KEY1";
    uint8_t exported_data[sizeof(data)] = {0};
    size_t exported_data_size = 0;
    psa_key_attributes_t key_attributes = psa_key_attributes_init();
    psa_key_attributes_t retrieved_attributes = psa_key_attributes_init();

    /* Setup the key policy */
    psa_set_key_usage_flags(&key_attributes, PSA_KEY_USAGE_EXPORT);
    psa_set_key_type(&key_attributes, key_type);

    status = psa_import_key(&key_attributes, data, sizeof(data),
                            &key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing a key");
        return;
    }

    status = psa_get_key_attributes(key_id_local, &retrieved_attributes);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error getting key metadata");
        return;
    }

    if (psa_get_key_bits(&retrieved_attributes) != BIT_SIZE_TEST_KEY) {
        TEST_FAIL("The number of key bits is different from expected");
        return;
    }

    if (psa_get_key_type(&retrieved_attributes) != key_type) {
        TEST_FAIL("The type of the key is different from expected");
        return;
    }

    psa_reset_key_attributes(&retrieved_attributes);

    status = psa_export_key(key_id_local,
                            exported_data,
                            sizeof(data),
                            &exported_data_size);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error exporting a key");
        return;
    }

    if (exported_data_size != BYTE_SIZE_TEST_KEY) {
        TEST_FAIL("Number of bytes of exported key different from expected");
        return;
    }

    /* Check that the exported key is the same as the imported one */
    for (i=0; i<exported_data_size; i++) {
        if (exported_data[i] != data[i]) {
            TEST_FAIL("Exported key doesn't match the imported key");
            return;
        }
    }

    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error destroying the key");
        return;
    }

    status = psa_get_key_attributes(key_id_local, &retrieved_attributes);
    if (status != PSA_ERROR_INVALID_HANDLE) {
        TEST_FAIL("Key ID should be invalid now");
        return;
    }

    psa_reset_key_attributes(&retrieved_attributes);

    ret->val = TEST_PASSED;
}

void psa_cipher_padded_modes_test(const psa_key_type_t key_type,
                                  const psa_algorithm_t alg,
                                  uint8_t len,
                                  struct test_result_t *ret)
{
    psa_cipher_operation_t handle = psa_cipher_operation_init();
    psa_cipher_operation_t handle_dec = psa_cipher_operation_init();
    psa_status_t status = PSA_SUCCESS;
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    const uint8_t data[] = "THIS IS MY KEY1";
    const size_t iv_length = PSA_BLOCK_CIPHER_BLOCK_LENGTH(key_type);
    const uint8_t iv[] = "012345678901234";
    const uint8_t plain_text[PLAIN_DATA_SIZE_PAD_TEST] =
        "Little text, full!!";
    uint8_t decrypted_data[ENC_DEC_BUFFER_SIZE_PAD_TEST] = {0};
    size_t output_length = 0, total_output_length = 0;
    uint8_t encrypted_data[ENC_DEC_BUFFER_SIZE_PAD_TEST] = {0};
    uint32_t comp_result;
    psa_key_attributes_t key_attributes = psa_key_attributes_init();
    psa_key_usage_t usage = (PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    bool bAbortDecryption = false;

    if (iv_length != sizeof(iv)) {
        /* Whenever this condition is hit, it's likely the test requires
         * refactoring to remove any hardcoded behaviour
         */
        TEST_FAIL("Hardcoded IV does not match cipher block length");
        return;
    }

    if (len > sizeof(plain_text)) {
        TEST_FAIL("Requested input length is greater than supported");
        return;
    }

    ret->val = TEST_PASSED;

    /* Setup the key policy */
    psa_set_key_usage_flags(&key_attributes, usage);
    psa_set_key_algorithm(&key_attributes, alg);
    psa_set_key_type(&key_attributes, key_type);

    /* Import a key */
    status = psa_import_key(&key_attributes, data, sizeof(data), &key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing a key");
        return;
    }

    status = psa_get_key_attributes(key_id_local, &key_attributes);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error getting key metadata");
        goto destroy_key;
    }

    if (psa_get_key_bits(&key_attributes) != BIT_SIZE_TEST_KEY) {
        TEST_FAIL("The number of key bits is different from expected");
        goto destroy_key;
    }

    if (psa_get_key_type(&key_attributes) != key_type) {
        TEST_FAIL("The type of the key is different from expected");
        goto destroy_key;
    }

    /* Setup the encryption object */
    status = psa_cipher_encrypt_setup(&handle, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_FAIL("Algorithm NOT SUPPORTED by the implementation");
        } else {
            TEST_FAIL("Error setting up cipher operation object");
        }
        goto destroy_key;
    }

    /* Set the IV */
    status = psa_cipher_set_iv(&handle, iv, iv_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting the IV on the cipher operation object");
        goto abort;
    }

    /* Encrypt one chunk of information */
    if (len < BYTE_SIZE_CHUNK) {
        status = psa_cipher_update(&handle, plain_text,
                                   len,
                                   encrypted_data,
                                   sizeof(encrypted_data),
                                   &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error encrypting one chunk of information");
            goto abort;
        }

        /* When encrypting less than a block, the output is produced only
         * when performing the following finish operation
         */
        if (output_length != 0) {
            TEST_FAIL("Expected encrypted length is different from expected");
            goto abort;
        }

        status = psa_cipher_finish(&handle, encrypted_data,
                                   sizeof(encrypted_data),
                                   &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error finalising the cipher operation");
            goto abort;
        }

    } else if (len < 2 * BYTE_SIZE_CHUNK) {
        status = psa_cipher_update(&handle, plain_text,
                                   BYTE_SIZE_CHUNK,
                                   encrypted_data,
                                   sizeof(encrypted_data),
                                   &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error encrypting one chunk of information");
            goto abort;
        }

        /* When encrypting one block, the output is produced right away */
        if (output_length != BYTE_SIZE_CHUNK) {
            TEST_FAIL("Expected encrypted length is different from expected");
            goto abort;
        }

        total_output_length += output_length;
        status = psa_cipher_update(&handle, &plain_text[BYTE_SIZE_CHUNK],
                                   len % BYTE_SIZE_CHUNK,
                                   &encrypted_data[total_output_length],
                                   sizeof(encrypted_data) - total_output_length,
                                   &output_length);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error encrypting one chunk of information");
            goto abort;
        }

        /* When encrypting less than a block, the output is zero */
        if (output_length != 0) {
            TEST_FAIL("Expected encrypted length is different from expected");
            goto abort;
        }

        /* The output is then produced only when calling finish if the previous
         * update did not produce any output - We need to take padding into
         * account
         */
        total_output_length += output_length;
        status = psa_cipher_finish(&handle, &encrypted_data[total_output_length],
                                   sizeof(encrypted_data) - total_output_length,
                                   &output_length);

        total_output_length += output_length;
    }

    /* Setup the decryption object */
    status = psa_cipher_decrypt_setup(&handle_dec, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting up cipher operation object");
        goto destroy_key;
    }

    /* From now on, in case of failure we want to abort the decryption op */
    bAbortDecryption = true;

    /* Set the IV for decryption */
    status = psa_cipher_set_iv(&handle_dec, iv, iv_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting the IV for decryption");
        goto abort;
    }

    /* Reset total output length */
    total_output_length = 0;
    if (len < BYTE_SIZE_CHUNK) {
        status = psa_cipher_update(&handle_dec,
                                   encrypted_data,
                                   BYTE_SIZE_CHUNK,
                                   decrypted_data,
                                   sizeof(decrypted_data),
                                   &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error decrypting one chunk of information");
            goto abort;
        }

        /* Doesn't produce output on the first cipher update */
        if (output_length != 0) {
            TEST_FAIL("Expected decrypted length is different from expected");
            goto abort;
        }

        status = psa_cipher_finish(&handle_dec, decrypted_data,
                                   sizeof(decrypted_data),
                                   &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error finalising the cipher operation");
            goto abort;
        }

        if (output_length != len) {
            TEST_FAIL("Expected decrypted length is different from expected");
            goto destroy_key;
        }

    } else if (len < 2*BYTE_SIZE_CHUNK) {
        status = psa_cipher_update(&handle_dec, encrypted_data,
                                   BYTE_SIZE_CHUNK,
                                   decrypted_data,
                                   sizeof(decrypted_data),
                                   &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error encrypting one chunk of information");
            goto abort;
        }

        /* Doesn't produce output on the first cipher update */
        if (output_length != 0) {
            TEST_FAIL("Expected decrypted length is different from expected");
            goto abort;
        }

        total_output_length += output_length;
        status = psa_cipher_update(&handle_dec,
                                   &encrypted_data[BYTE_SIZE_CHUNK],
                                   BYTE_SIZE_CHUNK,
                                   &decrypted_data[total_output_length],
                                   sizeof(decrypted_data) - total_output_length,
                                   &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error decrypting one chunk of information");
            goto abort;
        }

        /* We now get the output corresponding to the previous block */
        if (output_length != BYTE_SIZE_CHUNK) {
            TEST_FAIL("Expected decrypted length is different from expected");
            goto abort;
        }

        total_output_length += output_length;
        status = psa_cipher_finish(&handle_dec,
                                   &decrypted_data[total_output_length],
                                   sizeof(decrypted_data) - total_output_length,
                                   &output_length);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error finalising the cipher operation");
            goto abort;
        }

        total_output_length += output_length;
        if (total_output_length != len) {
            TEST_FAIL("Expected decrypted length is different from expected");
            goto destroy_key;
        }
    }

    /* Check that the plain text matches the decrypted data */
    comp_result = memcmp(plain_text, decrypted_data, len);
    if (comp_result != 0) {
        TEST_FAIL("Decrypted data doesn't match with plain text");
        goto destroy_key;
    }

    /* Go directly to destroy key from here */
    goto destroy_key;

abort:
    /* Abort the operation */
    status = bAbortDecryption ? psa_cipher_abort(&handle_dec) :
                                psa_cipher_abort(&handle);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error aborting the operation");
    }
destroy_key:
    /* Destroy the key */
    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error destroying a key");
    }
}

#ifdef TFM_CRYPTO_TEST_CHACHA20
/* Chacha20 test vectors are taken directly from RFC7539 */
static const uint8_t chacha20_testKey[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

static const uint8_t chacha20_testNonce[] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x4a,
  0x00, 0x00, 0x00, 0x00
};

/* The initial counter of the Chacha20 RFC7539 test vectors is 1, while the PSA
 * APIs assume it to be zero. This means that this expected ciphertext is not
 * the same as the one presented in the RFC
 */
static const uint8_t chacha20_testCiphertext_expected[] = {
  0xe3, 0x64, 0x7a, 0x29, 0xde, 0xd3, 0x15, 0x28, 0xef, 0x56, 0xba, 0xc7,
  0x0f, 0x7a, 0x7a, 0xc3, 0xb7, 0x35, 0xc7, 0x44, 0x4d, 0xa4, 0x2d, 0x99,
  0x82, 0x3e, 0xf9, 0x93, 0x8c, 0x8e, 0xbf, 0xdc, 0xf0, 0x5b, 0xb7, 0x1a,
  0x82, 0x2c, 0x62, 0x98, 0x1a, 0xa1, 0xea, 0x60, 0x8f, 0x47, 0x93, 0x3f,
  0x2e, 0xd7, 0x55, 0xb6, 0x2d, 0x93, 0x12, 0xae, 0x72, 0x03, 0x76, 0x74,
  0xf3, 0xe9, 0x3e, 0x24, 0x4c, 0x23, 0x28, 0xd3, 0x2f, 0x75, 0xbc, 0xc1,
  0x5b, 0xb7, 0x57, 0x4f, 0xde, 0x0c, 0x6f, 0xcd, 0xf8, 0x7b, 0x7a, 0xa2,
  0x5b, 0x59, 0x72, 0x97, 0x0c, 0x2a, 0xe6, 0xcc, 0xed, 0x86, 0xa1, 0x0b,
  0xe9, 0x49, 0x6f, 0xc6, 0x1c, 0x40, 0x7d, 0xfd, 0xc0, 0x15, 0x10, 0xed,
  0x8f, 0x4e, 0xb3, 0x5d, 0x0d, 0x62
};
#endif /* TFM_CRYPTO_TEST_CHACHA20 */

#if defined(TFM_CRYPTO_TEST_CHACHA20) ||      \
    defined(TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305)
/* The plaintext of the vectors is the same for both Chacha20 and
 * Chacha20-Poly1305
 */
static const uint8_t chacha20_testPlaintext[] = {
  0x4c, 0x61, 0x64, 0x69, 0x65, 0x73, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x47,
  0x65, 0x6e, 0x74, 0x6c, 0x65, 0x6d, 0x65, 0x6e, 0x20, 0x6f, 0x66, 0x20,
  0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x61, 0x73, 0x73, 0x20, 0x6f, 0x66,
  0x20, 0x27, 0x39, 0x39, 0x3a, 0x20, 0x49, 0x66, 0x20, 0x49, 0x20, 0x63,
  0x6f, 0x75, 0x6c, 0x64, 0x20, 0x6f, 0x66, 0x66, 0x65, 0x72, 0x20, 0x79,
  0x6f, 0x75, 0x20, 0x6f, 0x6e, 0x6c, 0x79, 0x20, 0x6f, 0x6e, 0x65, 0x20,
  0x74, 0x69, 0x70, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20,
  0x66, 0x75, 0x74, 0x75, 0x72, 0x65, 0x2c, 0x20, 0x73, 0x75, 0x6e, 0x73,
  0x63, 0x72, 0x65, 0x65, 0x6e, 0x20, 0x77, 0x6f, 0x75, 0x6c, 0x64, 0x20,
  0x62, 0x65, 0x20, 0x69, 0x74, 0x2e
};
/* To hold intermediate results in both Chacha20 and Chacha20-Poly1305 */
static uint8_t chacha20_testCiphertext[sizeof(chacha20_testPlaintext)] = {0};
static uint8_t chacha20_testDecryptedtext[sizeof(chacha20_testPlaintext)] = {0};
#endif /* TFM_CRYPTO_TEST_CHACHA20 || TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305 */

#ifdef TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305
/* Chacha20-Poly1305 test vectors are taken directly from RFC7539 */
static const uint8_t chacha20poly1305_testKey[] = {
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
  0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
  0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f
};

static const uint8_t chacha20poly1305_testNonce[] = {
  0x07, 0x00, 0x00, 0x00, /* constant */
  0x40, 0x41, 0x42, 0x43, /* IV[0] */
  0x44, 0x45, 0x46, 0x47  /* IV[1] */
};

static const uint8_t chacha20poly1305_testAad[] = {
  0x50, 0x51, 0x52, 0x53, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7
};

static const uint8_t chacha20poly1305_testCiphertext_expected[] = {
  0xd3, 0x1a, 0x8d, 0x34, 0x64, 0x8e, 0x60, 0xdb, 0x7b, 0x86, 0xaf, 0xbc,
  0x53, 0xef, 0x7e, 0xc2, 0xa4, 0xad, 0xed, 0x51, 0x29, 0x6e, 0x08, 0xfe,
  0xa9, 0xe2, 0xb5, 0xa7, 0x36, 0xee, 0x62, 0xd6, 0x3d, 0xbe, 0xa4, 0x5e,
  0x8c, 0xa9, 0x67, 0x12, 0x82, 0xfa, 0xfb, 0x69, 0xda, 0x92, 0x72, 0x8b,
  0x1a, 0x71, 0xde, 0x0a, 0x9e, 0x06, 0x0b, 0x29, 0x05, 0xd6, 0xa5, 0xb6,
  0x7e, 0xcd, 0x3b, 0x36, 0x92, 0xdd, 0xbd, 0x7f, 0x2d, 0x77, 0x8b, 0x8c,
  0x98, 0x03, 0xae, 0xe3, 0x28, 0x09, 0x1b, 0x58, 0xfa, 0xb3, 0x24, 0xe4,
  0xfa, 0xd6, 0x75, 0x94, 0x55, 0x85, 0x80, 0x8b, 0x48, 0x31, 0xd7, 0xbc,
  0x3f, 0xf4, 0xde, 0xf0, 0x8e, 0x4b, 0x7a, 0x9d, 0xe5, 0x76, 0xd2, 0x65,
  0x86, 0xce, 0xc6, 0x4b, 0x61, 0x16
};

static const uint8_t chacha20poly1305_testTag_expected[] = {
  0x1a, 0xe1, 0x0b, 0x59, 0x4f, 0x09, 0xe2, 0x6a,
  0x7e, 0x90, 0x2e, 0xcb, 0xd0, 0x60, 0x06, 0x91
};
#endif /* TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305 */

#ifdef TFM_CRYPTO_TEST_CHACHA20
void psa_cipher_rfc7539_test(struct test_result_t *ret)
{
    psa_cipher_operation_t handle = psa_cipher_operation_init();
    psa_cipher_operation_t handle_dec = psa_cipher_operation_init();
    psa_status_t status = PSA_SUCCESS;
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    psa_key_attributes_t key_attributes = psa_key_attributes_init();
    psa_key_usage_t usage = (PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    const psa_key_type_t key_type = PSA_KEY_TYPE_CHACHA20;
    const psa_algorithm_t alg = PSA_ALG_STREAM_CIPHER;
    bool bAbortDecryption = false;
    /* Variables required during multipart update */
    size_t data_left = sizeof(chacha20_testPlaintext);
    size_t lengths[] = {42, 24, 48};
    size_t start_idx = 0;
    size_t output_length = 0; size_t total_output_length = 0;
    int comp_result;

    ret->val = TEST_PASSED;

    /* Setup the key policy */
    psa_set_key_usage_flags(&key_attributes, usage);
    psa_set_key_algorithm(&key_attributes, alg);
    psa_set_key_type(&key_attributes, key_type);

    status = psa_import_key(&key_attributes, chacha20_testKey,
                            sizeof(chacha20_testKey), &key_id_local);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing a key");
        return;
    }

    /* Setup the encryption object */
    status = psa_cipher_encrypt_setup(&handle, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Encryption setup shouldn't fail");
        goto destroy_key;
    }

    /* Set the IV */
    status = psa_cipher_set_iv(&handle,
                               chacha20_testNonce, sizeof(chacha20_testNonce));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting the IV on the cipher operation object");
        goto abort;
    }

    for (int i=0; i<sizeof(lengths)/sizeof(size_t); i++) {
        /* Encrypt one chunk of information */
        status = psa_cipher_update(
                        &handle,
                        &chacha20_testPlaintext[start_idx],
                        lengths[i],
                        &chacha20_testCiphertext[total_output_length],
                        sizeof(chacha20_testCiphertext) - total_output_length,
                        &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error encrypting one chunk of information");
            goto abort;
        }

        if (output_length != lengths[i]) {
            TEST_FAIL("Expected encrypted length is different from expected");
            goto abort;
        }

        data_left -= lengths[i];
        total_output_length += output_length;

        start_idx += lengths[i];
    }

    /* Finalise the cipher operation */
    status = psa_cipher_finish(
                    &handle,
                    &chacha20_testCiphertext[total_output_length],
                    sizeof(chacha20_testCiphertext) - total_output_length,
                    &output_length);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error finalising the cipher operation");
        goto abort;
    }

    if (output_length != 0) {
        TEST_FAIL("Un-padded mode final output length unexpected");
        goto abort;
    }

    /* Add the last output produced, it might be encrypted padding */
    total_output_length += output_length;

    /* Compare encrypted data produced with single-shot and multipart APIs */
    comp_result = memcmp(chacha20_testCiphertext_expected,
                         chacha20_testCiphertext,
                         total_output_length);
    if (comp_result != 0) {
        TEST_FAIL("Single-shot crypt doesn't match with multipart crypt");
        goto destroy_key;
    }

    /* Setup the decryption object */
    status = psa_cipher_decrypt_setup(&handle_dec, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting up cipher operation object");
        goto destroy_key;
    }

    /* From now on, in case of failure we want to abort the decryption op */
    bAbortDecryption = true;

    /* Set the IV for decryption */
    status = psa_cipher_set_iv(&handle_dec,
                               chacha20_testNonce, sizeof(chacha20_testNonce));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting the IV for decryption");
        goto abort;
    }

    /* Decrypt - total_output_length considers encrypted padding */
    data_left = total_output_length;
    /* Update in different chunks of plainText */
    lengths[0] = 14; lengths[1] = 70; lengths[2] = 30;
    start_idx = 0;
    output_length = 0; total_output_length = 0;
    for (int i=0; i<sizeof(lengths)/sizeof(size_t); i++) {
        status = psa_cipher_update(
                    &handle_dec,
                    &chacha20_testCiphertext[start_idx],
                    lengths[i],
                    &chacha20_testDecryptedtext[total_output_length],
                    sizeof(chacha20_testDecryptedtext) - total_output_length,
                    &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error decrypting one chunk of information");
            goto abort;
        }

        if (output_length != lengths[i]) {
            TEST_FAIL("Expected encrypted length is different from expected");
            goto abort;
        }

        data_left -= lengths[i];
        total_output_length += output_length;

        start_idx += lengths[i];
    }

    /* Finalise the cipher operation for decryption */
    status = psa_cipher_finish(
                    &handle_dec,
                    &chacha20_testDecryptedtext[total_output_length],
                    sizeof(chacha20_testDecryptedtext) - total_output_length,
                    &output_length);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error finalising the cipher operation");
        goto abort;
    }

    /* Finalize the count of output which has been produced */
    total_output_length += output_length;

    /* Check that the decrypted length is equal to the original length */
    if (total_output_length != sizeof(chacha20_testPlaintext)) {
        TEST_FAIL("After finalising, unexpected decrypted length");
        goto destroy_key;
    }

    /* Check that the plain text matches the decrypted data */
    comp_result = memcmp(chacha20_testPlaintext,
                         chacha20_testDecryptedtext,
                         sizeof(chacha20_testPlaintext));
    if (comp_result != 0) {
        TEST_FAIL("Decrypted data doesn't match with plain text");
    }

    /* Go directly to the destroy_key label at this point */
    goto destroy_key;

abort:
    /* Abort the operation */
    status = bAbortDecryption ? psa_cipher_abort(&handle_dec) :
                                psa_cipher_abort(&handle);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error aborting the operation");
    }
destroy_key:
    /* Destroy the key */
    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error destroying a key");
    }

    return;
}
#endif /* TFM_CRYPTO_TEST_CHACHA20 */

#ifdef TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305
void psa_aead_rfc7539_test(struct test_result_t *ret)
{
    psa_aead_operation_t handle = psa_aead_operation_init();
    psa_aead_operation_t handle_dec = psa_aead_operation_init();
    psa_status_t status = PSA_SUCCESS;
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    psa_key_attributes_t key_attributes = psa_key_attributes_init();
    psa_key_usage_t usage = (PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    const psa_key_type_t key_type = PSA_KEY_TYPE_CHACHA20;
    const psa_algorithm_t alg = PSA_ALG_CHACHA20_POLY1305;
    uint8_t tag[16] = {0}; /* tag in chacha20-poly1305 is 16 bytes */
    size_t tag_length = 0;
    bool bAbortDecryption = false;
    /* Variables related to multipart update */
    size_t data_left = sizeof(chacha20_testPlaintext);
    size_t lengths[] = {42, 24, 48};
    size_t start_idx = 0;
    size_t output_length = 0; size_t total_output_length = 0;
    int comp_result;
#ifdef TFM_CRYPTO_TEST_SINGLE_PART_FUNCS
    uint8_t encrypted_data_single_shot[sizeof(chacha20_testPlaintext) + 16] = {0};
    size_t encrypted_data_length, decrypted_data_length;
#endif

    ret->val = TEST_PASSED;

    /* Setup the key policy */
    psa_set_key_usage_flags(&key_attributes, usage);
    psa_set_key_algorithm(&key_attributes, alg);
    psa_set_key_type(&key_attributes, key_type);
    status = psa_import_key(&key_attributes, chacha20poly1305_testKey,
                            sizeof(chacha20poly1305_testKey), &key_id_local);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing a key");
        return;
    }

#ifdef TFM_CRYPTO_TEST_SINGLE_PART_FUNCS
    /* Perform AEAD encryption */
    status = psa_aead_encrypt(key_id_local, alg,
                              chacha20poly1305_testNonce,
                              sizeof(chacha20poly1305_testNonce),
                              chacha20poly1305_testAad,
                              sizeof(chacha20poly1305_testAad),
                              chacha20_testPlaintext,
                              sizeof(chacha20_testPlaintext),
                              encrypted_data_single_shot,
                              sizeof(encrypted_data_single_shot),
                              &encrypted_data_length);

    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_FAIL("Algorithm NOT SUPPORTED by the implementation");
        } else {
            TEST_FAIL("Error performing single shot AEAD encryption");
        }
        goto destroy_key;
    }

    if (encrypted_data_length
        != PSA_AEAD_ENCRYPT_OUTPUT_SIZE(key_type, alg, sizeof(chacha20_testPlaintext))) {
        TEST_FAIL("Single shot encrypted data length is different than expected");
        goto destroy_key;
    }

    /* Check that the encrypted data is the same as reference */
    comp_result = memcmp(encrypted_data_single_shot,
                         chacha20poly1305_testCiphertext_expected,
                         sizeof(chacha20poly1305_testCiphertext_expected));
    if (comp_result != 0) {
        TEST_FAIL("Single shot encrypted data doesn't match with reference");
        goto destroy_key;
    }

    /* Check that the tag matches the expected tag */
    comp_result = memcmp(&encrypted_data_single_shot[encrypted_data_length - 16],
                         chacha20poly1305_testTag_expected,
                         sizeof(chacha20poly1305_testTag_expected));
    if (comp_result != 0) {
        TEST_FAIL("Single shot computed tag doesn't match with reference");
        goto destroy_key;
    }

    /* Perform AEAD decryption */
    status = psa_aead_decrypt(key_id_local, alg,
                              chacha20poly1305_testNonce,
                              sizeof(chacha20poly1305_testNonce),
                              chacha20poly1305_testAad,
                              sizeof(chacha20poly1305_testAad),
                              encrypted_data_single_shot,
                              encrypted_data_length,
                              chacha20_testCiphertext,
                              sizeof(chacha20_testCiphertext),
                              &decrypted_data_length);

    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_FAIL("Algorithm NOT SUPPORTED by the implementation");
        } else {
            TEST_FAIL("Error performing single shot AEAD decryption");
        }
        goto destroy_key;
    }

    if (sizeof(chacha20_testPlaintext) != decrypted_data_length) {
        TEST_FAIL("Single shot decrypted data length is different from plain text");
        goto destroy_key;
    }

    /* Check that the decrypted data is the same as the original data */
    comp_result = memcmp(chacha20_testPlaintext,
                         chacha20_testCiphertext,
                         sizeof(chacha20_testPlaintext));
    if (comp_result != 0) {
        TEST_FAIL("Single shot decrypted data doesn't match with plain text");
        goto destroy_key;
    }

    memset(chacha20_testCiphertext, 0, sizeof(chacha20_testCiphertext));
#endif /* TFM_CRYPTO_TEST_SINGLE_PART_FUNCS */

    /* Setup the encryption object */
    status = psa_aead_encrypt_setup(&handle, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Encryption setup shouldn't fail");
        goto destroy_key;
    }

    /* Set the IV */
    status = psa_aead_set_nonce(&handle,
                                chacha20poly1305_testNonce,
                                sizeof(chacha20poly1305_testNonce));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting the nonce on the aead operation object");
        goto abort;
    }

    /* Set lengths */
    status = psa_aead_set_lengths(&handle,
                                  sizeof(chacha20poly1305_testAad),
                                  sizeof(chacha20_testPlaintext));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting the lengths on the aead operation object");
        goto abort;
    }

    /* Update AD in one go */
    status = psa_aead_update_ad(&handle,
                                chacha20poly1305_testAad,
                                sizeof(chacha20poly1305_testAad));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error updating AD");
        goto abort;
    }

    for (int i=0; i<sizeof(lengths)/sizeof(size_t); i++) {
        /* Encrypt one chunk of information */
        status = psa_aead_update(
                        &handle,
                        &chacha20_testPlaintext[start_idx],
                        lengths[i],
                        &chacha20_testCiphertext[total_output_length],
                        sizeof(chacha20_testCiphertext) - total_output_length,
                        &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error encrypting one chunk of information");
            goto abort;
        }

        if (output_length != lengths[i]) {
            TEST_FAIL("Expected encrypted length is different from expected");
            goto abort;
        }

        data_left -= lengths[i];
        total_output_length += output_length;

        start_idx += lengths[i];
    }

    /* Finalise the cipher operation */
    status = psa_aead_finish(
                    &handle,
                    &chacha20_testCiphertext[total_output_length],
                    sizeof(chacha20_testCiphertext) - total_output_length,
                    &output_length,
                    tag,
                    sizeof(tag),
                    &tag_length);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error finalising the cipher operation");
        goto abort;
    }

    if (output_length != 0) {
        TEST_FAIL("Un-padded mode final output length unexpected");
        goto abort;
    }

    if (tag_length != 16) {
        TEST_FAIL("Unexpected tag length different than 16");
        goto abort;
    }

    /* Add the last output produced, it might be encrypted padding */
    total_output_length += output_length;

    /* Compare encrypted data produced with single-shot and multipart APIs */
    comp_result = memcmp(chacha20poly1305_testCiphertext_expected,
                         chacha20_testCiphertext,
                         total_output_length);
    if (comp_result != 0) {
        TEST_FAIL("Encrypted data does not match reference data");
        goto destroy_key;
    }

    comp_result = memcmp(chacha20poly1305_testTag_expected, tag, tag_length);
    if (comp_result != 0) {
        TEST_FAIL("Computed tag does not match reference data");
        goto destroy_key;
    }

    /* Setup the decryption object */
    status = psa_aead_decrypt_setup(&handle_dec, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting up aead operation object");
        goto destroy_key;
    }

    /* From now on, in case of failure we want to abort the decryption op */
    bAbortDecryption = true;

    /* Set the IV for decryption */
    status = psa_aead_set_nonce(&handle_dec,
                                chacha20poly1305_testNonce,
                                sizeof(chacha20poly1305_testNonce));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting the nonce for decryption");
        goto abort;
    }

    /* Set lengths */
    status = psa_aead_set_lengths(&handle_dec,
                                  sizeof(chacha20poly1305_testAad),
                                  sizeof(chacha20_testPlaintext));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting the lengths on the aead operation object");
        goto abort;
    }

    /* Update AD in one go */
    status = psa_aead_update_ad(&handle_dec,
                                chacha20poly1305_testAad,
                                sizeof(chacha20poly1305_testAad));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error updating AD");
        goto abort;
    }

    /* Decrypt - total_output_length considers encrypted padding */
    data_left = total_output_length;
    /* Update in different chunks of plainText */
    lengths[0] = 14; lengths[1] = 70; lengths[2] = 30;
    start_idx = 0;
    output_length = 0; total_output_length = 0;
    for (int i=0; i<sizeof(lengths)/sizeof(size_t); i++) {
        status = psa_aead_update(
                    &handle_dec,
                    &chacha20_testCiphertext[start_idx],
                    lengths[i],
                    &chacha20_testDecryptedtext[total_output_length],
                    sizeof(chacha20_testDecryptedtext) - total_output_length,
                    &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error decrypting one chunk of information");
            goto abort;
        }

        if (output_length != lengths[i]) {
            TEST_FAIL("Expected encrypted length is different from expected");
            goto abort;
        }

        data_left -= lengths[i];
        total_output_length += output_length;

        start_idx += lengths[i];
    }

    /* Finalise the cipher operation for decryption (destroys decrypted data) */
    status = psa_aead_verify(
                    &handle_dec,
                    &chacha20_testDecryptedtext[total_output_length],
                    sizeof(chacha20_testDecryptedtext) - total_output_length,
                    &output_length,
                    tag,
                    tag_length);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error verifying the aead operation");
        goto abort;
    }

    /* Finalize the count of output which has been produced */
    total_output_length += output_length;

    /* Check that the decrypted length is equal to the original length */
    if (total_output_length != sizeof(chacha20_testPlaintext)) {
        TEST_FAIL("After finalising, unexpected decrypted length");
        goto destroy_key;
    }

    /* Check that the plain text matches the decrypted data */
    comp_result = memcmp(chacha20_testPlaintext,
                         chacha20_testDecryptedtext,
                         sizeof(chacha20_testPlaintext));
    if (comp_result != 0) {
        TEST_FAIL("Decrypted data doesn't match with plain text");
    }

    /* Go directly to the destroy_key label at this point */
    goto destroy_key;

abort:
    /* Abort the operation */
    status = bAbortDecryption ? psa_aead_abort(&handle_dec) :
                                psa_aead_abort(&handle);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error aborting the operation");
    }
destroy_key:
    /* Destroy the key */
    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error destroying a key");
    }

    return;
}
#endif /* TFM_CRYPTO_TEST_ALG_CHACHA20_POLY1305 */

void psa_cipher_test(const psa_key_type_t key_type,
                     const psa_algorithm_t alg,
                     const uint8_t *key,
                     size_t key_bits,
                     struct test_result_t *ret)
{
    psa_cipher_operation_t handle = psa_cipher_operation_init();
    psa_cipher_operation_t handle_dec = psa_cipher_operation_init();
    psa_status_t status = PSA_SUCCESS;
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    size_t iv_length = PSA_CIPHER_IV_LENGTH(key_type, alg);
    uint8_t iv[16] = {0};
    const uint8_t plain_text[PLAIN_DATA_SIZE] =
        "This is my plaintext to encrypt, 48 bytes long!";
    uint8_t decrypted_data[ENC_DEC_BUFFER_SIZE] = {0};
    size_t output_length = 0, total_output_length = 0;
    union {
        uint8_t encrypted_data[ENC_DEC_BUFFER_SIZE];
        uint8_t encrypted_data_pad[ENC_DEC_BUFFER_SIZE_PAD_MODES];
    } input = {0};
    uint32_t comp_result;
    psa_key_attributes_t key_attributes = psa_key_attributes_init();
    psa_key_usage_t usage = (PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    bool bAbortDecryption = false;
#ifdef TFM_CRYPTO_TEST_SINGLE_PART_FUNCS
    uint8_t encrypted_data_single_shot[ENC_DEC_BUFFER_SIZE];
#endif

    if (iv_length > 16) {
        TEST_FAIL("Unexpected IV length greater than 16 for this alg/key type");
        return;
    }

    ret->val = TEST_PASSED;

    /* Setup the key policy */
    psa_set_key_usage_flags(&key_attributes, usage);
    psa_set_key_algorithm(&key_attributes, alg);
    psa_set_key_type(&key_attributes, key_type);

    /* Import a key */
    status = psa_import_key(&key_attributes, key, PSA_BITS_TO_BYTES(key_bits),
                            &key_id_local);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing a key");
        return;
    }

    status = psa_get_key_attributes(key_id_local, &key_attributes);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error getting key metadata");
        goto destroy_key;
    }

    if (psa_get_key_bits(&key_attributes) != key_bits) {
        TEST_FAIL("The number of key bits is different from expected");
        goto destroy_key;
    }

    if (psa_get_key_type(&key_attributes) != key_type) {
        TEST_FAIL("The type of the key is different from expected");
        goto destroy_key;
    }

    psa_reset_key_attributes(&key_attributes);

#ifdef TFM_CRYPTO_TEST_SINGLE_PART_FUNCS
    /* Encrypt single part functions */
    status = psa_cipher_encrypt(key_id_local, alg, plain_text,
                                sizeof(plain_text),
                                input.encrypted_data_pad,
                                sizeof(input.encrypted_data_pad),
                                &output_length);

    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_FAIL("Algorithm NOT SUPPORTED by the implementation");
        } else {
            TEST_FAIL("Error encrypting with the single-shot API");
        }
        goto destroy_key;
    }

    /* Store a copy of the encrypted data for later checking it against
     * multipart results
     */
    memcpy(encrypted_data_single_shot, &input.encrypted_data_pad[iv_length],
           output_length-iv_length);

    /* Make sure to use the randomly generated IV for the multipart flow */
    for (int i=0; i<iv_length; i++) {
        iv[i] = input.encrypted_data_pad[i];
    }

    status = psa_cipher_decrypt(key_id_local, alg,
                                input.encrypted_data_pad,
                                output_length,
                                decrypted_data, ENC_DEC_BUFFER_SIZE,
                                &output_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error decrypting with the single shot API");
        goto destroy_key;
    }

    if (sizeof(plain_text) != output_length) {
        TEST_FAIL("Unexpected output length");
        goto destroy_key;
    }

    /* Check that the plain text matches the decrypted data */
    comp_result = memcmp(plain_text, decrypted_data, sizeof(plain_text));
    if (comp_result != 0) {
        TEST_FAIL("Decrypted data doesn't match with plain text");
        goto destroy_key;
    }

    /* Clear inputs/outputs before moving to multipart tests */

    /* Clear intermediate buffers for additional single-shot API tests */
    memset(input.encrypted_data_pad, 0, sizeof(input.encrypted_data_pad));
    memset(decrypted_data, 0, sizeof(decrypted_data));
#endif /* TFM_CRYPTO_TEST_SINGLE_PART_FUNCS */

    /* Replicate the same test as above, but now using the multipart APIs */

    /* Setup the encryption object */
    status = psa_cipher_encrypt_setup(&handle, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_FAIL("Algorithm NOT SUPPORTED by the implementation");
        } else {
            TEST_FAIL("Error setting up cipher operation object");
        }
        goto destroy_key;
    }

    /* Set the IV */
    if (alg != PSA_ALG_ECB_NO_PADDING) {
        status = psa_cipher_set_iv(&handle, iv, iv_length);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error setting the IV on the cipher operation object");
            goto abort;
        }
    }

    size_t data_left = sizeof(plain_text);
    while (data_left) {
        /* Encrypt one chunk of information */
        status = psa_cipher_update(&handle, &plain_text[sizeof(plain_text) - data_left],
                                   BYTE_SIZE_CHUNK,
                                   &input.encrypted_data[total_output_length],
                                   ENC_DEC_BUFFER_SIZE - total_output_length,
                                   &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error encrypting one chunk of information");
            goto abort;
        }

        /* When CHACHA20 is tested, if the backend does not support working in
         * stream cipher, it will output only when it has enough data to fill a
         * CHACHA20 block, i.e. 64 bytes
         */
        if (output_length != BYTE_SIZE_CHUNK && alg != PSA_ALG_STREAM_CIPHER) {
            TEST_FAIL("Encrypted length different from expected");
            goto abort;
        }

        data_left -= BYTE_SIZE_CHUNK;
        total_output_length += output_length;
    }

    /* Finalise the cipher operation */
    status = psa_cipher_finish(&handle,
                               &input.encrypted_data[total_output_length],
                               ENC_DEC_BUFFER_SIZE - total_output_length,
                               &output_length);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error finalising the cipher operation");
        goto abort;
    }

    if (alg == PSA_ALG_CBC_PKCS7) {
        /* Finalisation produces an output for padded modes, which is the
         * encryption of the padded data added
         */
        if (output_length != BYTE_SIZE_CHUNK) {
            TEST_FAIL("Padded mode final output length unexpected");
            goto abort;
        }
    } else {
        if (output_length != 0 && alg != PSA_ALG_STREAM_CIPHER) {
            TEST_FAIL("Un-padded mode final output length unexpected");
            goto abort;
        }
    }

    /* Add the last output produced, it might be encrypted padding */
    total_output_length += output_length;

#ifdef TFM_CRYPTO_TEST_SINGLE_PART_FUNCS
    /* Compare encrypted data produced with single-shot and multipart APIs */
    comp_result = memcmp(encrypted_data_single_shot,
                         input.encrypted_data,
                         total_output_length);
    if (comp_result != 0) {
        TEST_FAIL("Single-shot crypt doesn't match with multipart crypt");
        goto destroy_key;
    }
#endif /* TFM_CRYPTO_TEST_SINGLE_PART_FUNCS */

    /* Setup the decryption object */
    status = psa_cipher_decrypt_setup(&handle_dec, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting up cipher operation object");
        goto destroy_key;
    }

    /* From now on, in case of failure we want to abort the decryption op */
    bAbortDecryption = true;

    /* Set the IV for decryption */
    if (alg != PSA_ALG_ECB_NO_PADDING) {
        status = psa_cipher_set_iv(&handle_dec, iv, iv_length);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error setting the IV for decryption");
            goto abort;
        }
    }

    /* Padded mode output is produced one block later */
    bool bIsLagging = false;
    if (alg == PSA_ALG_CBC_PKCS7) {
        bIsLagging = true; /* Padded modes lag by 1 block */
    }

    /* Decrypt - total_output_length considers encrypted padding */
    data_left = total_output_length;
    total_output_length = 0;
    size_t message_start = 0;
    while (data_left) {
        status = psa_cipher_update(&handle_dec,
                                   &input.encrypted_data[message_start],
                                   BYTE_SIZE_CHUNK,
                                   &decrypted_data[total_output_length],
                                   (ENC_DEC_BUFFER_SIZE - total_output_length),
                                   &output_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error decrypting one chunk of information");
            goto abort;
        }

        if (!bIsLagging && output_length != BYTE_SIZE_CHUNK && alg != PSA_ALG_STREAM_CIPHER) {
            TEST_FAIL("Expected encrypted length is different from expected");
            goto abort;
        }

        message_start += BYTE_SIZE_CHUNK;
        data_left -= BYTE_SIZE_CHUNK;
        total_output_length += output_length;
        bIsLagging = false;
    }

    /* Finalise the cipher operation for decryption (destroys decrypted data) */
    status = psa_cipher_finish(&handle_dec, &decrypted_data[total_output_length],
                               ENC_DEC_BUFFER_SIZE - total_output_length,
                               &output_length);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error finalising the cipher operation");
        goto abort;
    }

    /* Finalize the count of output which has been produced */
    total_output_length += output_length;

    /* Check that the decrypted length is equal to the original length */
    if (total_output_length != 3*BYTE_SIZE_CHUNK) {
        TEST_FAIL("After finalising, unexpected decrypted length");
        goto destroy_key;
    }

    /* Check that the plain text matches the decrypted data */
    comp_result = memcmp(plain_text, decrypted_data, sizeof(plain_text));
    if (comp_result != 0) {
        TEST_FAIL("Decrypted data doesn't match with plain text");
    }

    /* Go directly to the destroy_key label at this point */
    goto destroy_key;

abort:
    /* Abort the operation */
    status = bAbortDecryption ? psa_cipher_abort(&handle_dec) :
                                psa_cipher_abort(&handle);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error aborting the operation");
    }
destroy_key:
    /* Destroy the key */
    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error destroying a key");
    }
}

void psa_invalid_cipher_test(const psa_key_type_t key_type,
                             const psa_algorithm_t alg,
                             const size_t key_size,
                             struct test_result_t *ret)
{
    psa_status_t status;
    psa_cipher_operation_t handle = psa_cipher_operation_init();
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    uint8_t data[TEST_MAX_KEY_LENGTH];
    psa_key_attributes_t key_attributes = psa_key_attributes_init();
    psa_key_usage_t usage = (PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);

    /* Setup the key policy */
    psa_set_key_usage_flags(&key_attributes, usage);
    psa_set_key_algorithm(&key_attributes, alg);
    psa_set_key_type(&key_attributes, key_type);

    /* Fill the key data */
    (void)memset(data, 'A', key_size);

    /* Import a key */
    status = psa_import_key(&key_attributes, data, key_size, &key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing a key");
        return;
    }

    /* Setup the encryption object */
    status = psa_cipher_encrypt_setup(&handle, key_id_local, alg);
    if (status == PSA_SUCCESS) {
        TEST_FAIL("Should not successfully setup an invalid cipher");
        (void)psa_destroy_key(key_id_local);
        return;
    }

    /* Destroy the key */
    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error destroying a key");
        return;
    }

    ret->val = TEST_PASSED;
}

void psa_unsupported_hash_test(const psa_algorithm_t alg,
                               struct test_result_t *ret)
{
    psa_status_t status;
    psa_hash_operation_t handle = PSA_HASH_OPERATION_INIT;

    /* Setup the hash object for the unsupported hash algorithm */
    status = psa_hash_setup(&handle, alg);
    if (status != PSA_ERROR_NOT_SUPPORTED) {
        TEST_FAIL("Should not successfully setup an unsupported hash alg");
        return;
    }

    ret->val = TEST_PASSED;
}

/*
 * \brief This is the list of algorithms supported by the current
 *        configuration of the crypto engine used by the crypto
 *        service. In case the crypto engine default capabilities
 *        is changed, this list needs to be updated accordingly
 */
static const psa_algorithm_t hash_alg[] = {
    PSA_ALG_SHA_224,
    PSA_ALG_SHA_256,
    PSA_ALG_SHA_384,
    PSA_ALG_SHA_512,
};

static const uint8_t hash_val[][PSA_HASH_LENGTH(PSA_ALG_SHA_512)] = {
    {0x00, 0xD2, 0x90, 0xE2, 0x0E, 0x4E, 0xC1, 0x7E, /*!< SHA-224 */
     0x7A, 0x95, 0xF5, 0x10, 0x5C, 0x76, 0x74, 0x04,
     0x6E, 0xB5, 0x56, 0x5E, 0xE5, 0xE7, 0xBA, 0x15,
     0x6C, 0x23, 0x47, 0xF3},
    {0x6B, 0x22, 0x09, 0x2A, 0x37, 0x1E, 0xF5, 0x14, /*!< SHA-256 */
     0xF7, 0x39, 0x4D, 0xCF, 0xAD, 0x4D, 0x17, 0x46,
     0x66, 0xCB, 0x33, 0xA0, 0x39, 0xD8, 0x41, 0x4E,
     0xF1, 0x2A, 0xD3, 0x4D, 0x69, 0xC3, 0xB5, 0x3E},
    {0x64, 0x79, 0x11, 0xBB, 0x47, 0x4E, 0x47, 0x59, /*!< SHA-384 */
     0x3E, 0x4D, 0xBC, 0x60, 0xA5, 0xF9, 0xBF, 0x9C,
     0xC0, 0xBA, 0x55, 0x0F, 0x93, 0xCA, 0x72, 0xDF,
     0x57, 0x1E, 0x50, 0x56, 0xF9, 0x4A, 0x01, 0xD6,
     0xA5, 0x6F, 0xF7, 0x62, 0x34, 0x4F, 0x48, 0xFD,
     0x9D, 0x15, 0x07, 0x42, 0xB7, 0x72, 0x94, 0xB8},
    {0xB4, 0x1C, 0xA3, 0x6C, 0xA9, 0x67, 0x1D, 0xAD, /*!< SHA-512 */
     0x34, 0x1F, 0xBE, 0x1B, 0x83, 0xC4, 0x40, 0x2A,
     0x47, 0x42, 0x79, 0xBB, 0x21, 0xCA, 0xF0, 0x60,
     0xE4, 0xD2, 0x6E, 0x9B, 0x70, 0x12, 0x34, 0x3F,
     0x55, 0x2C, 0x09, 0x31, 0x0A, 0x5B, 0x40, 0x21,
     0x01, 0xA8, 0x3B, 0x58, 0xE7, 0x48, 0x13, 0x1A,
     0x7E, 0xCD, 0xE1, 0xD2, 0x46, 0x10, 0x58, 0x34,
     0x49, 0x14, 0x4B, 0xAA, 0x89, 0xA9, 0xF5, 0xB1},
};

void psa_hash_test(const psa_algorithm_t alg,
                   struct test_result_t *ret)
{
    const char *msg =
        "This is my test message, please generate a hash for this.";
    /* Length of each chunk in the multipart API */
    const size_t msg_size[] = {25, 32};
    const uint32_t msg_num = sizeof(msg_size)/sizeof(msg_size[0]);
    uint32_t idx, start_idx = 0;

    psa_status_t status;
    psa_hash_operation_t handle = psa_hash_operation_init();

    /* Setup the hash object for the desired hash*/
    status = psa_hash_setup(&handle, alg);

    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_FAIL("Algorithm NOT SUPPORTED by the implementation");
            return;
        }

        TEST_FAIL("Error setting up hash operation object");
        return;
    }

    /* Update object with all the chunks of message */
    for (idx=0; idx<msg_num; idx++) {
        status = psa_hash_update(&handle,
                                 (const uint8_t *)&msg[start_idx],
                                 msg_size[idx]);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error updating the hash operation object");
            return;
        }
        start_idx += msg_size[idx];
    }

    /* Cycle until idx points to the correct index in the algorithm table */
    for (idx=0; hash_alg[idx] != alg; idx++);

    /* Finalise and verify that the hash is as expected */
    status = psa_hash_verify(&handle, hash_val[idx], PSA_HASH_LENGTH(alg));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error verifying the hash operation object");
        return;
    }

#ifdef TFM_CRYPTO_TEST_SINGLE_PART_FUNCS
    /* Do the same as above with the single shot APIs */
    status = psa_hash_compare(alg,
                              (const uint8_t *)msg, strlen(msg),
                              hash_val[idx], PSA_HASH_LENGTH(alg));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error using the single shot API");
        return;
    }
#endif

    ret->val = TEST_PASSED;
}

void psa_unsupported_mac_test(const psa_key_type_t key_type,
                              const psa_algorithm_t alg,
                              struct test_result_t *ret)
{
    psa_status_t status;
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    psa_mac_operation_t handle = PSA_MAC_OPERATION_INIT;
    psa_key_attributes_t key_attributes = PSA_KEY_ATTRIBUTES_INIT;
    const uint8_t data[] = "THIS IS MY KEY1";

    ret->val = TEST_PASSED;

    /* Setup the key policy */
    psa_set_key_usage_flags(&key_attributes, PSA_KEY_USAGE_VERIFY_HASH);
    psa_set_key_algorithm(&key_attributes, alg);
    psa_set_key_type(&key_attributes, key_type);

    /* Import key */
    status = psa_import_key(&key_attributes, data, sizeof(data), &key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing a key");
        return;
    }

    /* Setup the mac object for the unsupported mac algorithm */
    status = psa_mac_verify_setup(&handle, key_id_local, alg);
    if (status != PSA_ERROR_NOT_SUPPORTED) {
        TEST_FAIL("Should not successfully setup an unsupported MAC alg");
        /* Do not return, to ensure key is destroyed */
    }

    /* Destroy the key */
    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error destroying the key");
    }
}

static const uint8_t hmac_val[][PSA_HASH_LENGTH(PSA_ALG_SHA_512)] = {
    {0xc1, 0x9f, 0x19, 0xac, 0x05, 0x65, 0x5f, 0x02, /*!< SHA-224 */
     0x1b, 0x64, 0x32, 0xd9, 0xb1, 0x49, 0xba, 0x75,
     0x05, 0x60, 0x52, 0x4e, 0x78, 0xfa, 0x61, 0xc9,
     0x37, 0x5d, 0x7f, 0x58},
    {0x94, 0x37, 0xbe, 0xb5, 0x7f, 0x7c, 0x5c, 0xb0, /*!< SHA-256 */
     0x0a, 0x92, 0x4d, 0xd3, 0xba, 0x7e, 0xb1, 0x1a,
     0xdb, 0xa2, 0x25, 0xb2, 0x82, 0x8e, 0xdf, 0xbb,
     0x61, 0xbf, 0x91, 0x1d, 0x28, 0x23, 0x4a, 0x04},
    {0x94, 0x21, 0x9b, 0xc3, 0xd5, 0xed, 0xe6, 0xee, /*!< SHA-384 */
     0x42, 0x10, 0x5a, 0x58, 0xa4, 0x4d, 0x67, 0x87,
     0x16, 0xa2, 0xa7, 0x6c, 0x2e, 0xc5, 0x85, 0xb7,
     0x6a, 0x4c, 0x90, 0xb2, 0x73, 0xee, 0x58, 0x3c,
     0x59, 0x16, 0x67, 0xf3, 0x6f, 0x30, 0x99, 0x1c,
     0x2a, 0xf7, 0xb1, 0x5f, 0x45, 0x83, 0xf5, 0x9f},
    {0x8f, 0x76, 0xef, 0x12, 0x0b, 0x92, 0xc2, 0x06, /*!< SHA-512 */
     0xce, 0x01, 0x18, 0x75, 0x84, 0x96, 0xd9, 0x6f,
     0x23, 0x88, 0xd4, 0xf8, 0xcf, 0x79, 0xf8, 0xcf,
     0x27, 0x12, 0x9f, 0xa6, 0x7e, 0x87, 0x9a, 0x68,
     0xee, 0xe2, 0xe7, 0x1d, 0x4b, 0xf2, 0x87, 0xc0,
     0x05, 0x6a, 0xbd, 0x7f, 0x9d, 0xff, 0xaa, 0xf3,
     0x9a, 0x1c, 0xb7, 0xb7, 0xbd, 0x03, 0x61, 0xa3,
     0xa9, 0x6a, 0x5d, 0xb2, 0x81, 0xe1, 0x6f, 0x1f},
};

static const uint8_t long_key_hmac_val[PSA_HASH_LENGTH(PSA_ALG_SHA_224)] = {
    0x47, 0xa3, 0x42, 0xb1, 0x2f, 0x52, 0xd3, 0x8f, /*!< SHA-224 */
    0x1e, 0x02, 0x4a, 0x46, 0x73, 0x0b, 0x77, 0xc1,
    0x5e, 0x93, 0x31, 0xa9, 0x3e, 0xc2, 0x81, 0xb5,
    0x3d, 0x07, 0x6f, 0x31
};

void psa_mac_test(const psa_algorithm_t alg,
                  const uint8_t *key,
                  size_t key_bits,
                  struct test_result_t *ret)
{
    const char *msg =
        "This is my test message, please generate a hmac for this.";
    /* Length of each chunk in the multipart API */
    const size_t msg_size[] = {25, 32};
    const uint32_t msg_num = sizeof(msg_size)/sizeof(msg_size[0]);
    uint32_t idx, start_idx = 0;
    uint8_t *hmac_res;

    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    psa_key_type_t key_type = PSA_KEY_TYPE_HMAC;
    psa_status_t status;
    psa_mac_operation_t handle = psa_mac_operation_init();
    psa_key_attributes_t key_attributes = psa_key_attributes_init();
    psa_key_attributes_t retrieved_attributes = psa_key_attributes_init();
    psa_key_usage_t usage = PSA_KEY_USAGE_VERIFY_HASH;

    ret->val = TEST_PASSED;

    /* Setup the key policy */
    psa_set_key_usage_flags(&key_attributes, usage);
    psa_set_key_algorithm(&key_attributes, alg);
    psa_set_key_type(&key_attributes, key_type);

    /* Import key */
    status = psa_import_key(&key_attributes, key, PSA_BITS_TO_BYTES(key_bits),
                            &key_id_local);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing a key");
        return;
    }

    status = psa_get_key_attributes(key_id_local, &retrieved_attributes);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error getting key metadata");
        goto destroy_key_mac;
    }

    if (psa_get_key_bits(&retrieved_attributes) != key_bits) {
        TEST_FAIL("The number of key bits is different from expected");
        goto destroy_key_mac;
    }

    if (psa_get_key_type(&retrieved_attributes) != key_type) {
        TEST_FAIL("The type of the key is different from expected");
        goto destroy_key_mac;
    }

    psa_reset_key_attributes(&retrieved_attributes);

    /* Setup the mac object for hmac */
    status = psa_mac_verify_setup(&handle, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_FAIL("Algorithm NOT SUPPORTED by the implementation");
            goto destroy_key_mac;
        }

        TEST_FAIL("Error setting up mac operation object");
        goto destroy_key_mac;
    }

    /* Update object with all the chunks of message */
    for (idx=0; idx<msg_num; idx++) {
        status = psa_mac_update(&handle,
                                (const uint8_t *)&msg[start_idx],
                                msg_size[idx]);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error during mac operation");
            goto destroy_key_mac;
        }
        start_idx += msg_size[idx];
    }

    /* Cycle until idx points to the correct index in the algorithm table */
    for (idx=0; hash_alg[idx] != PSA_ALG_HMAC_GET_HASH(alg); idx++);

    if (key_bits == BIT_SIZE_TEST_LONG_KEY) {
        hmac_res = (uint8_t *)long_key_hmac_val;
    } else {
        hmac_res = (uint8_t *)hmac_val[idx];
    }

    /* Finalise and verify the mac value */
    status = psa_mac_verify_finish(&handle, hmac_res,
                                   PSA_HASH_LENGTH(PSA_ALG_HMAC_GET_HASH(alg)));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error during finalising the mac operation");
        goto destroy_key_mac;
    }

#ifdef TFM_CRYPTO_TEST_SINGLE_PART_FUNCS
    /* Do the same as above with the single shot APIs */
    status = psa_mac_verify(key_id_local, alg,
                            (const uint8_t *)msg, strlen(msg),
                            hmac_res,
                            PSA_HASH_LENGTH(PSA_ALG_HMAC_GET_HASH(alg)));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error using the single shot API");
    }
#endif

destroy_key_mac:
    /* Destroy the key */
    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error destroying the key");
    }
}

void psa_aead_test(const psa_key_type_t key_type,
                   const psa_algorithm_t alg,
                   const uint8_t *key,
                   size_t key_bits,
                   struct test_result_t *ret)
{
    psa_aead_operation_t encop = psa_aead_operation_init();
    psa_aead_operation_t decop = psa_aead_operation_init();
    psa_status_t status = PSA_SUCCESS;
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    const size_t nonce_length = 12;
    const uint8_t nonce[] = "01234567890";
    const uint8_t plain_text[MAX_PLAIN_DATA_SIZE_AEAD] =
        "This is my plaintext message and it's made of 68 characters...!1234";
    const uint8_t associated_data[] =
        "This is my associated data to authenticate";
    uint8_t decrypted_data[MAX_PLAIN_DATA_SIZE_AEAD] = {0};
    uint8_t encrypted_data[ENC_DEC_BUFFER_SIZE_AEAD] = {0};
    size_t encrypted_data_length = 0, decrypted_data_length = 0;
    size_t total_output_length = 0, total_encrypted_length = 0;
    uint32_t comp_result;
    psa_key_attributes_t key_attributes = psa_key_attributes_init();
    psa_key_attributes_t retrieved_attributes = psa_key_attributes_init();
    psa_key_usage_t usage = (PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
#ifdef TFM_CRYPTO_TEST_SINGLE_PART_FUNCS
    uint8_t encrypted_data_single_shot[ENC_DEC_BUFFER_SIZE_AEAD] = {0};
#endif

    /* Variables required for multipart operations */
    uint8_t *tag = &encrypted_data[MAX_PLAIN_DATA_SIZE_AEAD];
    size_t tag_size = PSA_AEAD_TAG_LENGTH(key_type,
                                          psa_get_key_bits(&key_attributes),
                                          alg);
    size_t tag_length = 0;

    ret->val = TEST_PASSED;

    if (sizeof(encop) != sizeof(uint32_t)) {
        TEST_FAIL("The test client is not picking up the client side definitions");
        return;
    }

    /* Setup the key policy */
    psa_set_key_usage_flags(&key_attributes, usage);
    psa_set_key_algorithm(&key_attributes, alg);
    psa_set_key_type(&key_attributes, key_type);

    /* Import a key */
    status = psa_import_key(&key_attributes, key, PSA_BITS_TO_BYTES(key_bits),
                            &key_id_local);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing a key");
        return;
    }

    status = psa_get_key_attributes(key_id_local, &retrieved_attributes);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error getting key metadata");
        goto destroy_key_aead;
    }

    if (psa_get_key_bits(&retrieved_attributes) != key_bits) {
        TEST_FAIL("The number of key bits is different from expected");
        goto destroy_key_aead;
    }

    if (psa_get_key_type(&retrieved_attributes) != key_type) {
        TEST_FAIL("The type of the key is different from expected");
        goto destroy_key_aead;
    }

    psa_reset_key_attributes(&retrieved_attributes);

#ifdef TFM_CRYPTO_TEST_SINGLE_PART_FUNCS
    /* Perform AEAD encryption */
    status = psa_aead_encrypt(key_id_local, alg, nonce, nonce_length,
                              associated_data,
                              sizeof(associated_data),
                              plain_text,
                              sizeof(plain_text),
                              encrypted_data,
                              sizeof(encrypted_data),
                              &encrypted_data_length);

    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_FAIL("Algorithm NOT SUPPORTED by the implementation");
            goto destroy_key_aead;
        }

        TEST_FAIL("Error performing AEAD encryption");
        goto destroy_key_aead;
    }

    if (encrypted_data_length
        != PSA_AEAD_ENCRYPT_OUTPUT_SIZE(key_type, alg, sizeof(plain_text))) {
        TEST_FAIL("Encrypted data length is different than expected");
        goto destroy_key_aead;
    }

    /* Store a copy of the encrypted data for later checking it against
     * multipart results
     */
    if (encrypted_data_length > ENC_DEC_BUFFER_SIZE_AEAD) {
        TEST_FAIL("Encrypted data length is above the maximum expected value");
        goto destroy_key_aead;
    }
    memcpy(encrypted_data_single_shot, encrypted_data, encrypted_data_length);

    /* Perform AEAD decryption */
    status = psa_aead_decrypt(key_id_local, alg, nonce, nonce_length,
                              associated_data,
                              sizeof(associated_data),
                              encrypted_data,
                              encrypted_data_length,
                              decrypted_data,
                              sizeof(decrypted_data),
                              &decrypted_data_length);

    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_FAIL("Algorithm NOT SUPPORTED by the implementation");
        } else {
            TEST_FAIL("Error performing AEAD decryption");
        }

        goto destroy_key_aead;
    }

    if (sizeof(plain_text) != decrypted_data_length) {
        TEST_FAIL("Decrypted data length is different from plain text");
        goto destroy_key_aead;
    }

    /* Check that the decrypted data is the same as the original data */
    comp_result = memcmp(plain_text, decrypted_data, sizeof(plain_text));
    if (comp_result != 0) {
        TEST_FAIL("Decrypted data doesn't match with plain text");
        goto destroy_key_aead;
    }
#endif /* TFM_CRYPTO_TEST_SINGLE_PART_FUNCS */

    /* Setup the encryption object */
    status = psa_aead_encrypt_setup(&encop, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        /* Implementations using the PSA Crypto Driver APIs, that don't
         * support multipart API flows, will return PSA_ERROR_NOT_SUPPORTED
         * when calling psa_aead_encrypt_setup(). In this case, it's fine
         * just to skip the multipart APIs test flow from this point on
         */
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_LOG("psa_aead_encrypt_setup(): Algorithm NOT SUPPORTED by"\
                     " the implementation - skip multipart API flow\r\n");
        } else {
            TEST_FAIL("Error setting up encryption object");
        }
        goto destroy_key_aead;
    }

    /* Set lengths */
    status = psa_aead_set_lengths(&encop,
                                  sizeof(associated_data),
                                  sizeof(plain_text));
    if (status != PSA_SUCCESS) {
        /* Implementations using the mbed TLS _ALT APIs, that don't support
         * multipart API flows in CCM mode, will return PSA_ERROR_NOT_SUPPORTED
         * when calling psa_aead_set_lengths(). In this case, it's fine just
         * to skip the multipart APIs test flow from this point on
         */
        if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_CCM
            && status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_LOG("psa_aead_set_lengths(): Algorithm NOT SUPPORTED by the "\
                     "implementation - skip multipart API flow\r\n");
        } else {
            TEST_FAIL("Error setting lengths");
        }
        status = psa_aead_abort(&encop);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error aborting the operation");
        }
        goto destroy_key_aead;
    }

    /* Set nonce */
    status = psa_aead_set_nonce(&encop, nonce, nonce_length);
    if (status != PSA_SUCCESS) {
        /* Implementations using the mbed TLS _ALT APIs, that don't support
         * multipart API flows in GCM, will return PSA_ERROR_NOT_SUPPORTED when
         * calling psa_aead_set_nonce(). In this case, it's fine just to skip
         * the multipart APIs test flow from this point on
         */
        if (PSA_ALG_AEAD_WITH_DEFAULT_LENGTH_TAG(alg) == PSA_ALG_GCM
            && status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_LOG("psa_aead_set_nonce(): Algorithm NOT SUPPORTED by the "\
                     "implementation - skip multipart API flow\r\n");
        } else {
            TEST_FAIL("Error setting nonce");
        }
        status = psa_aead_abort(&encop);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error aborting the operation");
        }
        goto destroy_key_aead;
    }

    /* Update additional data */
    status = psa_aead_update_ad(&encop,
                                associated_data,
                                sizeof(associated_data));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error updating additional data");
        status = psa_aead_abort(&encop);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error aborting the operation");
        }
        goto destroy_key_aead;
    }

    /* Encrypt one chunk of information at a time */
    for (size_t i = 0; i <= sizeof(plain_text)/BYTE_SIZE_CHUNK; i++) {

        size_t size_to_encrypt =
            (sizeof(plain_text) - i*BYTE_SIZE_CHUNK) > BYTE_SIZE_CHUNK ?
            BYTE_SIZE_CHUNK : (sizeof(plain_text) - i*BYTE_SIZE_CHUNK);

        status = psa_aead_update(&encop,
                                 plain_text + i*BYTE_SIZE_CHUNK,
                                 size_to_encrypt,
                                 encrypted_data + total_encrypted_length,
                                 sizeof(encrypted_data) -
                                                      total_encrypted_length,
                                 &encrypted_data_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error encrypting one chunk of information");
            status = psa_aead_abort(&encop);
            if (status != PSA_SUCCESS) {
                TEST_FAIL("Error aborting the operation");
            }
            goto destroy_key_aead;
        }
        total_encrypted_length += encrypted_data_length;
    }

    /* Finish the aead operation */
    status = psa_aead_finish(&encop,
                             encrypted_data + total_encrypted_length,
                             sizeof(encrypted_data) - total_encrypted_length,
                             &encrypted_data_length,
                             tag,
                             tag_size,
                             &tag_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error finalising the AEAD operation");
        status = psa_aead_abort(&encop);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error aborting the operation");
        }
        goto destroy_key_aead;
    }
    total_encrypted_length += encrypted_data_length;

#ifdef TFM_CRYPTO_TEST_SINGLE_PART_FUNCS
    /* Compare tag between single part and multipart case */
    comp_result = memcmp(
                      &encrypted_data_single_shot[total_encrypted_length],
                      tag, tag_length);
    if (comp_result != 0) {
        TEST_FAIL("Single shot tag does not match with multipart");
        goto destroy_key_aead;
    }

    /* Compare encrypted data between single part and multipart case */
    comp_result = memcmp(
                      encrypted_data_single_shot,
                      encrypted_data, total_encrypted_length);
    if (comp_result != 0) {
        TEST_FAIL("Single shot encrypted data does not match with multipart");
        goto destroy_key_aead;
    }
#endif /* TFM_CRYPTO_TEST_SINGLE_PART_FUNCS */

    /* Setup up the decryption object */
    status = psa_aead_decrypt_setup(&decop, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting up AEAD object");
        goto destroy_key_aead;
    }

    /* Set lengths */
    status = psa_aead_set_lengths(&decop,
                                  sizeof(associated_data),
                                  sizeof(plain_text));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting lengths");
        status = psa_aead_abort(&decop);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error aborting the operation");
        }
        goto destroy_key_aead;
    }

    /* Set nonce */
    status = psa_aead_set_nonce(&decop, nonce, nonce_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error setting nonce");
        status = psa_aead_abort(&decop);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error aborting the operation");
        }
        goto destroy_key_aead;
    }

    /* Update additional data */
    status = psa_aead_update_ad(&decop,
                                associated_data,
                                sizeof(associated_data));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error updating additional data");
        status = psa_aead_abort(&decop);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error aborting the operation");
        }
        goto destroy_key_aead;
    }

    /* Decrypt */
    for (size_t i = 0; i <= total_encrypted_length/BYTE_SIZE_CHUNK; i++) {

        size_t size_to_decrypt =
            (total_encrypted_length - i*BYTE_SIZE_CHUNK) > BYTE_SIZE_CHUNK ?
            BYTE_SIZE_CHUNK : (total_encrypted_length - i*BYTE_SIZE_CHUNK);

        status = psa_aead_update(&decop,
                                 encrypted_data + i*BYTE_SIZE_CHUNK,
                                 size_to_decrypt,
                                 decrypted_data + total_output_length,
                                 sizeof(decrypted_data)
                                                       - total_output_length,
                                 &decrypted_data_length);

        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error during decryption");
            status = psa_aead_abort(&decop);
            if (status != PSA_SUCCESS) {
                TEST_FAIL("Error aborting the operation");
            }
            goto destroy_key_aead;
        }
        total_output_length += decrypted_data_length;
    }

    /* Verify the AEAD operation */
    status = psa_aead_verify(&decop,
                             decrypted_data + total_output_length,
                             sizeof(decrypted_data) - total_output_length,
                             &decrypted_data_length,
                             tag,
                             tag_size);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error verifying the AEAD operation");
        status = psa_aead_abort(&decop);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error aborting the operation");
        }
        goto destroy_key_aead;
    }
    total_output_length += decrypted_data_length;

    if (total_output_length != sizeof(plain_text)) {
        TEST_FAIL("Total decrypted length does not match size of plain text");
        status = psa_aead_abort(&decop);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Error aborting the operation");
        }
        goto destroy_key_aead;
    }

    /* Check that the decrypted data is the same as the original data */
    comp_result = memcmp(plain_text, decrypted_data, sizeof(plain_text));
    if (comp_result != 0) {
        TEST_FAIL("Decrypted data doesn't match with plain text");
    }

destroy_key_aead:
    /* Destroy the key */
    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error destroying a key");
    }
}

/*
 * The list of available AES cipher/AEAD mode for test.
 * Not all the modes can be available in some use cases and configurations.
 */
static const psa_algorithm_t test_aes_mode_array[] = {
#ifdef TFM_CRYPTO_TEST_ALG_CBC
    PSA_ALG_CBC_NO_PADDING,
#endif
#ifdef TFM_CRYPTO_TEST_ALG_CCM
    PSA_ALG_CCM,
#endif
#ifdef TFM_CRYPTO_TEST_ALG_CFB
    PSA_ALG_CFB,
#endif
#ifdef TFM_CRYPTO_TEST_ALG_ECB
    PSA_ALG_ECB_NO_PADDING,
#endif
#ifdef TFM_CRYPTO_TEST_ALG_CTR
    PSA_ALG_CTR,
#endif
#ifdef TFM_CRYPTO_TEST_ALG_OFB
    PSA_ALG_OFB,
#endif
#ifdef TFM_CRYPTO_TEST_ALG_GCM
    PSA_ALG_GCM,
#endif
    /* In case no AES algorithm is available */
    PSA_ALG_VENDOR_FLAG,
};

/* Number of available AES cipher modes */
#define NR_TEST_AES_MODE(t) ((sizeof(t) / sizeof(t[0])) - 1)

void psa_invalid_key_length_test(struct test_result_t *ret)
{
    psa_status_t status;
    psa_key_attributes_t key_attributes = psa_key_attributes_init();
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    const uint8_t data[19] = {0};

    if (NR_TEST_AES_MODE(test_aes_mode_array) < 1) {
        TEST_FAIL("A cipher mode in AES is required in current test case");
        return;
    }

    /* Setup the key policy */
    psa_set_key_usage_flags(&key_attributes, PSA_KEY_USAGE_ENCRYPT);
    psa_set_key_algorithm(&key_attributes, test_aes_mode_array[0]);
    psa_set_key_type(&key_attributes, PSA_KEY_TYPE_AES);

    /* AES does not support 152-bit keys */
    status = psa_import_key(&key_attributes, data, sizeof(data), &key_id_local);
    if (status != PSA_ERROR_INVALID_ARGUMENT) {
        TEST_FAIL("Should not successfully import with an invalid key length");
        return;
    }

    ret->val = TEST_PASSED;
}

void psa_policy_key_interface_test(struct test_result_t *ret)
{
    psa_algorithm_t alg = test_aes_mode_array[0];
    psa_algorithm_t alg_out;
    psa_key_lifetime_t lifetime = PSA_KEY_LIFETIME_VOLATILE;
    psa_key_lifetime_t lifetime_out;
    psa_key_attributes_t key_attributes = psa_key_attributes_init();
    psa_key_usage_t usage = PSA_KEY_USAGE_EXPORT;
    psa_key_usage_t usage_out;

    if (NR_TEST_AES_MODE(test_aes_mode_array) < 1) {
        TEST_FAIL("A cipher mode in AES is required in current test case");
        return;
    }

    /* Verify that initialised policy forbids all usage */
    usage_out = psa_get_key_usage_flags(&key_attributes);
    if (usage_out != 0) {
        TEST_FAIL("Unexpected usage value");
        return;
    }

    alg_out = psa_get_key_algorithm(&key_attributes);
    if (alg_out != 0) {
        TEST_FAIL("Unexpected algorithm value");
        return;
    }

    /* Set the key policy values */
    psa_set_key_usage_flags(&key_attributes, usage);
    psa_set_key_algorithm(&key_attributes, alg);

    /* Check that the key policy has the correct usage */
    usage_out = psa_get_key_usage_flags(&key_attributes);
    if (usage_out != usage) {
        TEST_FAIL("Unexpected usage value");
        return;
    }

    /* Check that the key policy has the correct algorithm */
    alg_out = psa_get_key_algorithm(&key_attributes);
    if (alg_out != alg) {
        TEST_FAIL("Unexpected algorithm value");
        return;
    }

    /* Check the key has the correct key lifetime */
    lifetime_out = psa_get_key_lifetime(&key_attributes);

    if (lifetime_out != lifetime) {
        TEST_FAIL("Unexpected key lifetime value");
        return;
    }

    ret->val = TEST_PASSED;
}

void psa_policy_invalid_policy_usage_test(struct test_result_t *ret)
{
    psa_status_t status;
    psa_algorithm_t alg = PSA_ALG_NONE, not_permit_alg = PSA_ALG_NONE;
    psa_cipher_operation_t handle = psa_cipher_operation_init();
    psa_key_attributes_t key_attributes = psa_key_attributes_init();
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    psa_key_usage_t usage = (PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    size_t data_len;
    const uint8_t data[] = "THIS IS MY KEY1";
    uint8_t data_out[sizeof(data)];
    uint8_t i, j;

    ret->val = TEST_PASSED;

    if (NR_TEST_AES_MODE(test_aes_mode_array) < 2) {
        TEST_LOG("Two cipher modes are required. Skipping...\r\n");
        return;
    }

    /*
     * Search for two modes for test. Both modes should be Cipher algorithms.
     * Otherwise, cipher setup may fail before policy permission check.
     */
    for (i = 0; i < NR_TEST_AES_MODE(test_aes_mode_array); i++) {
        if (PSA_ALG_IS_CIPHER(test_aes_mode_array[i])) {
            alg = test_aes_mode_array[i];
            break;
        }
    }

    for (j = 0; j < NR_TEST_AES_MODE(test_aes_mode_array); j++) {
        if (PSA_ALG_IS_CIPHER(test_aes_mode_array[j]) && j != i) {
            not_permit_alg = test_aes_mode_array[j];
            break;
        }
    }

    if (alg == PSA_ALG_NONE || not_permit_alg == PSA_ALG_NONE) {
        TEST_LOG("alg: %x, not_permit_alg: %x. Unable to find two Cipher algs. Skipping...\r\n", alg, not_permit_alg);
        return;
    }

    /* Setup the key policy */
    psa_set_key_usage_flags(&key_attributes, usage);
    psa_set_key_algorithm(&key_attributes, alg);
    psa_set_key_type(&key_attributes, PSA_KEY_TYPE_AES);

    /* Import the key after having set the policy */
    status = psa_import_key(&key_attributes, data, sizeof(data), &key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to import a key");
        return;
    }

    /* Setup a cipher permitted by the key policy */
    status = psa_cipher_encrypt_setup(&handle, key_id_local, alg);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to setup cipher operation");
        goto destroy_key;
    }

    status = psa_cipher_abort(&handle);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to abort cipher operation");
        goto destroy_key;
    }

    /* Attempt to setup a cipher with an alg not permitted by the policy */
    status = psa_cipher_encrypt_setup(&handle, key_id_local, not_permit_alg);
    if (status != PSA_ERROR_NOT_PERMITTED) {
        TEST_FAIL("Was able to setup cipher operation with wrong alg");
        goto destroy_key;
    }

    /* Attempt to export the key, which is forbidden by the key policy */
    status = psa_export_key(key_id_local, data_out,
                            sizeof(data_out), &data_len);
    if (status != PSA_ERROR_NOT_PERMITTED) {
        TEST_FAIL("Should not be able to export key without correct usage");
        goto destroy_key;
    }

destroy_key:
    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to destroy key");
    }
}

void psa_persistent_key_test(psa_key_id_t key_id, struct test_result_t *ret)
{
    psa_status_t status;
    int comp_result;
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    psa_algorithm_t alg = test_aes_mode_array[0];
    psa_key_usage_t usage = PSA_KEY_USAGE_EXPORT;
    psa_key_attributes_t key_attributes = PSA_KEY_ATTRIBUTES_INIT;
    size_t data_len;
    const uint8_t data[] = "THIS IS MY KEY1";
    uint8_t data_out[sizeof(data)] = {0};

    if (NR_TEST_AES_MODE(test_aes_mode_array) < 1) {
        TEST_FAIL("A cipher mode in AES is required in current test case");
        return;
    }

    /* Setup the key attributes with a key ID to create a persistent key */
    psa_set_key_id(&key_attributes, key_id);
    psa_set_key_usage_flags(&key_attributes, usage);
    psa_set_key_algorithm(&key_attributes, alg);
    psa_set_key_type(&key_attributes, PSA_KEY_TYPE_AES);

    /* Import key data to create the persistent key */
    status = psa_import_key(&key_attributes, data, sizeof(data), &key_id_local);
#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to import a key");
        return;
    }
#else
    if (status != PSA_ERROR_NOT_SUPPORTED) {
        TEST_FAIL("When ITS partition is not enabled, \
                   import should return NOT_SUPPORTED");
        return;
    }
    TEST_LOG("psa_import_key(): ITS partition is not enabled - skip\r\n");
    ret->val = TEST_PASSED;
    return;
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

    if (key_id_local != key_id) {
        TEST_FAIL("After importing key_id_local and key_id must match");
        return;
    }

    /* Close the persistent key through the key ID */
    status = psa_close_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to close a persistent key");
        return;
    }

    /* Open the previsously-created persistent key */
    status = psa_open_key(key_id, &key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to open a persistent key");
        return;
    }

    /* Export the persistent key */
    status = psa_export_key(key_id_local, data_out,
                            sizeof(data_out), &data_len);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to export a persistent key");
        return;
    }

    if (data_len != sizeof(data)) {
        TEST_FAIL("Number of bytes of exported key different from expected");
        return;
    }

    /* Check that the exported key is the same as the imported one */
    comp_result = memcmp(data_out, data, sizeof(data));
    if (comp_result != 0) {
        TEST_FAIL("Exported key does not match the imported key");
        return;
    }

    /* Destroy the persistent key */
    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to destroy a persistent key");
        return;
    }

    ret->val = TEST_PASSED;
}

#define KEY_DERIV_OUTPUT_LEN       32
#define KEY_DERIV_SECRET_LEN       16
#define KEY_DERIV_PEER_LEN         16
#define KEY_DERIV_LABEL_INFO_LEN   8
#define KEY_DERIV_SEED_SALT_LEN    8
#define KEY_DERIV_RAW_MAX_PEER_LEN 100
#define KEY_DERIV_RAW_OUTPUT_LEN   48

/* Pair of ECC test keys that are generated using openssl with the following
 * parameters:
 *
 * openssl ecparam -outform der -out test_prime256v1 -name prime256v1 -genkey
 *
 */
/* An example of a 32 bytes / 256 bits ECDSA private key */
static const uint8_t ecdsa_private_key[] = {
    0xED, 0x8F, 0xAA, 0x23, 0xE2, 0x8B, 0x1F, 0x51,
    0x63, 0x4F, 0x8E, 0xC9, 0xDC, 0x24, 0x92, 0x0F,
    0x3D, 0xA1, 0x7B, 0x47, 0x68, 0x38, 0xE3, 0x0D,
    0x10, 0x4A, 0x6D, 0xA7, 0x2D, 0x48, 0xA4, 0x18
};
/* Corresponding public key in uncompressed form, i.e. 0x04 X Y, and
 * encoded as per RFC 5480. This is obtained by running the following
 * command:
 *
 *   openssl ec -in private_key.der -inform der -pubout -out public_key.der -outform der
 *
 * where the private_key.der contains the DER encoding of the private key contained in
 * ecdsa_private_key above
 */
static const uint8_t ecdsa_public_key[] = {
    0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86,
    0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03,
    0x42, 0x00, 0x04, 0x41, 0xC6, 0xFC, 0xC5, 0xA4,
    0xBB, 0x70, 0x45, 0xA7, 0xB2, 0x5E, 0x50, 0xB3,
    0x2E, 0xD0, 0x2A, 0x8D, 0xA8, 0x8E, 0x1B, 0x34,
    0xC2, 0x71, 0x57, 0x38, 0x5C, 0x45, 0xAB, 0xF2,
    0x51, 0x7B, 0x17, 0x5A, 0xC5, 0x05, 0xA9, 0x9E,
    0x4B, 0x7D, 0xDD, 0xD7, 0xBF, 0xBB, 0x45, 0x51,
    0x92, 0x7D, 0x33, 0x33, 0x8B, 0x1B, 0x70, 0x5A,
    0xFD, 0x2B, 0xF2, 0x7A, 0xA4, 0xBD, 0x37, 0x50,
    0xED, 0x34, 0x9F
};

/* Associated encoded signature using the private key above on the reference
 * input message, as described in RFC 5480:
 *
 *   message = "This is the message that I would like to sign"
 *
 * using PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), i.e.
 *
 *   Ecdsa-Sig-Value  ::=  SEQUENCE  {
 *        r     INTEGER,
 *        s     INTEGER  }
 */
static const uint8_t reference_encoded_r_s[] = {
    0x30, 0x45, 0x02, 0x20, 0x6b, 0xdc, 0xc6, 0xd5,
    0xf5, 0xdc, 0xab, 0xc2, 0x52, 0xb6, 0xa0, 0xcd,
    0x12, 0x9e, 0xfc, 0x3e, 0x86, 0x24, 0x7d, 0xf1,
    0xbd, 0x7b, 0xe9, 0x76, 0xbd, 0xb5, 0x99, 0x82,
    0x44, 0xd4, 0xa5, 0x0c, 0x02, 0x21, 0x00, 0xa6,
    0x25, 0x7b, 0x3b, 0x2a, 0x2d, 0xea, 0xaa, 0x43,
    0xbc, 0x3a, 0xc7, 0x89, 0xdc, 0x1b, 0x52, 0xe0,
    0xd2, 0xb6, 0xbd, 0x8c, 0x5d, 0x5e, 0xf3, 0x32,
    0xe7, 0x32, 0x65, 0xbd, 0x7b, 0xcb, 0x06,
};

/* Buffer to hold the peer key of the key agreement process */
static uint8_t raw_agreement_peer_key[KEY_DERIV_RAW_MAX_PEER_LEN];
static uint8_t raw_agreement_output_buffer[KEY_DERIV_RAW_OUTPUT_LEN];
static uint8_t key_deriv_secret[KEY_DERIV_SECRET_LEN];
static uint8_t key_deriv_label_info[KEY_DERIV_LABEL_INFO_LEN];
static uint8_t key_deriv_seed_salt[KEY_DERIV_SEED_SALT_LEN];
/* Reference data for the shared secret obtained by multiplying the private
 * key contained in ecdsa_private_key and its associated public key, and
 * taking the value of x of the resulting point as the shared secret
 */
static const uint8_t raw_agreement_reference_secret[KEY_DERIV_RAW_OUTPUT_LEN] = {
    0x09, 0x3f, 0x86, 0x0d, 0x68, 0x0b, 0xb4, 0xe0,
    0x72, 0xa4, 0x23, 0x50, 0xd6, 0x67, 0x06, 0x06,
    0x3c, 0x8c, 0xb7, 0xb6, 0x49, 0xb6, 0x49, 0x1f,
    0x7b, 0xcb, 0x6d, 0x36, 0xe6, 0x63, 0x7e, 0x0c,
};

void psa_key_agreement_test(psa_algorithm_t deriv_alg,
                            struct test_result_t *ret)
{
    psa_status_t status;
    psa_key_type_t key_type;
    psa_key_id_t input_key_id_local = PSA_KEY_ID_NULL;
    psa_key_attributes_t input_key_attr = PSA_KEY_ATTRIBUTES_INIT;
    size_t raw_agreement_output_size = 0;
    size_t public_key_length = 0;

    if (!PSA_ALG_IS_RAW_KEY_AGREEMENT(deriv_alg)) {
        TEST_FAIL("Unsupported key agreement algorithm");
        return;
    }

    psa_set_key_usage_flags(&input_key_attr, PSA_KEY_USAGE_DERIVE);
    psa_set_key_algorithm(&input_key_attr, deriv_alg);
    key_type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);
    psa_set_key_type(&input_key_attr, key_type);

    status = psa_import_key(&input_key_attr, ecdsa_private_key,
                            sizeof(ecdsa_private_key), &input_key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing the private key");
        return;
    }

    /* For simplicity, as the peer key use the public part of private key */
    status = psa_export_public_key(input_key_id_local,
                                   raw_agreement_peer_key,
                                   KEY_DERIV_RAW_MAX_PEER_LEN,
                                   &public_key_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error extracting the public key as peer key");
        goto destroy_key;
    }

    status = psa_raw_key_agreement(deriv_alg,
                                   input_key_id_local,
                                   raw_agreement_peer_key,
                                   public_key_length,
                                   raw_agreement_output_buffer,
                                   KEY_DERIV_RAW_OUTPUT_LEN,
                                   &raw_agreement_output_size);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error performing single step raw key agreement");
        goto destroy_key;
    }

    if (raw_agreement_output_size != sizeof(ecdsa_private_key)) {
        TEST_FAIL("Agreed key size is different than expected!");
        goto destroy_key;
    }

    /* Check that the shared secret matches the reference provided */
    if (memcmp(raw_agreement_output_buffer,
               raw_agreement_reference_secret,
               sizeof(raw_agreement_reference_secret)) != 0) {
        TEST_FAIL("Computed shared secret does not match the reference!");
        goto destroy_key;
    }

    ret->val = TEST_PASSED;

destroy_key:
    psa_destroy_key(input_key_id_local);

    return;
}

void psa_key_derivation_test(psa_algorithm_t deriv_alg,
                             struct test_result_t *ret)
{
    psa_key_id_t input_key_id_local = PSA_KEY_ID_NULL,
                 output_key_id_local = PSA_KEY_ID_NULL;
    psa_key_attributes_t input_key_attr = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_attributes_t output_key_attr = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_derivation_operation_t deriv_ops;
    psa_status_t status;
    uint8_t counter = 0xA5;
    psa_key_type_t key_type;

    /* Prepare the parameters */
    memset(key_deriv_secret, counter, KEY_DERIV_SECRET_LEN);
    memset(key_deriv_label_info, counter++, KEY_DERIV_LABEL_INFO_LEN);
    memset(key_deriv_seed_salt, counter++, KEY_DERIV_SEED_SALT_LEN);

    deriv_ops = psa_key_derivation_operation_init();

    psa_set_key_usage_flags(&input_key_attr, PSA_KEY_USAGE_DERIVE);
    psa_set_key_algorithm(&input_key_attr, deriv_alg);
    key_type = PSA_KEY_TYPE_DERIVE;
    psa_set_key_type(&input_key_attr, key_type);
    status = psa_import_key(&input_key_attr, key_deriv_secret,
                            KEY_DERIV_SECRET_LEN, &input_key_id_local);

    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to import secret");
        return;
    }

    status = psa_key_derivation_setup(&deriv_ops, deriv_alg);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to setup derivation operation");
        goto destroy_key;
    }

    if (PSA_ALG_IS_TLS12_PRF(deriv_alg) ||
        PSA_ALG_IS_TLS12_PSK_TO_MS(deriv_alg)) {
        status = psa_key_derivation_input_bytes(&deriv_ops,
                                                PSA_KEY_DERIVATION_INPUT_SEED,
                                                key_deriv_seed_salt,
                                                KEY_DERIV_SEED_SALT_LEN);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Failed to input seed");
            goto deriv_abort;
        }

        status = psa_key_derivation_input_key(&deriv_ops,
                                              PSA_KEY_DERIVATION_INPUT_SECRET,
                                              input_key_id_local);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Failed to input key");
            goto deriv_abort;
        }

        status = psa_key_derivation_input_bytes(&deriv_ops,
                                                PSA_KEY_DERIVATION_INPUT_LABEL,
                                                key_deriv_label_info,
                                                KEY_DERIV_LABEL_INFO_LEN);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Failed to input label");
            goto deriv_abort;
        }
    } else if (PSA_ALG_IS_HKDF(deriv_alg)) {
        status = psa_key_derivation_input_bytes(&deriv_ops,
                                                PSA_KEY_DERIVATION_INPUT_SALT,
                                                key_deriv_seed_salt,
                                                KEY_DERIV_SEED_SALT_LEN);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Failed to input salt");
            goto deriv_abort;
        }

        status = psa_key_derivation_input_key(&deriv_ops,
                                              PSA_KEY_DERIVATION_INPUT_SECRET,
                                              input_key_id_local);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Failed to input key");
            goto deriv_abort;
        }

        status = psa_key_derivation_input_bytes(&deriv_ops,
                                                PSA_KEY_DERIVATION_INPUT_INFO,
                                                key_deriv_label_info,
                                                KEY_DERIV_LABEL_INFO_LEN);
        if (status != PSA_SUCCESS) {
            TEST_FAIL("Failed to input info");
            goto deriv_abort;
        }
    } else {
        TEST_FAIL("Unsupported derivation algorithm");
        goto deriv_abort;
    }

    if (NR_TEST_AES_MODE(test_aes_mode_array) < 1) {
        TEST_LOG("No AES algorithm to verify. Output raw data instead\r\n");
        psa_set_key_type(&output_key_attr, PSA_KEY_TYPE_RAW_DATA);
    } else {
        psa_set_key_usage_flags(&output_key_attr, PSA_KEY_USAGE_ENCRYPT);
        psa_set_key_algorithm(&output_key_attr, test_aes_mode_array[0]);
        psa_set_key_type(&output_key_attr, PSA_KEY_TYPE_AES);
    }
    psa_set_key_bits(&output_key_attr,
                     PSA_BYTES_TO_BITS(KEY_DERIV_OUTPUT_LEN));

    status = psa_key_derivation_output_key(&output_key_attr, &deriv_ops,
                                           &output_key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failed to output key");
        goto deriv_abort;
    }

    ret->val = TEST_PASSED;

deriv_abort:
    psa_key_derivation_abort(&deriv_ops);
destroy_key:
    psa_destroy_key(input_key_id_local);
    if (output_key_id_local) {
        psa_destroy_key(output_key_id_local);
    }

    return;
}

/* Key is generated using psa_generate_key then psa_export_key, using the
 * key attributes given in the test function */
static const uint8_t rsa_key_pair[] = {
0x30, 0x82, 0x02, 0x5d, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0xd1, 0xd9,
0xa2, 0x11, 0x6f, 0x2c, 0x56, 0x3b, 0x10, 0x01, 0xb8, 0x96, 0xa1, 0x8c, 0x79,
0xfa, 0x3c, 0x9a, 0x45, 0xe8, 0x03, 0xdb, 0x45, 0x66, 0xd1, 0x75, 0xf3, 0xd8,
0x3a, 0x10, 0xb3, 0x0f, 0x6c, 0x61, 0x78, 0x05, 0x21, 0xd6, 0x9a, 0x32, 0x25,
0xf0, 0x04, 0xd8, 0x1b, 0xf7, 0xa4, 0xd7, 0x0d, 0xd6, 0x00, 0xa4, 0x95, 0x92,
0xeb, 0x24, 0x1e, 0x37, 0xb5, 0x22, 0x9e, 0x7d, 0x22, 0x31, 0x6d, 0xd8, 0xf3,
0x2e, 0x4f, 0x8a, 0x94, 0x6f, 0xe6, 0x4e, 0x60, 0xa9, 0xed, 0x85, 0x70, 0x5b,
0xa3, 0x32, 0xbd, 0x8c, 0x1b, 0x11, 0xe4, 0x99, 0xce, 0x1f, 0xb9, 0x1f, 0xe1,
0xa8, 0x3a, 0x45, 0x55, 0xdd, 0x63, 0x94, 0x01, 0x99, 0xb7, 0xf7, 0x70, 0x7a,
0xf6, 0x49, 0xf3, 0x2e, 0xd3, 0xd7, 0x61, 0xfe, 0x81, 0x59, 0xd2, 0xe1, 0x45,
0x57, 0xfd, 0x1e, 0xfe, 0x87, 0x97, 0xbf, 0x40, 0x09, 0x02, 0x03, 0x01, 0x00,
0x01, 0x02, 0x81, 0x80, 0x03, 0x8d, 0x79, 0x2a, 0x6d, 0x64, 0xe5, 0x42, 0xd3,
0xb7, 0x0b, 0xbe, 0x75, 0x16, 0xb1, 0x3b, 0xf4, 0xc9, 0xb1, 0xd4, 0x47, 0x38,
0x6f, 0x98, 0xd9, 0x83, 0xf3, 0x30, 0x5e, 0x6f, 0x48, 0xf0, 0xc2, 0x67, 0x76,
0x06, 0x34, 0x37, 0xf3, 0x5d, 0x54, 0xfa, 0x16, 0xc2, 0xe7, 0xda, 0x4d, 0xee,
0x9c, 0x1b, 0xda, 0xdf, 0xee, 0x6e, 0x51, 0xcf, 0xc7, 0x39, 0x2f, 0x36, 0x5a,
0x53, 0x89, 0x00, 0x20, 0x5d, 0x0c, 0x14, 0x2c, 0xb1, 0x0b, 0xa1, 0x7f, 0xcf,
0xee, 0x48, 0x34, 0x01, 0x93, 0xb2, 0x67, 0x0b, 0x6b, 0xaa, 0xd8, 0xc6, 0x03,
0x67, 0xe3, 0xda, 0x64, 0xb4, 0xd1, 0x89, 0x9e, 0x46, 0xc5, 0xdd, 0x6c, 0x8a,
0xb6, 0x92, 0x0a, 0x24, 0xb7, 0x2d, 0x4a, 0x36, 0x7b, 0x5e, 0x3b, 0x98, 0x42,
0x5d, 0xa8, 0x58, 0xfe, 0x02, 0xd3, 0x5d, 0x1a, 0xc8, 0x02, 0xef, 0xc3, 0x4b,
0xe7, 0x59, 0x02, 0x41, 0x00, 0xf5, 0x8d, 0x58, 0xb0, 0x5e, 0x82, 0x17, 0x8b,
0xa7, 0x31, 0xed, 0x0b, 0xb2, 0x1c, 0x38, 0x7c, 0x07, 0x31, 0x15, 0xe3, 0x74,
0xc4, 0x7d, 0xd8, 0xd7, 0x29, 0xfe, 0x84, 0xa0, 0x8d, 0x81, 0xdd, 0x6c, 0xb4,
0x27, 0x41, 0xe3, 0x82, 0xa1, 0x6c, 0xa9, 0x3e, 0x93, 0xe8, 0xfd, 0x3c, 0xa4,
0xd1, 0x06, 0xb9, 0x9e, 0x07, 0x38, 0x51, 0xa4, 0x45, 0x3d, 0xff, 0x6d, 0x62,
0x39, 0xa3, 0x11, 0x5d, 0x02, 0x41, 0x00, 0xda, 0xc7, 0x67, 0xe8, 0xf0, 0xa2,
0xe3, 0xe7, 0x7d, 0x6c, 0xf4, 0xcc, 0x6d, 0x87, 0xf9, 0x76, 0xea, 0x61, 0xb4,
0xfa, 0xce, 0x05, 0x5e, 0xee, 0x5c, 0x12, 0x53, 0x88, 0xda, 0xc9, 0xc6, 0x81,
0x7d, 0x5c, 0xd3, 0x89, 0x1f, 0x2f, 0x7f, 0x1f, 0x11, 0x11, 0xd6, 0xd6, 0x45,
0x44, 0xe9, 0x1d, 0x35, 0x55, 0x1b, 0x02, 0xaf, 0xd4, 0xa5, 0xd1, 0xc2, 0xe7,
0x55, 0x35, 0x00, 0xf8, 0x62, 0x9d, 0x02, 0x41, 0x00, 0xbc, 0xe8, 0x90, 0x29,
0xa7, 0x9b, 0xa7, 0xe7, 0x9d, 0xaa, 0x50, 0x36, 0xa6, 0x41, 0x05, 0xc7, 0x8d,
0x74, 0xda, 0xe5, 0x11, 0x69, 0x35, 0x74, 0x44, 0x1c, 0x1f, 0x9e, 0x03, 0x32,
0xba, 0x8d, 0x11, 0xdb, 0x0b, 0x34, 0xaa, 0x86, 0x4e, 0x10, 0x1d, 0xa8, 0x71,
0xfc, 0x56, 0x0e, 0x78, 0xb2, 0x02, 0xdd, 0x7c, 0x51, 0x0b, 0xa7, 0xeb, 0x9c,
0x05, 0x95, 0x63, 0x9e, 0xa4, 0xbe, 0xea, 0x55, 0x02, 0x41, 0x00, 0x94, 0x8a,
0xc9, 0x79, 0x76, 0x51, 0x12, 0xae, 0x6d, 0x11, 0x9a, 0x50, 0x66, 0x99, 0xe8,
0xfe, 0x1d, 0x7b, 0x43, 0x96, 0xfa, 0x64, 0xd9, 0x24, 0xbb, 0xac, 0xd1, 0xbc,
0xdc, 0xd8, 0x1d, 0x08, 0x74, 0x66, 0x9f, 0x55, 0xbd, 0xaf, 0xd0, 0xfe, 0xf5,
0xe7, 0x07, 0xd8, 0x29, 0xe5, 0xf4, 0xe5, 0x18, 0xfd, 0xf4, 0xbd, 0xe9, 0x46,
0x57, 0x63, 0xc9, 0x92, 0xa9, 0xde, 0xb8, 0x0e, 0xed, 0x5d, 0x02, 0x40, 0x39,
0x30, 0x79, 0xb5, 0xe1, 0x22, 0xa7, 0x0e, 0xff, 0x96, 0x56, 0x7e, 0x16, 0xf3,
0x4c, 0xbd, 0x81, 0x94, 0x14, 0x53, 0xd4, 0x9b, 0xb6, 0xfa, 0xf9, 0x18, 0xf5,
0x4b, 0xe3, 0x5f, 0xb1, 0x87, 0x54, 0x67, 0x61, 0xc4, 0x05, 0x05, 0x01, 0x26,
0x89, 0xb0, 0xa3, 0x8b, 0xde, 0x18, 0xdb, 0x3f, 0x37, 0x09, 0x35, 0x65, 0x17,
0x9c, 0x37, 0xbe, 0x40, 0xed, 0xc6, 0x5c, 0xf8, 0x7e, 0x4b, 0x04};

#define PLAIN_TEXT_SIZE 16
#define RSA_KEY_SIZE 128

void psa_asymmetric_encryption_test(psa_algorithm_t alg,
                                    struct test_result_t *ret)
{
    psa_status_t status = PSA_SUCCESS;
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    const uint8_t plain_text[] = "This is a test.";
    uint8_t encrypted_data[PSA_ASYMMETRIC_ENCRYPT_OUTPUT_MAX_SIZE] = {0};
    size_t encrypted_size;
    size_t encrypted_length = 0;
    uint8_t decrypted_data[PLAIN_TEXT_SIZE] = {0};
    size_t decrypted_size = PLAIN_TEXT_SIZE;
    size_t decrypted_length = 0;
    const uint8_t * key_pair;
    size_t key_size;
    uint32_t comp_result;

    psa_key_attributes_t key_attributes = psa_key_attributes_init();

    /* Setup key attributes */
    psa_set_key_usage_flags(&key_attributes,
                            PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&key_attributes, alg);

    if (PSA_ALG_IS_RSA_OAEP(alg) || alg == PSA_ALG_RSA_PKCS1V15_CRYPT) {
        psa_set_key_type(&key_attributes, PSA_KEY_TYPE_RSA_KEY_PAIR);
        encrypted_size = RSA_KEY_SIZE;
        key_pair = rsa_key_pair;
        key_size = sizeof(rsa_key_pair);
    } else {
        TEST_FAIL("Unsupported asymmetric encryption algorithm");
        return;
    }

    status = psa_import_key(&key_attributes, key_pair, key_size, &key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing a key");
        return;
    }

    status = psa_asymmetric_encrypt(key_id_local, alg, plain_text,
                                    sizeof(plain_text), NULL, 0, encrypted_data,
                                    encrypted_size, &encrypted_length);
    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_LOG("psa_asymmetric_encrypt(): Algorithm NOT SUPPORTED by"\
                     " the implementation, skipping...\r\n");
        } else {
            TEST_FAIL("Error encrypting the plaintext");
        }
        goto destroy_key;
    }

    if (encrypted_size != encrypted_length) {
        TEST_FAIL("Unexpected encrypted length");
        goto destroy_key;
    }

    status = psa_asymmetric_decrypt(key_id_local, alg, encrypted_data,
                                    encrypted_size, NULL, 0, decrypted_data,
                                    decrypted_size, &decrypted_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error during asymmetric decryption");
        goto destroy_key;
    }

    if (decrypted_size != decrypted_length) {
        TEST_FAIL("Unexpected decrypted length");
        goto destroy_key;
    }

    /* Check that the plain text matches the decrypted data */
    comp_result = memcmp(plain_text, decrypted_data, sizeof(plain_text));
    if (comp_result != 0) {
        TEST_FAIL("Decrypted data doesn't match with plain text");
        goto destroy_key;
    }

    ret->val = TEST_PASSED;
destroy_key:
    /* Destroy the key */
    status = psa_destroy_key(key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error destroying a key");
    }
}

#define SIGNATURE_BUFFER_SIZE \
    (PSA_ECDSA_SIGNATURE_SIZE(PSA_BYTES_TO_BITS(sizeof(ecdsa_private_key))))

/* This helper function parses a signature as specified in RFC 5480 into a pair
 * (r,s) of contiguous bytes
 *
 * \param[in]  sig      Pointer to a buffer containing the encoded signature
 * \param[in]  slen     Size in bytes of the encoded signature structure
 * \param[out] r_s_pair Buffer containing the (r,s) pair extracted. It's caller
 *                      responsibility to ensure the buffer is big enough to
 *                      hold the parsed (r,s) pair.
 *
 * \return The size in bytes of the parsed signature, i.e. (r,s) pair
 */
static inline size_t parse_signature_from_rfc5480_encoding(const uint8_t *sig,
                                                           size_t slen,
                                                           uint8_t *r_s_pair)
{
    const uint8_t *sig_ptr = NULL;
    /* Move r in place */
    size_t r_len = sig[3];
    if (r_len % 2) {
        sig_ptr = &sig[5];
        r_len--;
    } else {
        sig_ptr = &sig[4];
    }
    memcpy(&r_s_pair[0], sig_ptr, r_len);
    /* Move s in place */
    size_t s_len = sig_ptr[r_len + 1];
    if (s_len % 2) {
        sig_ptr = &sig_ptr[3+r_len];
        s_len--;
    } else {
        sig_ptr = &sig_ptr[2+r_len];
    }
    memcpy(&r_s_pair[r_len], sig_ptr, s_len);
    slen = s_len + r_len; /* Update the length of the signature we're passing */
    return slen;
}

#define LEN_OFF (3) /* Offset for the Length field of the second SEQUENCE */
#define VAL_OFF (3) /* Offset for the value field of the BIT STRING */

/* This helper function gets a pointer to the bitstring associated to the publicKey
 * as encoded per RFC 5280. This function assumes that the public key encoding is not
 * bigger than 127 bytes (i.e. usually up until 384 bit curves)
 *
 * \param[in,out] p    Double pointer to a buffer containing the RFC 5280 of the ECDSA public key.
 *                     On output, the pointer is updated to point to the start of the public key
 *                     in BIT STRING form.
 * \param[in]     size Pointer to a buffer containing the size of the public key extracted
 *
 */
static inline void get_public_key_from_rfc5280_encoding(uint8_t **p, size_t *size)
{
    uint8_t *key_start = (*p) + (LEN_OFF + 1 + (*p)[LEN_OFF] + VAL_OFF);
    *p = key_start;
    *size = key_start[-2]-1; /* -2 from VAL_OFF to get the length, -1 to remove the ASN.1 padding byte count */
}

void psa_sign_verify_message_test(psa_algorithm_t alg,
                                  struct test_result_t *ret)
{
    psa_status_t status = PSA_SUCCESS;
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    psa_key_type_t key_type;
    psa_key_attributes_t input_key_attr = PSA_KEY_ATTRIBUTES_INIT;
    const uint8_t message[] =
        "This is the message that I would like to sign";
    uint8_t signature[SIGNATURE_BUFFER_SIZE] = {0};
    size_t signature_length = 0;
    /* The expected format of the public key is uncompressed, i.e. 0x04 X Y */
    uint8_t ecdsa_pub_key[
        PSA_KEY_EXPORT_ECC_PUBLIC_KEY_MAX_SIZE(PSA_BYTES_TO_BITS(sizeof(ecdsa_private_key)))] = {0};
    size_t pub_key_length = 0;
    uint32_t comp_result = 0;
    uint8_t hash[32] = {0}; /* Support only SHA-256 based signatures in the tests for simplicity */
    size_t hash_length = 0;
    uint8_t *p_key = (uint8_t *) ecdsa_public_key;
    uint8_t reformatted_signature[64] = {0};
    size_t public_key_size;
    size_t parsed_signature_size = parse_signature_from_rfc5480_encoding(
                                                                 reference_encoded_r_s,
                                                                 sizeof(reference_encoded_r_s),
                                                                 reformatted_signature);
    /* Get the BIT STRING for the public key */
    get_public_key_from_rfc5280_encoding(&p_key, &public_key_size);

    /* Initialize to the passing value */
    ret->val = TEST_PASSED;

    /* Set attributes and import key */
    psa_set_key_usage_flags(&input_key_attr,
        PSA_KEY_USAGE_SIGN_MESSAGE | PSA_KEY_USAGE_VERIFY_MESSAGE | PSA_KEY_USAGE_EXPORT);
    psa_set_key_algorithm(&input_key_attr, alg);
    key_type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);
    psa_set_key_type(&input_key_attr, key_type);

    status = psa_import_key(&input_key_attr, ecdsa_private_key,
                            sizeof(ecdsa_private_key), &key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing the private key");
        return;
    }

    status = psa_export_public_key(key_id_local, ecdsa_pub_key,
                                   sizeof(ecdsa_pub_key), &pub_key_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Public key export failed!");
        goto destroy_key;
    }

    if (pub_key_length !=
        PSA_KEY_EXPORT_ECC_PUBLIC_KEY_MAX_SIZE(PSA_BYTES_TO_BITS(sizeof(ecdsa_private_key)))) {
        TEST_FAIL("Unexpected length for the public key!");
        goto destroy_key;
    }

    /* Check that exported key matches the reference key */
    comp_result = memcmp(ecdsa_pub_key, p_key, pub_key_length);
    if (comp_result != 0) {
        TEST_FAIL("Exported ECDSA public key does not match the reference!");
        goto destroy_key;
    }

    status = psa_sign_message(key_id_local, alg,
                              message, sizeof(message) - 1,
                              signature, SIGNATURE_BUFFER_SIZE,
                              &signature_length);
    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_LOG("Algorithm NOT SUPPORTED by the implementation for signing, continue to verification...\r\n");
            signature_length = PSA_ECDSA_SIGNATURE_SIZE(PSA_BYTES_TO_BITS(sizeof(ecdsa_private_key)));
            memcpy(signature, reformatted_signature, signature_length);
        } else {
            TEST_FAIL("Message signing failed!");
            goto destroy_key;
        }
    }

    if (signature_length != PSA_ECDSA_SIGNATURE_SIZE(
                                PSA_BYTES_TO_BITS(
                                    sizeof(ecdsa_private_key)))) {
        TEST_FAIL("Unexpected signature length");
        goto destroy_key;
    }

    if (signature_length != parsed_signature_size) {
        TEST_FAIL("Produced signature length does not match the reference!");
        goto destroy_key;
    }

    /* Check that signature matches the reference provided */
    comp_result = memcmp(signature, reformatted_signature, signature_length);
    if (comp_result != 0) {
        TEST_FAIL("Signature does not match the reference!");
        goto destroy_key;
    }

    status = psa_verify_message(key_id_local, alg,
                                message, sizeof(message) - 1,
                                signature, signature_length);
    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_FAIL("Algorithm NOT SUPPORTED by the implementation for verification");
        } else {
            TEST_FAIL("Signature verification failed!");
        }
        goto destroy_key;
    }

destroy_key:
    psa_destroy_key(key_id_local);

    if (ret->val == TEST_FAILED) {
        return;
    }

    /* Continue with the dedicated verify hash flow */
    /* Set attributes and import key */
    psa_set_key_usage_flags(&input_key_attr, PSA_KEY_USAGE_VERIFY_HASH);
    psa_set_key_algorithm(&input_key_attr, alg);
    key_type = PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1);
    psa_set_key_type(&input_key_attr, key_type);

    status = psa_import_key(&input_key_attr, p_key,
                            public_key_size, &key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing the public key");
        return;
    }

    status = psa_hash_compute(PSA_ALG_GET_HASH(alg),
                              message, sizeof(message) - 1,
                              hash, sizeof(hash), &hash_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Hashing step failed!");
        goto destroy_public_key;
    }

    if (hash_length != PSA_HASH_LENGTH(PSA_ALG_GET_HASH(alg))) {
        TEST_FAIL("Unexpected hash length in the hashing step!");
        goto destroy_public_key;
    }

    status = psa_verify_hash(key_id_local, alg,
                             hash, hash_length,
                             signature, signature_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Signature verification failed in the verify_hash!");
    }

destroy_public_key:
    psa_destroy_key(key_id_local);

    return;
}

void psa_sign_verify_hash_test(psa_algorithm_t alg,
                               struct test_result_t *ret)
{
    psa_status_t status = PSA_SUCCESS;
    psa_key_id_t key_id_local = PSA_KEY_ID_NULL;
    psa_key_type_t key_type;
    psa_key_attributes_t input_key_attr = PSA_KEY_ATTRIBUTES_INIT;
    const uint8_t message[] =
        "This is the message that I would like to sign";
    uint8_t signature[SIGNATURE_BUFFER_SIZE] = {0};
    size_t signature_length = 0;
    /* The expected format of the public key is uncompressed, i.e. 0x04 X Y */
    uint8_t ecdsa_pub_key[
        PSA_KEY_EXPORT_ECC_PUBLIC_KEY_MAX_SIZE(PSA_BYTES_TO_BITS(sizeof(ecdsa_private_key)))] = {0};
    size_t pub_key_length = 0;
    uint32_t comp_result = 0;
    uint8_t hash[32] = {0}; /* Support only SHA-256 based signatures in the tests for simplicity */
    size_t hash_length = 0;
    uint8_t *p_key = (uint8_t *) ecdsa_public_key;
    size_t public_key_size;

    /* Get the BIT STRING for the public key */
    get_public_key_from_rfc5280_encoding(&p_key, &public_key_size);

    /* Initialize to the passing value */
    ret->val = TEST_PASSED;

    /* Set attributes and import key */
    psa_set_key_usage_flags(&input_key_attr,
        PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_EXPORT);
    psa_set_key_algorithm(&input_key_attr, alg);
    key_type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);
    psa_set_key_type(&input_key_attr, key_type);

    status = psa_import_key(&input_key_attr, ecdsa_private_key,
                            sizeof(ecdsa_private_key), &key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing the private key");
        return;
    }

    status = psa_export_public_key(key_id_local, ecdsa_pub_key,
                                   sizeof(ecdsa_pub_key), &pub_key_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Public key export failed!");
        goto destroy_key;
    }

    if (pub_key_length !=
        PSA_KEY_EXPORT_ECC_PUBLIC_KEY_MAX_SIZE(PSA_BYTES_TO_BITS(sizeof(ecdsa_private_key)))) {
        TEST_FAIL("Unexpected length for the public key!");
        goto destroy_key;
    }

    /* Check that exported key matches the reference key */
    comp_result = memcmp(ecdsa_pub_key, p_key, pub_key_length);
    if (comp_result != 0) {
        TEST_FAIL("Exported ECDSA public key does not match the reference!");
        goto destroy_key;
    }

    /* Compute the hash */
    status = psa_hash_compute(PSA_ALG_GET_HASH(alg), message, sizeof(message), hash, sizeof(hash), &hash_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Failure hashing the message before signing the hash!");
        goto destroy_key;
    }

    if (PSA_HASH_LENGTH(PSA_ALG_GET_HASH(alg)) != hash_length) {
        TEST_FAIL("Unexpected hash length!");
        goto destroy_key;
    }

    status = psa_sign_hash(key_id_local, alg, hash, hash_length, signature, sizeof(signature), &signature_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Hash signing failed!");
        goto destroy_key;
    }

    if (signature_length != PSA_ECDSA_SIGNATURE_SIZE(
                                PSA_BYTES_TO_BITS(
                                    sizeof(ecdsa_private_key)))) {
        TEST_FAIL("Unexpected signature length");
        goto destroy_key;
    }

    /* It is not possible to compare the signature against a reference because it might be non-deterministic */

    status = psa_verify_hash(key_id_local, alg, hash, hash_length, signature, signature_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Signature verification for the provided hash failed!");
    }

destroy_key:
    psa_destroy_key(key_id_local);

    if (ret->val == TEST_FAILED) {
        return;
    }

    /* Continue with the dedicated verify hash flow */
    /* Set attributes and import key */
    psa_set_key_usage_flags(&input_key_attr, PSA_KEY_USAGE_VERIFY_HASH);
    psa_set_key_algorithm(&input_key_attr, alg);
    key_type = PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1);
    psa_set_key_type(&input_key_attr, key_type);

    status = psa_import_key(&input_key_attr, p_key,
                            public_key_size, &key_id_local);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing the public key");
        return;
    }

    status = psa_verify_hash(key_id_local, alg, hash, hash_length, signature, signature_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Signature verification for the provided hash failed!");
    }

    psa_destroy_key(key_id_local);

    return;
}

#if defined(TFM_CRYPTO_TEST_ALG_RSASSA_PSS_VERIFICATION)
/* A random RSA key of length 2048 bits, public key (N,E) in DER format. The PSA format is
 * implementation dependent, so we parse the DER format of the key below through the function
 * mbedtls_pk_parse_public_key() and then we use mbedtls_pk_import_into_psa(), and finally
 * psa_export_public_key() to get the PSA format bytes
 *
 * static const uint8_t rsa_key_2048_der_format[] = {
 * 0x30, 0x82, 0x01, 0x22, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01,
 * 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0F, 0x00, 0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01, 0x01,
 * 0x00, 0xDA, 0x4E, 0x01, 0x18, 0x21, 0x6C, 0xA2, 0xCF, 0xEE, 0xD2, 0xD1, 0x8A, 0x55, 0xE5, 0x2B,
 * 0x43, 0x02, 0x0F, 0x89, 0xEC, 0xCA, 0xE9, 0x1A, 0x2F, 0x80, 0xCE, 0x7B, 0x42, 0xDD, 0xF9, 0xD2,
 * 0xF9, 0x7B, 0xA2, 0x17, 0x97, 0x76, 0x3F, 0xE2, 0x97, 0x95, 0x0B, 0xA7, 0x77, 0x21, 0x9D, 0x18,
 * 0x16, 0x1F, 0xC1, 0x1E, 0x1D, 0xF8, 0xDE, 0xDA, 0x18, 0x98, 0xE2, 0xF5, 0xB0, 0x3D, 0x71, 0xBB,
 * 0x63, 0x54, 0x36, 0x16, 0xCB, 0x03, 0x72, 0x27, 0xAF, 0x24, 0x66, 0xE9, 0x44, 0x1E, 0xDC, 0x17,
 * 0x56, 0x2E, 0x97, 0xA4, 0xA0, 0x52, 0xB0, 0x61, 0xEF, 0x9C, 0x10, 0x01, 0x2D, 0x5D, 0x12, 0x60,
 * 0xA8, 0xA3, 0xEE, 0xEF, 0x22, 0x49, 0xDE, 0xBA, 0xC1, 0x83, 0x6F, 0x0E, 0x38, 0x10, 0x0E, 0x56,
 * 0x63, 0x6D, 0x6C, 0x44, 0xD7, 0xB1, 0xA5, 0x15, 0xB6, 0x94, 0x1D, 0xAF, 0x5A, 0xE5, 0x1C, 0x10,
 * 0x8D, 0x05, 0x67, 0x3A, 0x60, 0xBD, 0xE7, 0xCB, 0x60, 0xE0, 0x6A, 0x84, 0x96, 0x3C, 0xDB, 0xA4,
 * 0x71, 0x93, 0x7C, 0xF4, 0x25, 0x15, 0x32, 0x41, 0x1F, 0x16, 0x72, 0xB3, 0x15, 0xFB, 0x76, 0xA2,
 * 0x9E, 0x77, 0x69, 0xA3, 0x34, 0x7D, 0x8A, 0x39, 0x38, 0xA8, 0xBA, 0x44, 0xAD, 0x0C, 0x3C, 0x61,
 * 0x5C, 0x7D, 0xC7, 0xDF, 0x25, 0xEC, 0x0F, 0xC4, 0x4E, 0x81, 0x73, 0xB6, 0xC1, 0x08, 0xE6, 0xB7,
 * 0xE7, 0xD6, 0xC8, 0x14, 0xCB, 0x6E, 0x4E, 0xA4, 0x1F, 0xD7, 0x29, 0x0D, 0x08, 0x46, 0xC7, 0x2C,
 * 0xC0, 0x8E, 0x42, 0x0F, 0x49, 0xA5, 0xA6, 0xB0, 0x0E, 0x5D, 0xEC, 0x7A, 0x4A, 0x4E, 0xAE, 0xFA,
 * 0x1B, 0x5F, 0x77, 0x9F, 0x4A, 0x86, 0x66, 0x47, 0x20, 0xBC, 0x69, 0xF3, 0xD9, 0x63, 0xC1, 0xF8,
 * 0x84, 0x99, 0x12, 0x67, 0x65, 0xE1, 0x40, 0xAC, 0x42, 0x93, 0x11, 0x97, 0xF3, 0xBB, 0x93, 0xB0,
 * 0x1B, 0x02, 0x03, 0x01, 0x00, 0x01,
 * };
 *
 * mbedtls_pk_parse_public_key(&ctx, rsa_key_2048_der_format, sizeof(rsa_key_2048_der_format));
 * mbedtls_pk_import_into_psa(&ctx, &attr, &id);
 * psa_export_public_key(id, rsa_key_2048, sizeof(rsa_key_2048), &rsa_key_2048_size);
 */
static const uint8_t rsa_key_2048[] = {
0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01, 0x01, 0x00, 0xDA, 0x4E, 0x01, 0x18, 0x21, 0x6C, 0xA2,
0xCF, 0xEE, 0xD2, 0xD1, 0x8A, 0x55, 0xE5, 0x2B, 0x43, 0x02, 0x0F, 0x89, 0xEC, 0xCA, 0xE9, 0x1A,
0x2F, 0x80, 0xCE, 0x7B, 0x42, 0xDD, 0xF9, 0xD2, 0xF9, 0x7B, 0xA2, 0x17, 0x97, 0x76, 0x3F, 0xE2,
0x97, 0x95, 0x0B, 0xA7, 0x77, 0x21, 0x9D, 0x18, 0x16, 0x1F, 0xC1, 0x1E, 0x1D, 0xF8, 0xDE, 0xDA,
0x18, 0x98, 0xE2, 0xF5, 0xB0, 0x3D, 0x71, 0xBB, 0x63, 0x54, 0x36, 0x16, 0xCB, 0x03, 0x72, 0x27,
0xAF, 0x24, 0x66, 0xE9, 0x44, 0x1E, 0xDC, 0x17, 0x56, 0x2E, 0x97, 0xA4, 0xA0, 0x52, 0xB0, 0x61,
0xEF, 0x9C, 0x10, 0x01, 0x2D, 0x5D, 0x12, 0x60, 0xA8, 0xA3, 0xEE, 0xEF, 0x22, 0x49, 0xDE, 0xBA,
0xC1, 0x83, 0x6F, 0x0E, 0x38, 0x10, 0x0E, 0x56, 0x63, 0x6D, 0x6C, 0x44, 0xD7, 0xB1, 0xA5, 0x15,
0xB6, 0x94, 0x1D, 0xAF, 0x5A, 0xE5, 0x1C, 0x10, 0x8D, 0x05, 0x67, 0x3A, 0x60, 0xBD, 0xE7, 0xCB,
0x60, 0xE0, 0x6A, 0x84, 0x96, 0x3C, 0xDB, 0xA4, 0x71, 0x93, 0x7C, 0xF4, 0x25, 0x15, 0x32, 0x41,
0x1F, 0x16, 0x72, 0xB3, 0x15, 0xFB, 0x76, 0xA2, 0x9E, 0x77, 0x69, 0xA3, 0x34, 0x7D, 0x8A, 0x39,
0x38, 0xA8, 0xBA, 0x44, 0xAD, 0x0C, 0x3C, 0x61, 0x5C, 0x7D, 0xC7, 0xDF, 0x25, 0xEC, 0x0F, 0xC4,
0x4E, 0x81, 0x73, 0xB6, 0xC1, 0x08, 0xE6, 0xB7, 0xE7, 0xD6, 0xC8, 0x14, 0xCB, 0x6E, 0x4E, 0xA4,
0x1F, 0xD7, 0x29, 0x0D, 0x08, 0x46, 0xC7, 0x2C, 0xC0, 0x8E, 0x42, 0x0F, 0x49, 0xA5, 0xA6, 0xB0,
0x0E, 0x5D, 0xEC, 0x7A, 0x4A, 0x4E, 0xAE, 0xFA, 0x1B, 0x5F, 0x77, 0x9F, 0x4A, 0x86, 0x66, 0x47,
0x20, 0xBC, 0x69, 0xF3, 0xD9, 0x63, 0xC1, 0xF8, 0x84, 0x99, 0x12, 0x67, 0x65, 0xE1, 0x40, 0xAC,
0x42, 0x93, 0x11, 0x97, 0xF3, 0xBB, 0x93, 0xB0, 0x1B, 0x02, 0x03, 0x01, 0x00, 0x01,
};

#define SIGNATURE_OUTPUT_SIZE_RSA_2048 \
    PSA_SIGN_OUTPUT_SIZE(PSA_KEY_TYPE_RSA_KEY_PAIR, 2048, PSA_ALG_RSA_PSS(PSA_ALG_SHA_256))

/*
 * Signature for the following message:
 *     const uint8_t message[] = "This is the message that I would like to sign";
 *
 * This can be obtained by running psa_sign_message() as follows:
 *     psa_sign_message(key_id, PSA_ALG_RSA_PSS(PSA_ALG_SHA256),
 *                      message, sizeof(message) - 1,
 *                      signature_pss_sha_256,  SIGNATURE_OUTPUT_SIZE_RSA_2048,
 *                      &signature_length);
 *
 * where signature_pss_sha_256 contains the produced signature. The key_id refers here to
 * the full RSA_KEY_PAIR, while for the verification path we only need to keep the public part.
 *
 */
static const uint8_t signature_pss_sha_256[] = {
0x47, 0xB5, 0x85, 0x1D, 0xE2, 0xD8, 0xFC, 0xC2, 0x69, 0x22, 0x33, 0x9C, 0xB2, 0xBA, 0x69, 0x87,
0xD1, 0x9C, 0x67, 0xEE, 0xE6, 0x48, 0x27, 0xD2, 0x07, 0xA3, 0x19, 0x2E, 0x43, 0xFD, 0xCE, 0xCA,
0xE7, 0x63, 0xB4, 0xC0, 0x02, 0x4D, 0x61, 0x69, 0xAA, 0x4A, 0xF7, 0xB9, 0x80, 0xC3, 0x53, 0x78,
0x5D, 0xC3, 0xA4, 0x0C, 0xC3, 0x67, 0x2F, 0xB7, 0xBD, 0x73, 0xF4, 0x58, 0x95, 0x16, 0x97, 0x45,
0xA1, 0x4F, 0x3F, 0xB1, 0x44, 0x08, 0x57, 0x0A, 0x87, 0xDE, 0x3F, 0xF1, 0x3A, 0xC9, 0x24, 0xE6,
0xA6, 0xEB, 0x22, 0xE9, 0xC9, 0x8F, 0x7E, 0x65, 0x30, 0x36, 0x35, 0x62, 0x06, 0xDF, 0xBE, 0x1D,
0xA4, 0x1C, 0x42, 0x3D, 0xAB, 0x22, 0x01, 0xDD, 0x86, 0x3D, 0x81, 0x12, 0xF5, 0x5B, 0x16, 0xFD,
0xF2, 0xFE, 0x7D, 0x90, 0x60, 0x3A, 0x70, 0xCB, 0xAB, 0xE5, 0x3F, 0xA5, 0x83, 0x42, 0xD5, 0x42,
0x18, 0x00, 0x5E, 0xC1, 0x4F, 0x7E, 0x3A, 0xCD, 0x42, 0xA2, 0x16, 0x40, 0xCD, 0xE6, 0xC6, 0xBC,
0xDD, 0xBA, 0x11, 0xA5, 0xA9, 0x31, 0xE1, 0x3F, 0xCB, 0x92, 0x3B, 0x5D, 0xE4, 0xB1, 0xD1, 0x3A,
0x5F, 0xE4, 0xCB, 0x3B, 0x61, 0xD7, 0x83, 0xCB, 0x9A, 0x7B, 0xA7, 0x71, 0x60, 0x90, 0x13, 0xC0,
0x27, 0x2D, 0xD4, 0xF4, 0x32, 0xC5, 0x80, 0xEA, 0x7E, 0xA7, 0xE8, 0x74, 0x97, 0x14, 0xC3, 0xCE,
0x15, 0x84, 0x5E, 0x09, 0x63, 0x08, 0xDA, 0x19, 0xB5, 0x27, 0x9F, 0xEE, 0xE4, 0xCE, 0x08, 0x5B,
0x9E, 0x95, 0xCA, 0xC8, 0xC1, 0x54, 0xEE, 0x69, 0xB8, 0x25, 0xB9, 0xE6, 0xCD, 0x1C, 0x4B, 0x4E,
0xA2, 0x69, 0xC2, 0xE7, 0x91, 0xA7, 0xA6, 0x5C, 0x55, 0x6A, 0xAA, 0xE9, 0x91, 0x4F, 0x0D, 0x05,
0x7E, 0x49, 0x19, 0xF4, 0xE9, 0xE5, 0xB4, 0x3E, 0x5C, 0x5E, 0xFB, 0x28, 0x23, 0xE7, 0x27, 0xAE,
};

/* A test case dedicated to RSASSA-PSA VERIFY operation */
void psa_verify_rsassa_pss_test(struct test_result_t *ret)
{
    psa_status_t status = PSA_SUCCESS;
    psa_key_id_t key_id = PSA_KEY_ID_NULL;
    psa_key_attributes_t key_attr = PSA_KEY_ATTRIBUTES_INIT;
    const psa_algorithm_t alg = PSA_ALG_RSA_PSS(PSA_ALG_SHA_256);
    uint8_t hash[32] = {0};
    size_t hash_length = 0;

    const uint8_t message[] =
        "This is the message that I would like to sign";

    /* Set attributes and import key */
    /* The verify_hash flag enables automatically verify_message as well */
    psa_set_key_usage_flags(&key_attr, PSA_KEY_USAGE_VERIFY_HASH);
    psa_set_key_algorithm(&key_attr, alg);
    psa_set_key_type(&key_attr, PSA_KEY_TYPE_RSA_PUBLIC_KEY);

    status = psa_import_key(&key_attr,
                            (const uint8_t *)rsa_key_2048,
                            sizeof(rsa_key_2048),
                            &key_id);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Error importing the private key");
        return;
    }

    if (sizeof(signature_pss_sha_256) != SIGNATURE_OUTPUT_SIZE_RSA_2048) {
        TEST_FAIL("Unexpected signature length");
        goto destroy_key;
    }

    status = psa_verify_message(key_id, alg,
                                message, sizeof(message) - 1,
                                signature_pss_sha_256, sizeof(signature_pss_sha_256));
    if (status != PSA_SUCCESS) {
        if (status == PSA_ERROR_NOT_SUPPORTED) {
            TEST_FAIL("Algorithm NOT SUPPORTED by the implementation");
            goto destroy_key;
        }

        TEST_FAIL("Signature verification failed in the verify_message!");
        goto destroy_key;
    }

    /* Try the same verification, but this time split the hash calculation in a
     * separate API call. This is useful for those protocols that require to
     * treat the hashing in a special way (i.e. special seeding), or need to do
     * hashing in multipart. For simplicity here we just use single-part hashing
     */
    status = psa_hash_compute(PSA_ALG_GET_HASH(alg),
                              message, sizeof(message) - 1,
                              hash, sizeof(hash), &hash_length);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Hashing step failed!");
        goto destroy_key;
    }

    if (hash_length != 32) {
        TEST_FAIL("Unexpected hash length in the hashing step!");
        goto destroy_key;
    }

    status = psa_verify_hash(key_id, alg,
                             hash, hash_length,
                             signature_pss_sha_256, sizeof(signature_pss_sha_256));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Signature verification failed in the verify_hash!");
        goto destroy_key;
    }
    ret->val = TEST_PASSED;

destroy_key:
    psa_destroy_key(key_id);

    return;
}
#endif /* TFM_CRYPTO_TEST_ALG_RSASSA_PSS_VERIFICATION */

#if defined(TFM_CRYPTO_TEST_SINGLE_PART_FUNCS)
#if defined(TFM_CRYPTO_TEST_ALG_GCM) || defined(TFM_CRYPTO_TEST_ALG_CCM)
static const uint8_t cipher_tag_auth_test[][2][16] = {
{
    {0x60, 0x9b, 0x3d, 0x51, 0x91, 0x60, 0x8, 0x17,
     0x82, 0xec, 0x63, 0x21, 0x3a, 0x4, 0xdc, 0x93}, /* Valid set */
    {0xaa, 0x96, 0xcf, 0xb4, 0x68, 0xe5, 0x4, 0x91,
     0x52, 0x50, 0x59, 0xa, 0xab, 0x1a, 0xe9, 0x1b}  /* Invalid set */
},
{
    {0x59, 0x93, 0x96, 0x41, 0x29, 0xe6, 0xe4, 0x25,
     0x41, 0xcc, 0x64, 0x90, 0x6e, 0xdf, 0x4c, 0xee}, /* Valid set */
    {0x5e, 0x29, 0xf4, 0x3c, 0xe9, 0x94, 0x2e, 0xa4,
     0xc3, 0x9d, 0x61, 0x91, 0x9a, 0x46, 0xf5, 0x74}  /* Invalid set - might need to replace with the invalidly passing dumped values */
}
};

static const uint8_t iv_tag_auth_test[][12] = {
    {0x87, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, /* Valid set */
    {0x8a, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}  /* Invalid set */
};

static const uint8_t add_tag_auth_test[][364] = {
    {0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xcf, 0xf, 0xf2, 0x63, 0xb0, 0x1b, 0xa7, 0x28,
     0xfa, 0x46, 0xbd, 0x8d, 0x42, 0x34, 0xbb, 0x83, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0xba, 0xb, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x28, 0x1d, 0x9c, 0x36, 0x16, 0x84, 0x34, 0x91,
     0x8d, 0x2f, 0xf3, 0xf8, 0x27, 0xe5, 0x36, 0xc4, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0xba, 0xb, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7d, 0x0, 0x0, 0x0},
    {0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xcf, 0xf, 0xf2, 0x63, 0xb0, 0x1b, 0xa7, 0x28,
     0xfa, 0x46, 0xbd, 0x8d, 0x42, 0x34, 0xbb, 0x83, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0xba, 0xb, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xed, 0x58, 0x8f, 0xa4, 0x87, 0x90, 0xe2, 0xf7,
     0xca, 0x0, 0x2d, 0x9b, 0x93, 0x1f, 0x9, 0x66, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0xba, 0xb, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80, 0x0, 0x0, 0x0}
};

int psa_aead_as_authenticator_test(void)
{
    psa_status_t status;
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t key_id = PSA_KEY_ID_NULL;
    const uint8_t key[] = {
        0xc8, 0x8b, 0x74, 0xc6,
        0x24, 0x50, 0xee, 0xd8,
        0xe9, 0x22, 0xd, 0x98,
        0x3a, 0x11, 0xc6, 0x1,
    };
    size_t out_len;
    uint8_t ref[16]; size_t ref_size;
    int ret = 0;
    uint8_t test_sel;
    psa_algorithm_t alg = PSA_ALG_NONE;

    for (test_sel = 0; test_sel < 2 && ret == 0; test_sel++) {
#if defined(TFM_CRYPTO_TEST_ALG_GCM)
        if (test_sel == 0) {
            alg = PSA_ALG_GCM;
        } else
#endif /* TFM_CRYPTO_TEST_ALG_GCM */
#if defined(TFM_CRYPTO_TEST_ALG_CCM)
        if (test_sel == 1) {
            alg = PSA_ALG_CCM;
        } else
#endif /* TFM_CRYPTO_TEST_ALG_GCM */
        {
            continue;
        }

        /* import key */
        psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_DECRYPT | PSA_KEY_USAGE_ENCRYPT);
        psa_set_key_algorithm(&attr, alg);
        psa_set_key_type(&attr, PSA_KEY_TYPE_AES);

        status = psa_import_key(&attr, key, sizeof(key), &key_id);
        if (status != PSA_SUCCESS) {
            TEST_LOG("Unable to import the key\r\n");
            return 1;
        }

        /* Create the reference for set 0 (valid set) */
        status = psa_aead_encrypt(key_id, alg,
                     iv_tag_auth_test[0], sizeof(iv_tag_auth_test[0]),
                     add_tag_auth_test[0], sizeof(add_tag_auth_test[0]),
                     NULL, 0,
                     ref, sizeof(ref), &ref_size);
        if (status != PSA_SUCCESS) {
            TEST_LOG("status is %d\r\n", status);
            TEST_LOG("Unable to create the reference tag for the valid set %d, alg: 0x%x\r\n", test_sel, alg);
            ret = 1;
            goto destroy_key;
        } else {
            TEST_LOG("REF(0)TAG: ");
            for (int i=0; i<ref_size; i++)
                TEST_LOG("0x%x, ", ref[i]);
            TEST_LOG("\r\n");
        }

        /* Validate the provided tag for set 0 */
        status = psa_aead_decrypt(key_id, alg,
                     iv_tag_auth_test[0], sizeof(iv_tag_auth_test[0]),
                     add_tag_auth_test[0], sizeof(add_tag_auth_test[0]),
                     cipher_tag_auth_test[test_sel][0], sizeof(cipher_tag_auth_test[test_sel][0]),
                     NULL, 0, &out_len);
        if (status != PSA_SUCCESS) {
            TEST_LOG("status with test0 is %d\r\n", status);
            return 1;
        }

        /* Validate the reference tag for set 0 */
        status = psa_aead_decrypt(key_id, alg,
                     iv_tag_auth_test[0], sizeof(iv_tag_auth_test[0]),
                     add_tag_auth_test[0], sizeof(add_tag_auth_test[0]),
                     ref, ref_size,
                     NULL, 0, &out_len);
        if (status != PSA_SUCCESS) {
            TEST_LOG("status with ref0 is %d\r\n", status);
            return 1;
        }

        /* Create the reference for set 1 (invalid set) */
        status = psa_aead_encrypt(
                     key_id, alg,
                     iv_tag_auth_test[1], sizeof(iv_tag_auth_test[1]),
                     add_tag_auth_test[1], sizeof(add_tag_auth_test[1]),
                     NULL, 0,
                     ref, sizeof(ref), &ref_size);
        if (status != PSA_SUCCESS) {
            TEST_LOG("Unable to create the reference tag for the invalid set %d, alg: 0x%x\r\n", test_sel, alg);
            ret = 1;
            goto destroy_key;
        } else {
            TEST_LOG("REF(1)TAG: ");
            for (int i=0; i<ref_size; i++)
                TEST_LOG("0x%x, ", ref[i]);
            TEST_LOG("\r\n");
        }

        /* Validate the provided tag for set 1 */
        status = psa_aead_decrypt(
                     key_id, alg,
                     iv_tag_auth_test[1], sizeof(iv_tag_auth_test[1]),
                     add_tag_auth_test[1], sizeof(add_tag_auth_test[1]),
                     cipher_tag_auth_test[test_sel][1], sizeof(cipher_tag_auth_test[test_sel][1]),
                     NULL, 0, &out_len);
        if (status != PSA_ERROR_INVALID_SIGNATURE) {
            TEST_LOG("status with test1 is %d\r\n", status);
            ret = 1;
        }

        /* Validate the reference tag for set 1 */
        status = psa_aead_decrypt(
                     key_id, alg,
                     iv_tag_auth_test[1], sizeof(iv_tag_auth_test[1]),
                     add_tag_auth_test[1], sizeof(add_tag_auth_test[1]),
                     ref, ref_size,
                     NULL, 0, &out_len);
        if (status != PSA_SUCCESS) {
            TEST_LOG("status with ref1 is %d\r\n", status);
            ret = 1;
        }

destroy_key:
        psa_destroy_key(key_id);
    }

    return ret;
}
#endif /* TFM_CRYPTO_TEST_ALG_GCM || TFM_CRYPTO_TEST_ALG_CCM */
#endif /* TFM_CRYPTO_TEST_SINGLE_PART_FUNCS */
