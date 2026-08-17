# Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
project(xilsfl)

find_package(common)
enable_language(C ASM)

# Same ASP switch as libxilsfl: xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION (../src/xilsfl_options.cmake).
include(${CMAKE_CURRENT_SOURCE_DIR}/../src/xilsfl_options.cmake NO_POLICY_SCOPE)

collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xilsfl)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

SET(COMMON_EXAMPLES xilsfl_ospi_flash_readwrite_example.c)
if(CONFIG_ADVANCED_SECTOR_PROTECTION_NUM EQUAL 1)
	list(APPEND COMMON_EXAMPLES xilsfl_advanced_sector_protection_example.c)
endif()

linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/../../../../scripts/linker_files/")
foreach(EXAMPLE ${COMMON_EXAMPLES})
    string(REGEX REPLACE "\\.[^.]*$" "" EXAMPLE ${EXAMPLE})
    add_executable(${EXAMPLE}.elf ${EXAMPLE})
    target_link_libraries(${EXAMPLE}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
endforeach()
