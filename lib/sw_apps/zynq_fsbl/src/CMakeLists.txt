# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

set(APP_NAME zynq_fsbl)
project(${APP_NAME})

include_directories(${CMAKE_BINARY_DIR}/include)
include(${CMAKE_CURRENT_SOURCE_DIR}/UserConfig.cmake)

find_package(common)
enable_language(C ASM)

collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilffs)
collect(PROJECT_LIB_DEPS xilrsa)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)

list (APPEND _deps ${USER_LINK_LIBRARIES})
collector_list (_deps PROJECT_LIB_DEPS)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

collect (PROJECT_LIB_HEADERS fsbl_debug.h)
collect (PROJECT_LIB_HEADERS fsbl.h)
collect (PROJECT_LIB_HEADERS fsbl_hooks.h)
collect (PROJECT_LIB_HEADERS image_mover.h)
collect (PROJECT_LIB_HEADERS md5.h)
collect (PROJECT_LIB_HEADERS nand.h)
collect (PROJECT_LIB_HEADERS nor.h)
collect (PROJECT_LIB_HEADERS pcap.h)
collect (PROJECT_LIB_HEADERS qspi.h)
collect (PROJECT_LIB_HEADERS rsa.h)
collect (PROJECT_LIB_HEADERS sd.h)
collect (PROJECT_LIB_HEADERS ps7_init.h)

collect (PROJECT_LIB_SOURCES fsbl_hooks.c)
collect (PROJECT_LIB_SOURCES image_mover.c)
collect (PROJECT_LIB_SOURCES main.c)
collect (PROJECT_LIB_SOURCES md5.c)
collect (PROJECT_LIB_SOURCES nand.c)
collect (PROJECT_LIB_SOURCES nor.c)
collect (PROJECT_LIB_SOURCES pcap.c)
collect (PROJECT_LIB_SOURCES qspi.c)
collect (PROJECT_LIB_SOURCES rsa.c)
collect (PROJECT_LIB_SOURCES sd.c)
collect (PROJECT_LIB_SOURCES ps7_init.c)

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

string(APPEND CMAKE_C_FLAGS ${USER_COMPILE_OPTIONS})
string(APPEND CMAKE_CXX_FLAGS ${USER_COMPILE_OPTIONS})
string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINK_OPTIONS})
string(APPEND CMAKE_CXX_LINK_FLAGS ${USER_LINK_OPTIONS})
set_source_files_properties(${_sources} OBJECT_DEPENDS "${CMAKE_LIBRARY_PATH}/*.a")
add_executable(${APP_NAME}.elf fsbl_handoff.S ${_sources})
set_target_properties(${APP_NAME}.elf PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/lscript.ld)

target_link_libraries(${APP_NAME}.elf -Os -Wl,--gc-sections -n -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_LIBRARY_PATH}/\" -L\"${USER_LINK_DIRECTORIES}/\" -Wl,--start-group ${_deps} -Wl,--end-group)

target_compile_definitions(${APP_NAME}.elf PUBLIC ${USER_COMPILE_DEFINITIONS})
target_include_directories(${APP_NAME}.elf PUBLIC ${USER_INCLUDE_DIRECTORIES})

print_elf_size(CMAKE_SIZE ${APP_NAME})
