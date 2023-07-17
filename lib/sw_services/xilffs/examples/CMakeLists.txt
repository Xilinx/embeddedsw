# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
include(${CMAKE_CURRENT_SOURCE_DIR}/XilffsExample.cmake)
project(xilffs)

find_package(common)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilffs)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

SET(COMMON_EXAMPLES xilffs_polled_example.c;)

linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/../../../../scripts/linker_files/")
foreach(EXAMPLE ${COMMON_EXAMPLES})
    string(REGEX REPLACE "\\.[^.]*$" "" EXAMPLE ${EXAMPLE})
    add_executable(${EXAMPLE}.elf ${EXAMPLE})
    target_link_libraries(${EXAMPLE}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
endforeach()
