#-------------------------------------------------------------------------------
# Copyright (c) 2020-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT TEST_NS_ATTESTATION)
    return()
endif()

add_library(tfm_test_suite_attestation_ns STATIC EXCLUDE_FROM_ALL)

target_sources(tfm_test_suite_attestation_ns
    PRIVATE
        ../attest_token_test.c
        ../attest_token_decode_common.c
        ../ext/qcbor_util/qcbor_util.c
        $<$<BOOL:${SYMMETRIC_INITIAL_ATTESTATION}>:../attest_token_decode_symmetric.c>
        $<$<BOOL:${SYMMETRIC_INITIAL_ATTESTATION}>:attest_symmetric_ns_interface_testsuite.c>
        $<$<NOT:$<BOOL:${SYMMETRIC_INITIAL_ATTESTATION}>>:../attest_token_decode_asymmetric.c>
        $<$<NOT:$<BOOL:${SYMMETRIC_INITIAL_ATTESTATION}>>:attest_asymmetric_ns_interface_testsuite.c>
)

target_include_directories(tfm_test_suite_attestation_ns
    PUBLIC
        .
        ../ext/qcbor_util
    PRIVATE
        ../
        ${CONFIG_SPE_PATH}/initial_attestation
        ${CONFIG_SPE_PATH}/t_cose/inc
)

target_compile_definitions(tfm_test_suite_attestation_ns
    PRIVATE
        DOMAIN_NS=1
        $<$<BOOL:${SYMMETRIC_INITIAL_ATTESTATION}>:SYMMETRIC_INITIAL_ATTESTATION>
    INTERFACE
        TEST_NS_ATTESTATION
)

target_link_libraries(tfm_test_suite_attestation_ns
    PRIVATE
        tfm_qcbor_ns
        tfm_t_cose_ns
        tfm_test_framework_ns
)

target_link_libraries(tfm_ns_tests
    INTERFACE
        tfm_test_suite_attestation_ns
)
