/*
 * Copyright (c) 2018-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "attest_ns_tests.h"
#include "psa/initial_attestation.h"
#include "attest.h"
#include "attest_tests_common.h"
#include "attest_token_test_values.h"
#include "attest_token_test.h"

/* Define test suite for attestation service tests */
/* List of tests */
static void tfm_attest_test_2001(struct test_result_t *ret);
#ifdef INCLUDE_TEST_CODE
static void tfm_attest_test_2002(struct test_result_t *ret);
static void tfm_attest_test_2003(struct test_result_t *ret);
#endif

static struct test_t attestation_interface_tests[] = {
    {&tfm_attest_test_2001, "TFM_NS_ATTEST_TEST_2001",
     "Symmetric key algorithm based Initial Attestation test"},
#ifdef INCLUDE_TEST_CODE
    {&tfm_attest_test_2002, "TFM_NS_ATTEST_TEST_2002",
     "Minimal token size test of attest token"},
    {&tfm_attest_test_2003, "TFM_NS_ATTEST_TEST_2003",
     "Negative test cases for initial attestation service"},
#endif
};

void
register_testsuite_ns_attestation_interface(struct test_suite_t *p_test_suite)
{
    uint32_t list_size;

    list_size = (sizeof(attestation_interface_tests) /
                 sizeof(attestation_interface_tests[0]));

    set_testsuite("Symmetric key algorithm based Initial Attestation Service "
                  "non-secure interface tests (TFM_NS_ATTEST_TEST_2XXX)",
                  attestation_interface_tests, list_size, p_test_suite);
}

/*!
 * \brief Get an IAT with symmetric key algorithm based Initial Attestation.
 */
static void tfm_attest_test_2001(struct test_result_t *ret)
{
    int32_t err;

    err = decode_test_symmetric_initial_attest();
    if (err != 0) {
        TEST_LOG("tfm_attest_test_2001() returned: %d\r\n", err);
        TEST_FAIL("Attest token tfm_attest_test_2001() has failed");
        return;
    }

    ret->val = TEST_PASSED;
}

#ifdef INCLUDE_TEST_CODE
/*!
 * \brief Get the size of the minimal token, only include a hard coded
 *        challenge, but omit the rest of the claims
 */
static void tfm_attest_test_2002(struct test_result_t *ret)
{
    int32_t err;

    err = minimal_get_size_test();
    if (err != 0) {
        TEST_LOG("minimal_get_size_test() returned: %d\r\n", err);
        TEST_FAIL("Attest token minimal_get_size_test() has failed");
        return;
    }

    ret->val = TEST_PASSED;
}

/*!
 * \brief Negative tests for initial attestation service
 *
 *    - Calling initial attestation service with bigger challenge object than
 *      allowed.
 *    - Calling initial attestation service with smaller buffer size than the
 *      expected size of the token.
 */
static void tfm_attest_test_2003(struct test_result_t *ret)
{
    psa_status_t err;
    size_t token_size;

    /* Call with with bigger challenge object than allowed */
    err = psa_initial_attest_get_token_size(INVALID_CHALLENGE_OBJECT_SIZE,
                                            &token_size);

    if (err != PSA_ERROR_INVALID_ARGUMENT) {
        TEST_FAIL("Attestation should fail with too big challenge object");
        return;
    }

    /* Call with smaller buffer size than size of test token */
    err = buffer_too_small_test();
    if (err != 0) {
        TEST_LOG("buffer_too_small_test() returned: %d\r\n", err);
        TEST_FAIL("Attest token buffer_too_small_test() has failed");
        return;
    }

    ret->val = TEST_PASSED;
}
#endif /* INCLUDE_TEST_CODE */
