# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(xilskey)

find_package(common)
include(${CMAKE_CURRENT_SOURCE_DIR}/xilskey.cmake NO_POLICY_SCOPE)

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_BINARY_DIR}/include)

enable_language(C ASM)

add_subdirectory(include)

collect (PROJECT_LIB_SOURCES xilskey_eps_zynqmp.c)
collect (PROJECT_LIB_HEADERS xilskey_eps_zynqmp_hw.h)
collect (PROJECT_LIB_SOURCES xilskey_utils.c)
collect (PROJECT_LIB_HEADERS xilskey_bbramps_zynqmp_hw.h)

if(NOT ${XILSKEY_use_puf_hd_as_user_efuse})
  collect (PROJECT_LIB_SOURCES xilskey_eps_zynqmp_puf.c)
endif()

if(NOT("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
  collect (PROJECT_LIB_SOURCES xilskey_bbramps_zynqmp.c)
endif()

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9") OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze"))
  collect (PROJECT_LIB_SOURCES xilskey_epl.c)
  collect (PROJECT_LIB_SOURCES xilskey_eps.c)
  collect (PROJECT_LIB_HEADERS xilskey_epshw.h)
  collect (PROJECT_LIB_HEADERS xilskey_js.h)
  collect (PROJECT_LIB_SOURCES xilskey_jscmd.c)
  collect (PROJECT_LIB_HEADERS xilskey_jscmd.h)
  collect (PROJECT_LIB_SOURCES xilskey_jslib.c)
  collect (PROJECT_LIB_HEADERS xilskey_jslib.h)
  collect (PROJECT_LIB_HEADERS xilskey_jtag.h)
  collect (PROJECT_LIB_SOURCES xilskey_bbram.c)
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
if(NOT ${YOCTO})
file(COPY ${CMAKE_BINARY_DIR}/include/xilskey_bsp_config.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()

add_library(xilskey STATIC ${_sources})

get_headers(${_headers})

set_target_properties(xilskey PROPERTIES LINKER_LANGUAGE C)
set_target_properties(xilskey PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/libxilskey.a;${CMAKE_INCLUDE_PATH}/xilskey_bsp_config.h;${clean_headers}")

install(TARGETS xilskey LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
