/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TEST_LOG_H__
#define __TEST_LOG_H__

#ifdef USE_SP_LOG
#include "tfm_sp_log.h"
#elif defined USE_STDIO
#include <stdio.h>
#else
#include "tfm_log_raw.h"
#endif /* USE_SP_LOG */

#ifdef __cplusplus
extern "C" {
#endif

#if defined USE_SP_LOG || USE_STDIO
#define TEST_LOG(...) printf(__VA_ARGS__)
#else
#define TEST_LOG(...) tfm_log_printf(__VA_ARGS__)
#endif /* USE_SP_LOG */

#ifdef __cplusplus
}
#endif

#endif /* __TEST_LOG_H__ */
