#-------------------------------------------------------------------------------
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if (NOT TFM_PARTITION_FIRMWARE_UPDATE)
    return()
endif()

if("${TFM_FWU_BOOTLOADER_LIB}" STREQUAL "mcuboot")
    add_subdirectory(../mcuboot/non_secure    ${CMAKE_CURRENT_BINARY_DIR}/mcuboot)
endif()
