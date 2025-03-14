#-------------------------------------------------------------------------------
# Copyright (c) 2023-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21)

project("ERPC Host" LANGUAGES CXX C)

add_executable(erpc_main)

add_subdirectory(../client client)

# Include exported targets as we need psa_crypto_config
include(${CONFIG_SPE_PATH}/cmake/spe_export.cmake)

if (ERPC_TRANSPORT STREQUAL "UART")
    if (NOT DEFINED PORT_NAME)
        message(FATAL_ERROR "Please provide PORT_NAME!")
    endif()
elseif (ERPC_TRANSPORT STREQUAL "TCP")
    if((NOT DEFINED ERPC_HOST) OR (NOT DEFINED ERPC_PORT))
        message(FATAL_ERROR "Please provide ERPC_HOST and ERPC_PORT!")
    endif()
elseif (DEFINED ERPC_TRANSPORT)
    message(FATAL_ERROR "Please provided supported transportation type (UART and TCP)!")
endif()

target_sources(erpc_main
    PRIVATE
        main.c
        ${ERPC_REPO_PATH}/erpc_c/port/erpc_threading_pthreads.cpp
        ${ERPC_REPO_PATH}/erpc_c/setup/erpc_setup_serial.cpp
        ${ERPC_REPO_PATH}/erpc_c/transports/erpc_serial_transport.cpp
        ${ERPC_REPO_PATH}/erpc_c/setup/erpc_setup_tcp.cpp
        ${ERPC_REPO_PATH}/erpc_c/transports/erpc_tcp_transport.cpp
        ${CONFIG_SPE_PATH}/interface/src/tfm_crypto_api.c
)

target_link_libraries(erpc_main
    PRIVATE
        erpc_client
        pthread
    PUBLIC
        psa_crypto_config
)

target_compile_definitions(erpc_main
    PRIVATE
        $<$<STREQUAL:${ERPC_TRANSPORT},UART>:ERPC_TRANSPORT_UART>
        $<$<STREQUAL:${ERPC_TRANSPORT},TCP>:ERPC_TRANSPORT_TCP>
        $<$<AND:$<STREQUAL:${ERPC_TRANSPORT},UART>,$<BOOL:${PORT_NAME}>>:PORT_NAME="${PORT_NAME}">
        $<$<AND:$<STREQUAL:${ERPC_TRANSPORT},TCP>,$<BOOL:${ERPC_HOST}>>:ERPC_HOST="${ERPC_HOST}">
        $<$<AND:$<STREQUAL:${ERPC_TRANSPORT},TCP>,$<BOOL:${ERPC_PORT}>>:ERPC_PORT=${ERPC_PORT}>
        PLATFORM_DEFAULT_CRYPTO_KEYS
)
