# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.14.7)
include_directories(${CMAKE_BINARY_DIR}/include)
include(${CMAKE_CURRENT_SOURCE_DIR}/UserConfig.cmake)
set(APP_NAME zynqmp_pmufw)
project(${APP_NAME})

find_package(common)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilsecure)
collect(PROJECT_LIB_DEPS xilfpga)
collect(PROJECT_LIB_DEPS xilskey)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)

set(_lib "xilstandalone")
list (APPEND _lib "xil")
list (APPEND _lib "xilsecure")
list (APPEND _lib "xilfpga")
list (APPEND _lib "xilskey")
list (APPEND _lib "gcc")
list (APPEND _lib "c")
find_library(_lib_path ${_lib})

collector_list (_deps PROJECT_LIB_DEPS)
list (APPEND _deps ${USER_LINK_LIBRARIES})

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${D}${libdir})

if(CMAKE_EXPORT_COMPILE_COMMANDS)
	set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
	set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

aux_source_directory(${CMAKE_SOURCE_DIR} _sources)
string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINK_OPTIONS})
string(APPEND CMAKE_CXX_LINK_FLAGS ${USER_LINK_OPTIONS})
set_source_files_properties(${_sources} OBJECT_DEPENDS "${CMAKE_LIBRARY_PATH}/*.a")
add_executable(${APP_NAME}.elf xpfw_start.S ${_sources})
set_target_properties(${APP_NAME}.elf PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/lscript.ld)
target_link_libraries(${APP_NAME}.elf -Wl,--no-relax -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v9.2 -mxl-soft-mul -Os -flto -ffat-lto-objects -Wl,--gc-sections -nostartfiles -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_LIBRARY_PATH}/\" -L\"${USER_LINK_DIRECTORIES}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
target_compile_definitions(${APP_NAME}.elf PUBLIC ${USER_COMPILE_DEFINITIONS})
target_include_directories(${APP_NAME}.elf PUBLIC ${USER_INCLUDE_DIRECTORIES})
print_elf_size(CMAKE_SIZE ${APP_NAME})
