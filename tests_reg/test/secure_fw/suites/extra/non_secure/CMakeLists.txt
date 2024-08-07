#-------------------------------------------------------------------------------
# Copyright (c) 2021-2024, Arm Limited. All rights reserved.
# Copyright (c) 2022 Cypress Semiconductor Corporation (an Infineon company)
# or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT EXTRA_NS_TEST_SUITE_PATH)
    return()
endif()

add_library(tfm_test_suite_extra_ns STATIC EXCLUDE_FROM_ALL)

target_include_directories(tfm_test_suite_extra_ns
    PUBLIC
        .
)

target_compile_definitions(tfm_test_suite_extra_ns
    INTERFACE
        EXTRA_NS_TEST_SUITE
)

target_link_libraries(tfm_test_suite_extra_ns
    PRIVATE
        tfm_test_framework_ns
)

target_link_libraries(tfm_ns_tests
    INTERFACE
        tfm_test_suite_extra_ns
)

# Add extra test suites source from outside
add_subdirectory(${EXTRA_NS_TEST_SUITE_PATH} ${CMAKE_CURRENT_BINARY_DIR}/extra_non_secure_suite)
