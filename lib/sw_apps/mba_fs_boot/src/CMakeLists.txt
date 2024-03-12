# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

set(APP_NAME mba_fs_boot)
project(${APP_NAME})

include_directories(${CMAKE_BINARY_DIR}/include)
include(${CMAKE_CURRENT_SOURCE_DIR}/UserConfig.cmake)

find_package(common)
enable_language(C ASM)

collect(PROJECT_LIB_DEPS xiltimer)
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

collect (PROJECT_LIB_HEADERS fs-boot.h)
collect (PROJECT_LIB_HEADERS fs-xspi.h)
collect (PROJECT_LIB_HEADERS spi-flash.h)

collect (PROJECT_LIB_SOURCES fs-boot.c)
collect (PROJECT_LIB_SOURCES fs-xspi.c)
collect (PROJECT_LIB_SOURCES winbond-spi-flash.c)


collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

string(APPEND CMAKE_C_FLAGS ${USER_COMPILE_OPTIONS})
string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINK_OPTIONS})
add_executable(${APP_NAME}.elf ${_sources})

target_link_libraries(${APP_NAME}.elf -Os -Wl,--gc-sections -n -T\"${USER_LINKER_SCRIPT}\" -L\"${CMAKE_LIBRARY_PATH}/\" -L\"${USER_LINK_DIRECTORIES}/\" -Wl,--start-group ${_deps} -Wl,--end-group)

target_compile_definitions(${APP_NAME}.elf PUBLIC ${USER_COMPILE_DEFINITIONS})
target_include_directories(${APP_NAME}.elf PUBLIC ${USER_INCLUDE_DIRECTORIES})

print_elf_size(CMAKE_SIZE ${APP_NAME})
