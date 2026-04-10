# Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
include(${CMAKE_CURRENT_SOURCE_DIR}/Lwip220Example.cmake)
project(lwip220)

find_package(common)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS lwip220)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)

SET(DRIVER_INSTANCES "${NUM_DRIVER_INSTANCES}" CACHE STRING "Driver Instances")
SET_PROPERTY(CACHE DRIVER_INSTANCES PROPERTY STRINGS "${NUM_DRIVER_INSTANCES}")
set(index 0)
LIST_INDEX(${index} ${DRIVER_INSTANCES} "${NUM_DRIVER_INSTANCES}")
list(GET TOTAL_EXAMPLE_LIST ${index} ex_list)
list(GET REG_LIST ${index} reg)
SET(COMMON_FILE lwip_example_iic_phyreset.c)
SET(EXAMPLE_LIST "${${ex_list}}" CACHE STRING "Driver Example List")
SET_PROPERTY(CACHE EXAMPLE_LIST PROPERTY STRINGS "${${ex_list}}")

if (EXAMPLE_LIST MATCHES "_ws_|_tftp|_webserver")
    collect(PROJECT_LIB_DEPS xilffs)
endif()

collector_list (_deps PROJECT_LIB_DEPS)

set(valid_ex 0)
foreach(LIST1 ${TOTAL_EXAMPLE_LIST})
    if ("${${LIST1}}" STREQUAL "${EXAMPLE_LIST}")
	set(valid_ex 1)
	break()
    endif()
endforeach()

linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/linker_files/")

if (${valid_ex})
    list(APPEND EXAMPLES "${${ex_list}}")
else()
    list(APPEND EXAMPLES ${EXAMPLE_LIST})
endif()

foreach(EXAMPLE ${EXAMPLES})
    string(REGEX REPLACE "\\.[^.]*$" "" EXAMPLE ${EXAMPLE})
    add_executable(${EXAMPLE}.elf ${EXAMPLE} ${COMMON_FILE})
    target_link_libraries(${EXAMPLE}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
endforeach()
