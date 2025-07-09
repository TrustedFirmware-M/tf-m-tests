#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT TEST_BL1_1)
    return()
endif()

add_library(bl1_1_tests STATIC)

target_link_libraries(tfm_test_framework_common
    INTERFACE
        platform_bl1_1_interface
)

add_subdirectory(suites/crypto)
add_subdirectory(suites/random)
add_subdirectory(suites/integration)
add_subdirectory(suites/extra)

target_sources(bl1_1_tests
    PRIVATE
        ./bl1_1_suites.c
)

target_include_directories(bl1_1_tests
    PUBLIC
        interface
)

target_link_libraries(bl1_1_tests
    PRIVATE
        tfm_test_framework_common
        platform_bl1_1_interface
        bl1_1_shared_lib_interface
        bl1_1_test_suite_crypto
        bl1_1_test_suite_random
        bl1_1_test_suite_integration
        ${BL1_1_PLATFORM_SPECIFIC_LINK_LIBRARIES}
)
