#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21)

add_custom_target(docs)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(Sphinx)

################################## ENV #########################################

project("TF-M Tests Documentation" LANGUAGES)

set(SPHINXCFG_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINXCFG_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINXCFG_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

################################## SPHINX ######################################

file(GLOB_RECURSE SPHINXCFG_DOC_FILES ${SPHINXCFG_SOURCE_PATH}/*.rst ${SPHINXCFG_SOURCE_PATH}/*.md)

add_custom_target(tfm_test_docs_html ALL
    COMMAND "${SPHINX_EXECUTABLE}" -b html -j 1 -c ${SPHINXCFG_CONFIG_PATH} ${SPHINXCFG_SOURCE_PATH} "${SPHINXCFG_OUTPUT_PATH}/html"
    WORKING_DIRECTORY ${SPHINXCFG_SOURCE_PATH}
    DEPENDS ${SPHINXCFG_DOC_FILES}
)
add_dependencies(docs tfm_test_docs_html)

