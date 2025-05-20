#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_library(bl1_1_test_suite_random)

target_sources(bl1_1_test_suite_random
    PRIVATE
        bl1_1_random_generation_tests.c
)

target_include_directories(bl1_1_test_suite_random
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(bl1_1_test_suite_random
    PRIVATE
        tfm_test_framework_common
        bl1_1_shared_lib_interface
        ${BL1_1_PLATFORM_SPECIFIC_LINK_LIBRARIES}
)
