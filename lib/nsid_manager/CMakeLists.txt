#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
#
cmake_minimum_required(VERSION 3.21)

# NSID manager
if (NOT TFM_NS_MANAGE_NSID)
    add_library(tfm_nsid_manager INTERFACE)
    target_include_directories(tfm_nsid_manager INTERFACE .)
else()
    add_library(tfm_nsid_manager STATIC
            tfm_nsid_manager.c
            tfm_nsid_map_table.c
    )

    target_include_directories(tfm_nsid_manager
        PUBLIC
            .
    )

    target_compile_definitions(tfm_nsid_manager
        PUBLIC
            TFM_NS_MANAGE_NSID
            $<$<BOOL:${TEST_NS_MANAGE_NSID}>:TEST_NS_MANAGE_NSID>
    )

    target_link_libraries(tfm_nsid_manager
        PRIVATE
            tfm_api_ns
    )
endif()
