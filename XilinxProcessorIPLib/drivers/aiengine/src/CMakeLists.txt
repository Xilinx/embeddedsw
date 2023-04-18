# Copyright (C) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(aiengine)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

#data
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dma/xaiedma_shim.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/dma/xaiedma_shim.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dma/xaiedma_tile.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/dma/xaiedma_tile.h")

#global
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl_defs.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl_g.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl_params.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl_reginit.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl_reginit.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl_sinit.c")

#lib
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lib/xaieconfig.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lib/xaielib.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lib/xaielib.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lib/xaielib_npi.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lib/xaielib_npi.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lib/xparameters_aie.h")

#lib/ext
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lib/ext/xaieio.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lib/ext/xaietile_proc.h")

#pm
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaiepm_clock.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaiepm_clock.h")

#tile
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_core.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_core.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_error.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_error.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_event.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_event.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_lock.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_lock.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_mem.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_mem.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_noc.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_noc.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_perfcnt.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_perfcnt.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_pl.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_pl.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_plif.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_plif.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_shim.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_shim.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_strm.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_strm.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_timer.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tile/xaietile_timer.h")
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/xaiengine.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(aiengine STATIC ${_sources})

set_target_properties(aiengine PROPERTIES LINKER_LANGUAGE C)

target_compile_options(aiengine PRIVATE -Wall -Wextra -D__AIEBAREMTL__)
