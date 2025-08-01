#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21)

# Default configuration of QCBOR repository
set(QCBOR_PATH      "DOWNLOAD"  CACHE PATH      "Path to qcbor (or DOWNLOAD to fetch automatically")
set(QCBOR_VERSION   "v1.2"      CACHE STRING    "The version of qcbor to use")

# Entirely disable the floating-point support
# in the qcbor library to reduce code size.
set(QCBOR_OPT_DISABLE_FLOAT_HW_USE     ON  CACHE BOOL  "Eliminate dependency on FP hardware and FP instructions" FORCE)
set(QCBOR_OPT_DISABLE_FLOAT_PREFERRED  ON  CACHE BOOL  "Eliminate support for half-precision and CBOR preferred serialization" FORCE)
set(QCBOR_OPT_DISABLE_FLOAT_ALL        ON  CACHE BOOL  "Eliminate floating-point support completely" FORCE)

fetch_remote_library(
    LIB_NAME                qcbor
    LIB_SOURCE_PATH_VAR     QCBOR_PATH
    LIB_PATCH_DIR           ${CMAKE_CURRENT_LIST_DIR}
    LIB_BASE_DIR            "${CMAKE_BINARY_DIR}/lib/ext"
    FETCH_CONTENT_ARGS
        GIT_REPOSITORY      https://github.com/laurencelundblade/QCBOR.git
        GIT_TAG             ${QCBOR_VERSION}
        GIT_SHALLOW         TRUE
        GIT_PROGRESS        TRUE
)

# Adding tfm_qcbor_ns library manually as logical target name
# must be globally unique in CMake. The Secure image also relies on the qcbor
# library (including its own CMake files), but a separate Non Secure library
# target must be maintained (with a unique name) as they may be built
# with different configurations - within the same CMake environment.

####################### Non Secure qcbor library ###############################

add_library(tfm_qcbor_ns STATIC EXCLUDE_FROM_ALL)

target_sources(tfm_qcbor_ns
    PRIVATE
        ${QCBOR_PATH}/src/ieee754.c
        ${QCBOR_PATH}/src/qcbor_encode.c
        ${QCBOR_PATH}/src/qcbor_decode.c
        ${QCBOR_PATH}/src/UsefulBuf.c
)

target_include_directories(tfm_qcbor_ns
    PUBLIC
        $<BUILD_INTERFACE:${QCBOR_PATH}/inc>
        ${CONFIG_SPE_PATH}/qcbor/inc
    PRIVATE
        $<BUILD_INTERFACE:${QCBOR_PATH}/src>
)

target_compile_definitions(tfm_qcbor_ns
    PRIVATE
        $<$<BOOL:${QCBOR_OPT_DISABLE_FLOAT_HW_USE}>:QCBOR_DISABLE_FLOAT_HW_USE>
        $<$<BOOL:${QCBOR_OPT_DISABLE_FLOAT_PREFERRED}>:QCBOR_DISABLE_PREFERRED_FLOAT>
        $<$<BOOL:${QCBOR_OPT_DISABLE_FLOAT_ALL}>:USEFULBUF_DISABLE_ALL_FLOAT>
)

# The math library is needed for floating-point support.
# To avoid need for it #define QCBOR_DISABLE_FLOAT_HW_USE
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # Using GCC
    target_link_libraries(tfm_qcbor_ns
        PRIVATE
            $<$<NOT:$<BOOL:${QCBOR_OPT_DISABLE_FLOAT_HW_USE}>>:m>
    )
endif()
