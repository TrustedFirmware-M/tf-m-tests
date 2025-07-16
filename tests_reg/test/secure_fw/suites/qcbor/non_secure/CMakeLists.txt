#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Adding tfm_qcbor_test library manually as logical target name
# must be globally unique in CMake.

####################### qcbor test library #####################################

if (NOT TEST_NS_QCBOR)
    return()
endif()

add_library(tfm_qcbor_test STATIC EXCLUDE_FROM_ALL)

target_sources(tfm_qcbor_test
    PRIVATE
        ${QCBOR_PATH}/test/qcbor_decode_tests.c
        ${QCBOR_PATH}/test/qcbor_encode_tests.c
        ${QCBOR_PATH}/test/run_tests.c
        ${QCBOR_PATH}/test/UsefulBuf_Tests.c
)

target_include_directories(tfm_qcbor_test
    PUBLIC
        $<BUILD_INTERFACE:${QCBOR_PATH}/test>
    PRIVATE
        $<BUILD_INTERFACE:${QCBOR_PATH}/inc>
)

target_link_libraries(tfm_qcbor_test
    PRIVATE
        tfm_qcbor_ns
)

# Note: All floating point support is disabled in tfm_qcbor_ns library
#       to reduce code size. These compile options are public so the qcbor
#       test library will also get them. It means that effectively
#       the floating point test cases are also disabled.

####################### Non Secure test ########################################

if (NOT TEST_NS_QCBOR)
    return()
endif()

add_library(tfm_test_suite_qcbor_ns STATIC EXCLUDE_FROM_ALL)

target_sources(tfm_test_suite_qcbor_ns
    PRIVATE
        qcbor_ns_testsuite.c
)

target_include_directories(tfm_test_suite_qcbor_ns
    PUBLIC
        .
)

target_compile_definitions(tfm_test_suite_qcbor_ns
    PRIVATE
        DOMAIN_NS=1
    INTERFACE
        TEST_NS_QCBOR
)

target_link_libraries(tfm_test_suite_qcbor_ns
    PRIVATE
        tfm_test_framework_ns
        tfm_qcbor_test
)

target_link_libraries(tfm_ns_tests
    INTERFACE
        tfm_test_suite_qcbor_ns
)
