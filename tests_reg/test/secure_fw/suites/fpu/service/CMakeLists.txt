#-------------------------------------------------------------------------------
# Copyright (c) 2021-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if (NOT TEST_S_FPU AND NOT TEST_NS_FPU)
    return()
endif()

add_library(tfm_app_rot_partition_fpu_service STATIC)

add_dependencies(tfm_app_rot_partition_fpu_service manifest_tool)

target_sources(tfm_app_rot_partition_fpu_service
    PRIVATE
        tfm_fpu_service_test.c
)

# The generated sources
target_sources(tfm_app_rot_partition_fpu_service
    PRIVATE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/test_services/tfm_fpu_service/auto_generated/intermedia_tfm_fpu_service_test.c
)

target_sources(tfm_partitions
    INTERFACE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/test_services/tfm_fpu_service/auto_generated/load_info_tfm_fpu_service_test.c
)

target_include_directories(tfm_app_rot_partition_fpu_service
    PUBLIC
        .
    PRIVATE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/test_services/tfm_fpu_service
        ${SECURE_FW_REG_DIR}/suites/fpu
)

target_include_directories(tfm_partitions
    INTERFACE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/test_services/tfm_fpu_service
)

target_link_libraries(tfm_app_rot_partition_fpu_service
    PRIVATE
        tfm_sprt
        platform_s
)

target_link_libraries(tfm_spm
    PRIVATE
        tfm_app_rot_partition_fpu_service
)

############################ Partition Defs ####################################

target_link_libraries(tfm_partitions
    INTERFACE
        tfm_app_rot_partition_fpu_service
)

target_compile_definitions(tfm_config
    INTERFACE
        TFM_PARTITION_FPU_TEST
)
