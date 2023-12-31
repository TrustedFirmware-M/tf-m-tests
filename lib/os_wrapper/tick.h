/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 * Copyright (c) 2023 Cypress Semiconductor Corporation (an Infineon company)
 * or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __OS_WRAPPER_TICK_H__
#define __OS_WRAPPER_TICK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os_wrapper/common.h"

/**
 * \brief Return RTOS current tick count
 *
 * \return The current tick count
 */
uint32_t os_wrapper_get_tick(void);

#ifdef __cplusplus
}
#endif

#endif /* __OS_WRAPPER_TICK_H__ */
