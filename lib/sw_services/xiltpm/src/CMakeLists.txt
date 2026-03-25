# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(xiltpm)

find_package(common)

include(${CMAKE_CURRENT_SOURCE_DIR}/xiltpm.cmake NO_POLICY_SCOPE)

if ((NOT ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")) AND
	(NOT ("xilmailbox" IN_LIST BSP_LIBSRC_SUBDIRS)))
      message(FATAL_ERROR "Xiltpm library requires xilmailbox library in client mode.")
endif()

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_BINARY_DIR}/include)

add_subdirectory(common)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  add_subdirectory(server)
else()
  add_subdirectory(client)
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

if (NOT ${YOCTO})
  file(COPY ${CMAKE_BINARY_DIR}/include/xiltpm_bsp_config.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()

set(CUSTOM_COMPILE_OPTIONS " -Wall -Wextra -Warray-bounds=2 -Wformat=2 -Wno-main -Wimplicit-function-declaration -Wmissing-prototypes -Wmissing-braces -Wmultistatement-macros -Wparentheses -Wreturn-type -Wsequence-point -Wbool-compare -Wbool-operation -Wchar-subscripts -Wenum-compare -Winit-self -fno-strict-aliasing -Wtautological-compare -Wuninitialized -Wunused-function -Wunused-label -Wunused-variable -Wcast-function-type -Wenum-conversion -Wmissing-parameter-type -Wtype-limits -Wshift-negative-value -Wshadow -Wdouble-promotion -Wundef -fno-common -Wconversion -Wnull-dereference -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wswitch-bool -Wshift-overflow=2 -Wswitch-outside-range -Wswitch-unreachable -Wunused-parameter -Wduplicated-branches -Wduplicated-cond -Wdiv-by-zero -Wshadow=local -Wpointer-arith -Wunused-macros -Wconversion -Wdangling-else -Wlogical-op -Waggressive-loop-optimizations -Wstrict-prototypes -Woverflow -Wredundant-decls -Wvla")
string(APPEND CMAKE_C_FLAGS ${CUSTOM_COMPILE_OPTIONS})

add_library(xiltpm STATIC ${_sources})
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
target_compile_options(xiltpm PRIVATE -iquote ${CMAKE_BINARY_DIR}/include)
endif()

get_headers(${_headers})

set_target_properties(xiltpm PROPERTIES LINKER_LANGUAGE C)
set_target_properties(xiltpm PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/libxiltpm.a;${clean_headers}")

install(TARGETS xiltpm LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/ DESTINATION ${CMAKE_INCLUDE_PATH})