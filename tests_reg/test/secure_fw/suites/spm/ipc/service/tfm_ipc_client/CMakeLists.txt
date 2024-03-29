#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_library(tfm_app_rot_partition_ipc_client STATIC)

add_dependencies(tfm_app_rot_partition_ipc_client manifest_tool)

target_sources(tfm_app_rot_partition_ipc_client
    PRIVATE
        ./tfm_ipc_client_test.c
)

# The generated sources
target_sources(tfm_app_rot_partition_ipc_client
    PRIVATE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/test_services/tfm_ipc_client/auto_generated/intermedia_tfm_ipc_client_test.c
)

target_sources(tfm_partitions
    INTERFACE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/test_services/tfm_ipc_client/auto_generated/load_info_tfm_ipc_client_test.c
)

# Set include directory
target_include_directories(tfm_app_rot_partition_ipc_client
    PUBLIC
        .
    PRIVATE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/test_services/tfm_ipc_client
        ${SECURE_FW_REG_DIR}/common_test_services/tfm_secure_client_2
)

target_include_directories(tfm_partitions
    INTERFACE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/test_services/tfm_ipc_client
)

target_link_libraries(tfm_app_rot_partition_ipc_client
    PRIVATE
        tfm_sprt
        platform_s
)

target_link_libraries(tfm_spm
    PRIVATE
        tfm_app_rot_partition_ipc_client
)

target_compile_definitions(tfm_app_rot_partition_ipc_client
    PRIVATE
        $<$<BOOL:${TFM_ISOLATION_TEST_APP_2_PSA}>:TFM_IPC_ISOLATION_2_APP_ACCESS_PSA_MEM>
        $<$<BOOL:${TFM_ISOLATION_TEST_MEM_CHECK}>:TFM_IPC_ISOLATION_2_MEM_CHECK>
)

############################ Partition Defs ####################################

target_link_libraries(tfm_partitions
    INTERFACE
        tfm_app_rot_partition_ipc_client
)

target_compile_definitions(tfm_config
    INTERFACE
        TFM_PARTITION_TEST_CORE_IPC
)
