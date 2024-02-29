# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
include(${CMAKE_CURRENT_SOURCE_DIR}/XilskeyExample.cmake)
project(xilskey)

find_package(common)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilsecure)
collect(PROJECT_LIB_DEPS xilskey)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

if("${CMAKE_MACHINE}" STREQUAL "ZynqMP")
    SET(COMMON_EXAMPLES xilskey_bbramps_zynqmp_example.c;xilskey_efuseps_zynqmp_example.c;xilskey_puf_regeneration_example.c;xilskey_puf_registration_example.c;)
endif()

linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/../../../../scripts/linker_files/")
foreach(EXAMPLE ${COMMON_EXAMPLES})
    string(REGEX REPLACE "\\.[^.]*$" "" EXAMPLE ${EXAMPLE})
    add_executable(${EXAMPLE}.elf ${EXAMPLE})
    target_link_libraries(${EXAMPLE}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
endforeach()
