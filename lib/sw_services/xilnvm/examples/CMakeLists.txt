# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
include(${CMAKE_CURRENT_SOURCE_DIR}/XilnvmExample.cmake)
project(xilnvm)

find_package(common)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
if(NOT("${CMAKE_MACHINE}" STREQUAL "spartanuplus"))
collect(PROJECT_LIB_DEPS xilmailbox)
endif()
collect(PROJECT_LIB_DEPS xilnvm)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

if("${CMAKE_MACHINE}" STREQUAL "spartanuplus")
    SET(COMMON_EXAMPLES xilnvm_efuse_spartan_ultrascale_plus_example.c;)
else()
    if("${XILNVM_mode}" STREQUAL "server")
        SET(COMMON_EXAMPLES xilnvm_bbram_versal_server_example.c;xilnvm_efuse_versal_server_example.c;)
    else()
	if ("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM")
		SET(COMMON_EXAMPLES xilnvm_bbram_versal_client_example.c;xilnvm_efuse_versal_net_client_example.c;xilnvm_bbram_config_limiter_client_example.c)
	else()
		SET(COMMON_EXAMPLES xilnvm_bbram_versal_client_example.c;xilnvm_efuse_versal_client_example.c;xilnvm_bbram_versal_ssit_client_example.c;xilnvm_efuse_versal_ssit_client_example.c)
	endif()
    endif()
endif()

linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/../../../../scripts/linker_files/")
foreach(EXAMPLE ${COMMON_EXAMPLES})
    string(REGEX REPLACE "\\.[^.]*$" "" EXAMPLE ${EXAMPLE})
    add_executable(${EXAMPLE}.elf ${EXAMPLE})
    target_link_libraries(${EXAMPLE}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
endforeach()
