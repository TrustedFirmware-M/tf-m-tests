#-------------------------------------------------------------------------------
# Copyright (c) 2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Add sfn test service.
if (TFM_PARTITION_SFN_BACKEND_TEST)
    add_subdirectory(sfn_backend_test_partition)
endif()
