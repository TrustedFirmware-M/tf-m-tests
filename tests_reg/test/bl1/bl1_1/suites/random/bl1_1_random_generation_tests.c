/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "bl1_1_random_generation_tests.h"

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "test_framework_helpers.h"
#include "psa/crypto.h"

#define TEST_VAL_AM      512
#define REPEAT_THRESHOLD (TEST_VAL_AM / 16)

static void tfm_bl1_random_generate_test_3001(struct test_result_t *ret)
{
    uint8_t buf[TEST_VAL_AM];
    size_t buf_idx;
    uint16_t table[UINT8_MAX + 1];
    size_t table_idx;
    bool failed = false;
    psa_status_t status;

    memset(table, 0, sizeof(table));
    status = psa_generate_random(buf, sizeof(buf));
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Random generation returned error");
        return;
    }

    /* Count the amount of each byte value that's emitted by the DRBG */
    for (buf_idx = 0; buf_idx < sizeof(buf); buf_idx++) {
        table[buf[buf_idx]] += 1;
    }

    /* If any of the counts is over the repeat threshold (currently P >= 1/16,
     * it is a failure.
     */
    for (table_idx = 0; table_idx < (sizeof(table) / sizeof(table[0])); table_idx++) {
        if (table[table_idx] >= REPEAT_THRESHOLD) {
            TEST_LOG("Byte 0x%x was repeated %d times\r\n", table_idx,
                                                            table[table_idx]);
            failed = true;
        }
    }

    if (failed) {
        TEST_FAIL("Byte was repeated too many times");
        return;
    }

    ret->val = TEST_PASSED;
    return;
}

static void tfm_bl1_random_generate_test_3002(struct test_result_t *ret)
{
    psa_status_t status;

    status = psa_generate_random(NULL, 0);
    if (status != PSA_SUCCESS) {
        TEST_FAIL("Random generation should have succeeded");
        return;
    }

    ret->val = TEST_PASSED;
    return;
}

static struct test_t random_generation_tests[] = {
    {&tfm_bl1_random_generate_test_3001, "TFM_BL1_RANDOM_TEST_3001",
     "Secure random generation repeated bytes test" },
    {&tfm_bl1_random_generate_test_3002, "TFM_BL1_RANDOM_TEST_3002",
     "Secure random generation zero-length output test" },
};

void register_testsuite_bl1_random_generation_interface(struct test_suite_t *p_test_suite)
{
    uint32_t list_size = (sizeof(random_generation_tests) / sizeof(random_generation_tests[0]));

    set_testsuite("BL1 random generation interface test (TFM_BL1_RANDOM_TEST_3XXX)",
                  random_generation_tests, list_size, p_test_suite);
}
