#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if (NOT TEST_NS_T_COSE)
    return()
endif()

############################ t_cose test library ###############################

add_library(tfm_t_cose_test STATIC EXCLUDE_FROM_ALL)

target_sources(tfm_t_cose_test
    PRIVATE
        ${T_COSE_PATH}/test/run_tests.c
        ${T_COSE_PATH}/test/t_cose_test.c
        ${T_COSE_PATH}/test/t_cose_compute_validate_mac_test.c
        $<$<NOT:$<BOOL:${SYMMETRIC_INITIAL_ATTESTATION}>>:${T_COSE_PATH}/test/t_cose_sign_verify_test.c>
        ${T_COSE_PATH}/test/t_cose_crypto_test.c
        ${T_COSE_PATH}/examples/example_keys.c
        ${T_COSE_PATH}/examples/init_keys_psa.c
)

target_include_directories(tfm_t_cose_test
    PUBLIC
        $<BUILD_INTERFACE:${T_COSE_PATH}/test>
    PRIVATE
        $<BUILD_INTERFACE:${T_COSE_PATH}/examples>
)

target_link_libraries(tfm_t_cose_test
    PRIVATE
        tfm_api_ns
        tfm_t_cose_ns
        tfm_qcbor_ns
)

####################### Non Secure test ########################################

add_library(tfm_test_suite_t_cose_ns STATIC EXCLUDE_FROM_ALL)

target_sources(tfm_test_suite_t_cose_ns
    PRIVATE
        t_cose_ns_testsuite.c
)

target_include_directories(tfm_test_suite_t_cose_ns
    PUBLIC
        .
)

target_compile_definitions(tfm_test_suite_t_cose_ns
    PRIVATE
        DOMAIN_NS=1
    INTERFACE
        TEST_NS_T_COSE
)

target_link_libraries(tfm_test_suite_t_cose_ns
    PRIVATE
        tfm_test_framework_ns
        tfm_t_cose_test
)

target_link_libraries(tfm_ns_tests
    INTERFACE
        tfm_test_suite_t_cose_ns
)
