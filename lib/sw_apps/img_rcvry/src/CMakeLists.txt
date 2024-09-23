# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

set(APP_NAME img_rcvry)
project(${APP_NAME})

include_directories(${CMAKE_BINARY_DIR}/include)
include(${CMAKE_CURRENT_SOURCE_DIR}/UserConfig.cmake)

find_package(common)
enable_language(C ASM)

collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilffs)
collect(PROJECT_LIB_DEPS lwip220)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)

collector_list (_deps PROJECT_LIB_DEPS)
list (APPEND _deps ${USER_LINK_LIBRARIES})

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

collect (PROJECT_LIB_HEADERS xbir_config.h)
collect (PROJECT_LIB_HEADERS xbir_err.h)
collect (PROJECT_LIB_HEADERS xbir_http.h)
collect (PROJECT_LIB_HEADERS xbir_i2c.h)
collect (PROJECT_LIB_HEADERS xbir_nw.h)
collect (PROJECT_LIB_HEADERS xbir_platform.h)
collect (PROJECT_LIB_HEADERS xbir_qspi.h)
collect (PROJECT_LIB_HEADERS xbir_qspi_hw.h)
collect (PROJECT_LIB_HEADERS xbir_qspimap.h)
collect (PROJECT_LIB_HEADERS xbir_sd.h)
collect (PROJECT_LIB_HEADERS xbir_ssi.h)
collect (PROJECT_LIB_HEADERS xbir_sys.h)
collect (PROJECT_LIB_HEADERS xbir_util.h)
collect (PROJECT_LIB_HEADERS xbir_ws.h)

collect (PROJECT_LIB_SOURCES xbir_http.c)
collect (PROJECT_LIB_SOURCES xbir_i2c.c)
collect (PROJECT_LIB_SOURCES xbir_main.c)
collect (PROJECT_LIB_SOURCES xbir_nw.c)
collect (PROJECT_LIB_SOURCES xbir_platform.c)
collect (PROJECT_LIB_SOURCES xbir_qspi.c)
collect (PROJECT_LIB_SOURCES xbir_sd.c)
collect (PROJECT_LIB_SOURCES xbir_ssi.c)
collect (PROJECT_LIB_SOURCES xbir_sys.c)
collect (PROJECT_LIB_SOURCES xbir_util.c)
collect (PROJECT_LIB_SOURCES xbir_ws.c)

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

if (CMAKE_EXPORT_COMPILE_COMMANDS AND
    (NOT ${YOCTO}))
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
  set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINK_OPTIONS})

add_executable(${APP_NAME}.elf ${S_SOURCES} ${_sources})
target_link_libraries(${APP_NAME}.elf -Wl,-Map=${APP_NAME}.map -Wl,--build-id=none -Wl,--gc-sections -T\"${CMAKE_CURRENT_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_LIBRARY_PATH}/\" -L\"${USER_LINK_DIRECTORIES}/\" -Wl,--start-group ${_deps} -Wl,--end-group)

target_compile_definitions(${APP_NAME}.elf PUBLIC ${USER_COMPILE_DEFINITIONS})
if (${NON_YOCTO})
  set (INCLUDE_DIRS
      "${CMAKE_INCLUDE_PATH}/include/"
      "${CMAKE_BINARY_DIR}/include")
  target_include_directories(${APP_NAME}.elf PUBLIC ${INCLUDE_DIRS} ${USER_INCLUDE_DIRECTORIES})
endif()

print_elf_size(CMAKE_SIZE ${APP_NAME})
