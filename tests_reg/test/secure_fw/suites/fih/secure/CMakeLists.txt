#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

####################### Non Secure #############################################

if(NOT TEST_S_FIH)
    return()
endif()

add_library(tfm_test_suite_fih_s STATIC EXCLUDE_FROM_ALL)

target_sources(tfm_test_suite_fih_s
    PRIVATE
        ./fih_s_test_suite.c
)

target_include_directories(tfm_test_suite_fih_s
    PUBLIC
        .
)

target_compile_definitions(tfm_test_suite_fih_s
    INTERFACE
        TEST_S_FIH
)

target_link_libraries(tfm_test_suite_fih_s
    PRIVATE
        tfm_test_framework_s
        platform_s
)

target_link_libraries(tfm_s_tests
    PRIVATE
        tfm_test_suite_fih_s
)
