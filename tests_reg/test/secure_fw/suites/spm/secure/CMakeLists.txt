#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Add test suites and test service common library
add_subdirectory(../common/suites   ${CMAKE_CURRENT_BINARY_DIR}/common/suites)

# Add test suites.
add_subdirectory(../ipc/secure      ${CMAKE_CURRENT_BINARY_DIR}/ipc)
add_subdirectory(../sfn/secure      ${CMAKE_CURRENT_BINARY_DIR}/sfn)
