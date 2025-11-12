#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Collect arguments via command line.
# cmd_line: the output argument to collect the arguments via command line
#
# Those command line arguments will be passed to ExternalProject_Add().
# Those arguments shall not be populated by the settings parsed inside each ExternalProject_Add()
function(collect_build_cmd_args cmd_line)

    get_cmake_property(CACHE_ARGS CACHE_VARIABLES)
    foreach(CACHE_ARG ${CACHE_ARGS})
        get_property(ARG_HELPSTRING CACHE "${CACHE_ARG}" PROPERTY HELPSTRING)
        if("${ARG_HELPSTRING}" MATCHES "variable specified on the command line")
            get_property(CACHE_ARG_TYPE CACHE ${CACHE_ARG} PROPERTY TYPE)
            set(ARG_VAL ${${CACHE_ARG}})

            # CMake automatically converts relative paths passed via command line into absolute
            # ones. Since external projects have different base directories compared to root
            # directory, relative paths will be incorrectly converted inside external projects.
            # Enforce all the relative paths into absolute paths before collecting them in the
            # build command argument list.
            if(NOT "${ARG_VAL}" STREQUAL "")
                set(ABS_PATH_LIST)
                # Handle list values and normalize relative paths per-item.
                foreach(_v IN LISTS ARG_VAL)
                    # Perform path conversion only for the variables with PATH or
                    # UNINITIALIZED type to avoid unnecessary conversions for other
                    # types (e.g. BOOL, STRING, etc.)
                    if((CACHE_ARG_TYPE STREQUAL "PATH" OR
                        CACHE_ARG_TYPE STREQUAL "UNINITIALIZED") AND
                       (IS_DIRECTORY ${_v} AND NOT IS_ABSOLUTE ${_v}))
                        get_filename_component(ABS_PATH ${_v} ABSOLUTE)
                        list(APPEND ABS_PATH_LIST ${ABS_PATH})
                    else()
                        list(APPEND ABS_PATH_LIST ${_v})
                    endif()
                endforeach()
                set(ARG_VAL "${ABS_PATH_LIST}")
            endif()

            # Keep the -D entry as a single list element even if ARG_VAL contains semicolons, and
            # the list elements will be properly separated by ";" in the result.
            string(REPLACE ";" "\\;" ARG_VAL_ESC "${ARG_VAL}")
            list(APPEND TEMP_CMD_LINE "-D${CACHE_ARG}:${CACHE_ARG_TYPE}=${ARG_VAL_ESC}")
        endif()
    endforeach()

    set(${cmd_line} ${TEMP_CMD_LINE} PARENT_SCOPE)
endfunction()
