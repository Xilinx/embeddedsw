# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(xilcert)

find_package(common)

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_BINARY_DIR}/include)

# only headers are included
collect (PROJECT_LIB_HEADERS xcert_createfield.h)
collect (PROJECT_LIB_HEADERS xcert_genx509cert.h)

collect (PROJECT_LIB_SOURCES xcert_createfield.c)
collect (PROJECT_LIB_SOURCES xcert_genx509cert.c)


collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

add_library(xilcert STATIC ${_sources})

get_headers(${_headers})

set_target_properties(xilcert PROPERTIES LINKER_LANGUAGE C)
set_target_properties(xilcert PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/libxilcert.a;${clean_headers}")

install(TARGETS xilcert LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
