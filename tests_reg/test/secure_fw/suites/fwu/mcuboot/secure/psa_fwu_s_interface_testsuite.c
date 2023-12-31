/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "fwu_s_tests.h"
#include "psa/update.h"
#include "test_framework_helpers.h"
#include "../fwu_tests_common.h"

static struct test_t psa_fwu_s_tests[] = {
    {&tfm_fwu_test_common_001, "TFM_S_FWU_TEST_1001",
     "Functionity test."},
    {&tfm_fwu_test_common_002, "TFM_S_FWU_TEST_1002",
     "Boundary test with invalid arguments."},
    {&tfm_fwu_test_common_003, "TFM_S_FWU_TEST_1003",
     "Boundary test with not supported components."},
    {&tfm_fwu_test_common_004, "TFM_S_FWU_TEST_1004",
     "Component state management test."},
#if (MCUBOOT_IMAGE_NUMBER > 1)
    {&tfm_fwu_test_common_005, "TFM_S_FWU_TEST_1005",
     "Dependency pass in multiple components case."},
#if (FWU_COMPONENT_NUMBER > 1)
    {&tfm_fwu_test_common_006, "TFM_S_FWU_TEST_1006",
     "Update two components which depends on each other."},
#endif
#endif
#ifdef TFM_FWU_TEST_REQUEST_REBOOT
    {&tfm_fwu_test_common_007, "TFM_S_FWU_TEST_1007",
     "Reboot interface"},
#endif
};

void register_testsuite_s_psa_fwu_interface(struct test_suite_t *p_test_suite)
{
    uint32_t list_size;

    list_size = (sizeof(psa_fwu_s_tests) / sizeof(psa_fwu_s_tests[0]));

    set_testsuite("PSA firmware update interface tests"
                  "(TFM_S_FWU_TEST_1XXX)",
                  psa_fwu_s_tests, list_size, p_test_suite);
}
