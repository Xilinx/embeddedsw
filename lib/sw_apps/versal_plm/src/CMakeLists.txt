# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

set(APP_NAME versal_plm)
project(${APP_NAME})

include_directories(${CMAKE_BINARY_DIR}/include)
include(${CMAKE_CURRENT_SOURCE_DIR}/UserConfig.cmake)

find_package(common)
enable_language(C ASM)

collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS gloss)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilsecure)
collect(PROJECT_LIB_DEPS xilffs)
collect(PROJECT_LIB_DEPS xilpuf)
collect(PROJECT_LIB_DEPS xilpdi)
collect(PROJECT_LIB_DEPS xilplmi)
collect(PROJECT_LIB_DEPS xilloader)
collect(PROJECT_LIB_DEPS xilnvm)
collect(PROJECT_LIB_DEPS xilsem)
if("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM")
  # Use xilpm_ng for Versal_2VE_2VM
  collect(PROJECT_LIB_DEPS xilpm_ng)
else()
  collect(PROJECT_LIB_DEPS xilpm)
endif()
if("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  collect(PROJECT_LIB_DEPS xilocp)
  collect(PROJECT_LIB_DEPS xilcert)
endif()
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)

collector_list (_deps PROJECT_LIB_DEPS)
list (APPEND _deps ${USER_LINK_LIBRARIES})

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(common)
if ("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM")
  add_subdirectory(versal_2ve_2vm)
else()
  if("${CMAKE_MACHINE}" STREQUAL "Versal")
    add_subdirectory(versal)
  elseif("${CMAKE_MACHINE}" STREQUAL "VersalNet")
    add_subdirectory(versal_net)
  endif()
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

if (CMAKE_EXPORT_COMPILE_COMMANDS AND
    (NOT ${YOCTO}))
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
  set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINK_OPTIONS})
string(APPEND CMAKE_CXX_LINK_FLAGS ${USER_LINK_OPTIONS})

add_dependency_on_bsp(_sources)
add_executable(${APP_NAME}.elf ${_sources})

if("${CMAKE_MACHINE}" STREQUAL "Versal")
	set_target_properties(${APP_NAME}.elf PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/versal/lscript.ld)
	target_link_libraries(${APP_NAME}.elf -Wl,-Map=versal_plm.map -Wl,--build-id=none -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/versal/lscript.ld\" -L\"${CMAKE_LIBRARY_PATH}/\" -L\"${USER_LINK_DIRECTORIES}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
elseif("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  if ( "${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM")
    # For xilpm_ng we need to link as whole-archive since it has override symbols
    list(REMOVE_ITEM _deps xilpm_ng)
    list(APPEND _deps -Wl,--whole-archive xilpm_ng -Wl,--no-whole-archive)
    set_target_properties(${APP_NAME}.elf PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/versal_2ve_2vm/lscript.ld)
    target_link_libraries(${APP_NAME}.elf -Wl,-Map=versal_plm.map -Wl,--build-id=none -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/versal_2ve_2vm/lscript.ld\" -L\"${CMAKE_LIBRARY_PATH}/\" -L\"${USER_LINK_DIRECTORIES}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
  else()
	  set_target_properties(${APP_NAME}.elf PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/versal_net/lscript.ld)
	  target_link_libraries(${APP_NAME}.elf -Wl,-Map=versal_plm.map -Wl,--build-id=none -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/versal_net/lscript.ld\" -L\"${CMAKE_LIBRARY_PATH}/\" -L\"${USER_LINK_DIRECTORIES}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
  endif()
endif()

target_compile_definitions(${APP_NAME}.elf PUBLIC ${USER_COMPILE_DEFINITIONS})
target_include_directories(${APP_NAME}.elf PUBLIC ${USER_INCLUDE_DIRECTORIES})

print_elf_size(CMAKE_SIZE ${APP_NAME})
