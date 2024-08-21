# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

set(APP_NAME plm)
project(${APP_NAME})

include_directories(${CMAKE_BINARY_DIR}/include)
include(${CMAKE_CURRENT_SOURCE_DIR}/UserConfig.cmake)

find_package(common)
enable_language(C ASM)

collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)

collector_list (_deps PROJECT_LIB_DEPS)
list (APPEND _deps ${USER_LINK_LIBRARIES})

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

collect (PROJECT_LIB_SOURCES xplm_cdo.c)
collect (PROJECT_LIB_HEADERS xplm_cdo.h)

collect (PROJECT_LIB_HEADERS xplm_config.h)

collect (PROJECT_LIB_SOURCES xplm_debug.c)
collect (PROJECT_LIB_HEADERS xplm_debug.h)

collect (PROJECT_LIB_SOURCES xplm_dma.c)
collect (PROJECT_LIB_HEADERS xplm_dma.h)

collect (PROJECT_LIB_SOURCES xplm_error.c)
collect (PROJECT_LIB_HEADERS xplm_error.h)

collect (PROJECT_LIB_SOURCES xplm_generic.c)
collect (PROJECT_LIB_HEADERS xplm_generic.h)

collect (PROJECT_LIB_HEADERS xplm_hooks.h)

collect (PROJECT_LIB_SOURCES xplm_init.c)
collect (PROJECT_LIB_HEADERS xplm_init.h)

collect (PROJECT_LIB_SOURCES xplm_load.c)
collect (PROJECT_LIB_HEADERS xplm_load.h)

collect (PROJECT_LIB_SOURCES xplm_main.c)

collect (PROJECT_LIB_SOURCES xplm_ospi.c)
collect (PROJECT_LIB_HEADERS xplm_ospi.h)

collect (PROJECT_LIB_SOURCES xplm_post_boot.c)
collect (PROJECT_LIB_HEADERS xplm_post_boot.h)

collect (PROJECT_LIB_SOURCES xplm_qspi.c)
collect (PROJECT_LIB_HEADERS xplm_qspi.h)

collect (PROJECT_LIB_SOURCES xplm_sbi.c)
collect (PROJECT_LIB_HEADERS xplm_sbi.h)

collect (PROJECT_LIB_HEADERS xplm_status.h)

collect (PROJECT_LIB_SOURCES xplm_util.c)
collect (PROJECT_LIB_HEADERS xplm_util.h)


collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
  set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINK_OPTIONS})

add_executable(${APP_NAME}.elf ${_sources})

target_link_libraries(${APP_NAME}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_LIBRARY_PATH}/\" -L\"${USER_LINK_DIRECTORIES}/\" -Wl,--start-group ${_deps} -Wl,--end-group)

target_compile_definitions(${APP_NAME}.elf PUBLIC ${USER_COMPILE_DEFINITIONS})
target_include_directories(${APP_NAME}.elf PUBLIC ${USER_INCLUDE_DIRECTORIES})

print_elf_size(CMAKE_SIZE ${APP_NAME})
