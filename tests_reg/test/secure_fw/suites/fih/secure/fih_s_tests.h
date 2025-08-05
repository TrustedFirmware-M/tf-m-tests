/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __FIH_S_TESTS_H__
#define __FIH_S_TESTS_H__

#include "test_framework.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Register testsuite for the FIH library.
 *
 * \param[in] p_test_suite The test suite to be executed.
 */
void
register_testsuite_s_fih_interface(struct test_suite_t *p_test_suite);

#ifdef __cplusplus
}
#endif

#endif /* __FIH_S_TESTS_H__ */
