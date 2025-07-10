/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include <assert.h>
#include "tfm_tests_log_msg.h"

#ifndef __ARMCC_VERSION
void __assert_func(const char *file, int line, const char *func, const char *reason)
{
    LOG_MSG("Assertion failed: (%s), file %s, line %d\r\n", reason, file, line);

    while (1) {
        ;
    }
}
#else /* __ARMCC_VERSION */
void __assert_puts(const char *msg)
{
    LOG_MSG("%s\r\n", msg);

    while (1) {
        ;
    }
}
#endif /* __ARMCC_VERSION*/
