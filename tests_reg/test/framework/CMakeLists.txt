#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21)

add_library(tfm_test_framework_common INTERFACE)

target_sources(tfm_test_framework_common
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/test_framework.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test_framework_helpers.c
)

target_include_directories(tfm_test_framework_common
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
