# Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(xilsecure)

find_package(common)

include(${CMAKE_CURRENT_SOURCE_DIR}/xilsecure.cmake NO_POLICY_SCOPE)

collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

foreach(lang ASM C CXX)
  # GNU ar always updates the index: no need for CMAKE_RANLIB
  SET(CMAKE_${lang}_CREATE_STATIC_LIBRARY
          "<CMAKE_AR> rc <TARGET> <LINK_FLAGS> <OBJECTS>")
endforeach()

include_directories(${CMAKE_BINARY_DIR}/include)

if(("${XILSECURE_mode}" STREQUAL "server") AND
   ((("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") AND (NOT("${CMAKE_MACHINE}" STREQUAL "spartanuplus") AND NOT("${CMAKE_MACHINE}" STREQUAL "Versal"))) OR
   (("${CMAKE_MACHINE}" STREQUAL "VersalNet") AND (NOT ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze"))))
  )
  message(FATAL_ERROR "XilSecure library is not supported for selected ${CMAKE_SYSTEM_PROCESSOR} processor in ${XILSECURE_mode} mode")
endif()

if(("${XILSECURE_mode}" STREQUAL "client") AND
	(NOT ("xilmailbox" IN_LIST BSP_LIBSRC_SUBDIRS)))
    message(WARNING "XilSecure library requires xilmailbox library in client mode.")
endif()

collect (PROJECT_LIB_HEADERS xsecure_ver.h)

if((("${CMAKE_MACHINE}" STREQUAL "Versal") OR
	("${CMAKE_MACHINE}" STREQUAL "VersalNet")) AND
	("${XILSECURE_mode}" STREQUAL "client"))
  add_subdirectory(client)
  add_subdirectory(common)
elseif ((("${CMAKE_MACHINE}" STREQUAL "Versal") OR
        ("${CMAKE_MACHINE}" STREQUAL "VersalNet")) AND
        ("${XILSECURE_mode}" STREQUAL "server"))
   add_subdirectory(server)
   add_subdirectory(common)
elseif(("${CMAKE_MACHINE}" STREQUAL "ZynqMP") OR
      ("${CMAKE_MACHINE}" STREQUAL "spartanuplus"))
   add_subdirectory(server)
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)

if(NOT ${YOCTO})
  file(COPY ${CMAKE_BINARY_DIR}/include/xsecure_config.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()

set(CUSTOM_COMPILE_OPTIONS " -Wall -Wextra -Warray-bounds=2 -Wformat=2 -Wno-main -Wimplicit-function-declaration -Wmissing-prototypes -Wmissing-braces -Wmultistatement-macros -Wparentheses -Wreturn-type -Wsequence-point -Wbool-compare -Wbool-operation -Wchar-subscripts -Wenum-compare -Winit-self -fno-strict-aliasing -Wtautological-compare -Wuninitialized -Wunused-function -Wunused-label -Wunused-variable -Wcast-function-type -Wenum-conversion -Wmissing-parameter-type -Wtype-limits -Wshift-negative-value -Wshadow -Wdouble-promotion -Wundef -fno-common -Wconversion -Wnull-dereference -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wswitch-bool -Wshift-overflow=2 -Wswitch-outside-range -Wswitch-unreachable -Wunused-parameter -Wduplicated-branches -Wduplicated-cond -Wdiv-by-zero -Wshadow=local -Wpointer-arith -Wunused-macros -Wconversion -Wdangling-else -Wlogical-op -Waggressive-loop-optimizations -Wstrict-prototypes -Woverflow -Wredundant-decls -Wvla")
string(APPEND CMAKE_C_FLAGS ${CUSTOM_COMPILE_OPTIONS})

# MicroBlaze-only flags for the PLM (server) build; not applicable to ARM client builds
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  string(APPEND CMAKE_C_FLAGS " -mlittle-endian")
endif()

add_library(xilsecure STATIC ${_sources})
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
target_compile_options(xilsecure PRIVATE -iquote ${CMAKE_BINARY_DIR}/include)
endif()

get_headers(${_headers})

set_target_properties(xilsecure PROPERTIES LINKER_LANGUAGE C)

set_target_properties(xilsecure PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/libxilsecure.a;${CMAKE_INCLUDE_PATH}/xsecure_config.h;${clean_headers}")

install(TARGETS xilsecure LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/ DESTINATION ${CMAKE_INCLUDE_PATH})
