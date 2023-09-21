# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

set(APP_NAME zynqmp_fsbl)
project(${APP_NAME})

include_directories(${CMAKE_BINARY_DIR}/include)
include(${CMAKE_CURRENT_SOURCE_DIR}/UserConfig.cmake)

find_package(common)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilpm)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilffs)
collect(PROJECT_LIB_DEPS xilsecure)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
list (APPEND _deps ${USER_LINK_LIBRARIES})

collector_list (_deps PROJECT_LIB_DEPS)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/psu_init.h defs)
string(FIND "${defs}" "xfsbl_hw.h" _index)
if (${_index} EQUAL -1)
list(PREPEND defs "#include \"xfsbl_hw.h\"\n\n")
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/psu_init.h "${defs}")
endif()
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
  set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

aux_source_directory(${CMAKE_SOURCE_DIR} _sources)

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
  set(S_SOURCES xfsbl_exit.S xfsbl_translation_table_a53_64.S)
else()
  set(S_SOURCES xfsbl_exit.S)
endif()

string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINK_OPTIONS})
string(APPEND CMAKE_CXX_LINK_FLAGS ${USER_LINK_OPTIONS})

set_source_files_properties(${_sources} OBJECT_DEPENDS "${CMAKE_LIBRARY_PATH}/*.a")
add_executable(${APP_NAME}.elf ${S_SOURCES} ${_sources})

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
  set_target_properties(${APP_NAME}.elf PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/lscript_a53.ld)
  target_link_libraries(${APP_NAME}.elf -Os -flto -ffat-lto-objects -Wl,--gc-sections -n -T\"${CMAKE_SOURCE_DIR}/lscript_a53.ld\" -L\"${CMAKE_LIBRARY_PATH}/\" -L\"${USER_LINK_DIRECTORIES}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
else()
  set_target_properties(${APP_NAME}.elf PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/lscript.ld)
  target_link_libraries(${APP_NAME}.elf -Os -flto -ffat-lto-objects -Wl,--gc-sections -n -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_LIBRARY_PATH}/\" -L\"${USER_LINK_DIRECTORIES}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
endif()

target_compile_definitions(${APP_NAME}.elf PUBLIC ${USER_COMPILE_DEFINITIONS})
target_include_directories(${APP_NAME}.elf PUBLIC ${USER_INCLUDE_DIRECTORIES})

print_elf_size(CMAKE_SIZE ${APP_NAME})
