#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
#
cmake_minimum_required(VERSION 3.21)

# Select the correct CMSIS RTX device header according to cpu/arch.

string(REPLACE "cortex-m" "ARMCM" rtx_device ${TFM_SYSTEM_PROCESSOR})

if(CONFIG_TFM_ENABLE_FP OR CONFIG_TFM_ENABLE_MVE_FP)
    if (TFM_SYSTEM_PROCESSOR MATCHES "cortex-m4")
        set(cmsis_device_header ${rtx_device}_FP)
    elseif(TFM_SYSTEM_PROCESSOR MATCHES "cortex-m33" OR
           TFM_SYSTEM_PROCESSOR MATCHES "cortex-35p")
        set(cmsis_device_header ${rtx_device}_DSP_FP)
    else()
        set(cmsis_device_header ${rtx_device})
    endif()
else()
    set(cmsis_device_header ${rtx_device})
endif()

if(TFM_SYSTEM_ARCHITECTURE MATCHES "armv8-m")
    set(cmsis_device_header ${cmsis_device_header}_TZ.h)
else()
    set(cmsis_device_header ${cmsis_device_header}.h)
endif()

# Select the correct IRQ handler file arch.

if(TFM_SYSTEM_ARCHITECTURE STREQUAL "armv8-m.main" OR
   TFM_SYSTEM_ARCHITECTURE STREQUAL "armv8.1-m.main")
    set(irq_source_file_name irq_armv8mml)
elseif(TFM_SYSTEM_ARCHITECTURE STREQUAL "armv8-m.base")
    set(irq_source_file_name irq_armv8mbl)
elseif(TFM_SYSTEM_ARCHITECTURE STREQUAL "armv6-m")
    set(irq_source_file_name irq_armv6m)
elseif(TFM_SYSTEM_ARCHITECTURE STREQUAL "armv7-m" OR
       TFM_SYSTEM_ARCHITECTURE STREQUAL "armv7e-m")
    set(irq_source_file_name irq_armv7m)
endif()

add_library(RTX_OS STATIC
    RTX/Source/rtx_delay.c
    RTX/Source/rtx_evflags.c
    RTX/Source/rtx_evr.c
    RTX/Source/rtx_kernel.c
    RTX/Source/rtx_lib.c
    RTX/Source/rtx_memory.c
    RTX/Source/rtx_mempool.c
    RTX/Source/rtx_msgqueue.c
    RTX/Source/rtx_mutex.c
    RTX/Source/rtx_semaphore.c
    RTX/Source/rtx_system.c
    RTX/Source/rtx_thread.c
    RTX/Source/rtx_timer.c
    RTX/os_systick.c
    Config/RTX_Config.c
    $<$<C_COMPILER_ID:GNU,Clang,ARMClang>:RTX/Source/GCC/${irq_source_file_name}.S>
    $<$<C_COMPILER_ID:IAR>:RTX/Source/IAR/${irq_source_file_name}.s>
)

target_include_directories(RTX_OS
    PUBLIC
        RTX/Include
        Config
        Core/Include
    PRIVATE
        Device
)

target_compile_definitions(RTX_OS
    PUBLIC
        RTE_COMPONENTS_H
    PRIVATE
        RTX_TZ_CONTEXT
        DOMAIN_NS=1
        CMSIS_device_header="${cmsis_device_header}"
        NDEBUG
)

target_compile_options(RTX_OS
    PRIVATE
        -fomit-frame-pointer
        -Os
        -g0
)

#target_link_libraries(RTX_OS PRIVATE platform_ns)
