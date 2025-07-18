/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_framework.h"

#include "bl1_1_crypto_tests.h"
#include "bl1_1_random_generation_tests.h"
#include "bl1_1_integration_tests.h"
#include "extra_bl1_1_tests.h"

#ifdef EXTRA_BL1_1_TEST_SUITE
#include "extra_bl1_1_tests.h"
#endif /* EXTRA_BL1_1_TEST_SUITE */

static struct test_suite_t test_suites[] = {

    {&register_testsuite_bl1_crypto_interface, 0, 0, 0},
    {&register_testsuite_bl1_random_generation_interface, 0, 0, 0},
    {&register_testsuite_bl1_1_integration, 0, 0, 0},

#ifdef EXTRA_BL1_1_TEST_SUITE
    /* BL1_1 extra test cases */
    {&register_testsuite_extra_bl1_1, 0, 0, 0},
#endif

    /* End of test suites */
    {0, 0, 0, 0}
};

enum test_suite_err_t run_bl1_1_testsuite(void)
{
    return run_test("BL1_1", test_suites);
}
