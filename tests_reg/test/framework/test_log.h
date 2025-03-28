/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TEST_LOG_H__
#define __TEST_LOG_H__

#ifdef USE_SP_LOG
#include "tfm_log_unpriv.h"
#elif defined USE_STDIO
#include <stdio.h>
#else
#include "tfm_log_raw.h"
#endif /* USE_SP_LOG */

#ifdef __cplusplus
extern "C" {
#endif

#if defined USE_SP_LOG
#define TEST_LOG(...) tfm_log_unpriv(LOG_MARKER_RAW __VA_ARGS__)
#elif defined USE_STDIO
#define TEST_LOG(...) printf(__VA_ARGS__)
#else
#define TEST_LOG(...) tfm_log_printf(__VA_ARGS__)
#endif /* USE_SP_LOG */

#ifdef __cplusplus
}
#endif

#endif /* __TEST_LOG_H__ */
