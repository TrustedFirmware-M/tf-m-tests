#-------------------------------------------------------------------------------
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21)

# Default configuration of T_COSE repository
set(T_COSE_PATH     "DOWNLOAD"      CACHE PATH      "Path to t_cose (or DOWNLOAD to fetch automatically")
set(T_COSE_VERSION  "v2.0-alpha-2"  CACHE STRING    "The version of t_cose to use")

fetch_remote_library(
    LIB_NAME                t_cose
    LIB_SOURCE_PATH_VAR     T_COSE_PATH
    LIB_PATCH_DIR           ${CMAKE_CURRENT_LIST_DIR}
    LIB_BASE_DIR            "${CMAKE_BINARY_DIR}/lib/ext"
    FETCH_CONTENT_ARGS
        GIT_REPOSITORY      https://github.com/laurencelundblade/t_cose.git
        GIT_TAG             ${T_COSE_VERSION}
        GIT_SHALLOW         TRUE
        GIT_PROGRESS        TRUE
)

############################ Non Secure t_cose defs ############################
add_library(tfm_t_cose_ns_defs INTERFACE)

target_include_directories(tfm_t_cose_ns_defs
    INTERFACE
        $<BUILD_INTERFACE:${T_COSE_PATH}/inc>
        $<BUILD_INTERFACE:${T_COSE_PATH}/src>
)

target_compile_definitions(tfm_t_cose_ns_defs
    INTERFACE
        T_COSE_USE_PSA_CRYPTO
        T_COSE_DISABLE_CONTENT_TYPE
        T_COSE_DISABLE_COSE_SIGN
        T_COSE_DISABLE_KEYWRAP
        T_COSE_DISABLE_PS256
        T_COSE_DISABLE_PS384
        T_COSE_DISABLE_PS512
        T_COSE_DISABLE_SHORT_CIRCUIT_SIGN
        $<$<OR:$<NOT:$<STREQUAL:${ATTEST_KEY_BITS},384>>,$<BOOL:${SYMMETRIC_INITIAL_ATTESTATION}>>:T_COSE_DISABLE_ES384>
        $<$<OR:$<NOT:$<STREQUAL:${ATTEST_KEY_BITS},521>>,$<BOOL:${SYMMETRIC_INITIAL_ATTESTATION}>>:T_COSE_DISABLE_ES512>
        $<$<BOOL:${SYMMETRIC_INITIAL_ATTESTATION}>:T_COSE_DISABLE_SIGN_VERIFY_TESTS>
)

############################ Non Secure t_cose #################################

add_library(tfm_t_cose_ns STATIC EXCLUDE_FROM_ALL)

target_sources(tfm_t_cose_ns
    PRIVATE
        ${T_COSE_PATH}/src/t_cose_mac_compute.c
        ${T_COSE_PATH}/src/t_cose_mac_validate.c
        ${T_COSE_PATH}/src/t_cose_sign_sign.c
        ${T_COSE_PATH}/src/t_cose_sign1_sign.c
        ${T_COSE_PATH}/src/t_cose_signature_sign_main.c
        ${T_COSE_PATH}/src/t_cose_sign_verify.c
        ${T_COSE_PATH}/src/t_cose_sign1_verify.c
        ${T_COSE_PATH}/src/t_cose_signature_verify_main.c
        ${T_COSE_PATH}/crypto_adapters/t_cose_psa_crypto.c
        ${T_COSE_PATH}/src/t_cose_key.c
        ${T_COSE_PATH}/src/t_cose_parameters.c
        ${T_COSE_PATH}/src/t_cose_util.c
)

target_link_libraries(tfm_t_cose_ns
    PUBLIC
        tfm_t_cose_ns_defs
    PRIVATE
        tfm_api_ns
        tfm_qcbor_ns
)
