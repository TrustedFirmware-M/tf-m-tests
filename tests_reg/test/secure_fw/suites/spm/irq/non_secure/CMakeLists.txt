#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if (NOT TEST_NS_SLIH_IRQ AND NOT TEST_NS_FLIH_IRQ)
    return()
endif()

####################### Non Secure #############################################

add_library(tfm_test_suite_irq STATIC EXCLUDE_FROM_ALL)

target_sources(tfm_test_suite_irq
    PRIVATE
        ./irq_testsuite.c
        $<$<OR:$<BOOL:${TEST_NS_SLIH_IRQ}>,$<BOOL:${TEST_NS_FLIH_IRQ}>>:
            ${CMAKE_CURRENT_SOURCE_DIR}/../../common/suites/irq_test.c>
)

target_include_directories(tfm_test_suite_irq
    PUBLIC
        ./
    PRIVATE
        $<$<BOOL:${TEST_NS_SLIH_IRQ}>:${CMAKE_CURRENT_LIST_DIR}/../service/tfm_slih_test_service>
        $<$<BOOL:${TEST_NS_FLIH_IRQ}>:${CMAKE_CURRENT_LIST_DIR}/../service/tfm_flih_test_service>
)

target_compile_definitions(tfm_test_suite_irq
    PUBLIC
        $<$<BOOL:${TEST_NS_FLIH_IRQ}>:TEST_NS_FLIH_IRQ>
        $<$<BOOL:${TEST_NS_SLIH_IRQ}>:TEST_NS_SLIH_IRQ>
)

target_link_libraries(tfm_test_suite_irq
    PRIVATE
        tfm_test_framework_ns
        spm_test_common
)

target_link_libraries(tfm_ns_tests
    INTERFACE
        tfm_test_suite_irq
)
