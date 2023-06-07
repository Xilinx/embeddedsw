# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
include(${CMAKE_CURRENT_SOURCE_DIR}/XilmailboxExample.cmake)
project(xilmailbox)

find_package(common)
enable_language(C ASM)

list(LENGTH IPIPSU_NUM_DRIVER_INSTANCES _len)
if(${_len} EQUAL 0)
    message(FATAL_ERROR "This application requires an IPI instance in the hardware.")
else()
    list(GET IPIPSU0_PROP_LIST 0 XMAILBOX_IPI_BASEADDRESS)
    list(GET IPIPSU0_PROP_LIST 1 XMAILBOX_IPI_CHANNEL_ID)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../src/xilmailbox_hwconfig.h.in ${CMAKE_BINARY_DIR}/include/xilmailbox_hwconfig.h)

collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilmailbox)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

SET(COMMON_EXAMPLES xilmailbox_example.c;)

linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/../../../../scripts/linker_files/")
foreach(EXAMPLE ${COMMON_EXAMPLES})
    string(REGEX REPLACE "\\.[^.]*$" "" EXAMPLE ${EXAMPLE})
    add_executable(${EXAMPLE}.elf ${EXAMPLE})
    target_link_libraries(${EXAMPLE}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
    target_include_directories(${EXAMPLE}.elf PUBLIC "${CMAKE_BINARY_DIR}/include")
endforeach()
