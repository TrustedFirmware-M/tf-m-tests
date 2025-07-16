#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(CONFIG_TFM_ERPC_TEST_FRAMEWORK)
    add_subdirectory(erpc)
endif()

add_subdirectory(qcbor)
add_subdirectory(t_cose)
add_subdirectory(CMSIS)
