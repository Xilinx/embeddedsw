# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
include(${CMAKE_CURRENT_SOURCE_DIR}/Lwip213Example.cmake NO_POLICY_SCOPE)
project(lwip213)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lwip-2.1.3)
set(LWIP_INCLUDE_DIRS
    "${LWIP_DIR}/src"
    "${LWIP_DIR}/src/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/contrib/ports/xilinx/include/arch"
    "${CMAKE_CURRENT_SOURCE_DIR}/contrib/ports/xilinx/include/netif"
)
include(${CMAKE_CURRENT_SOURCE_DIR}/Filelists.cmake NO_POLICY_SCOPE)
collect (PROJECT_LIB_SOURCES ${lwipnoapps_SRCS})

list(LENGTH EMACLITE_NUM_DRIVER_INSTANCES CONFIG_EMACLITE)
list(LENGTH AXIETHERNET_NUM_DRIVER_INSTANCES CONFIG_AXIETHERNET)
list(LENGTH EMACPS_NUM_DRIVER_INSTANCES CONFIG_EMACPS)
foreach(x IN LISTS TOTAL_AXIETHERNET_PROP_LIST)
    set(y ${${x}})
    list(GET y -1 connected_ip)
    list(GET y -2 axieth_rxcsum)
    list(GET y -3 axieth_txcsum)
    math(EXPR connected_ip "${connected_ip} & 0xF" OUTPUT_FORMAT HEXADECIMAL)
    if (${connected_ip} EQUAL 0x1)
       set(CONFIG_AXIFIFO 1)
    elseif (${connected_ip} EQUAL 0x2)
       set(CONFIG_AXIDMA 1)
    elseif (${connected_ip} EQUAL 0x3)
       set(CONFIG_AXIMCDMA 1)
    endif()
endforeach()
add_subdirectory(contrib/ports/xilinx/)
include(${CMAKE_CURRENT_SOURCE_DIR}/lwip213.cmake NO_POLICY_SCOPE)

list(APPEND TOTAL_MAC_INSTANCES ${EMACPS_NUM_DRIVER_INSTANCES})
list(APPEND TOTAL_MAC_INSTANCES ${AXIETHERNET_NUM_DRIVER_INSTANCES})
list(APPEND TOTAL_MAC_INSTANCES ${EMACLITE_NUM_DRIVER_INSTANCES})
SET(MAC_INSTANCES "${TOTAL_MAC_INSTANCES}" CACHE STRING "MAC Instances")
SET_PROPERTY(CACHE MAC_INSTANCES PROPERTY STRINGS "${TOTAL_MAC_INSTANCES}")
list(LENGTH TOTAL_MAC_INSTANCES _len)
if(${_len} EQUAL 0)
	message(FATAL_ERROR "This library requires an Ethernet MAC IP instance in the hardware.")
endif()

collect (PROJECT_LIB_SOURCES xtopology_g.c)

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
file(COPY ${LWIP_INCLUDE_DIRS} DESTINATION ${CMAKE_BINARY_DIR}/include)
if (${NON_YOCTO})
file(COPY_FILE ${CMAKE_BINARY_DIR}/include/lwipopts.h ${CMAKE_INCLUDE_PATH}/lwipopts.h)
file(COPY_FILE ${CMAKE_BINARY_DIR}/include/xlwipconfig.h ${CMAKE_INCLUDE_PATH}/xlwipconfig.h)
endif()
add_library(lwip213 STATIC ${_sources})
target_include_directories(lwip213 PUBLIC ${LWIP_INCLUDE_DIRS} "${LWIP_DIR}/src/include/lwip/")
set_target_properties(lwip213 PROPERTIES LINKER_LANGUAGE C)
get_headers(${_headers})
set_target_properties(lwip213 PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_PATH}/liblwip213.a;${CMAKE_INCLUDE_PATH}/lwipopts.h;${CMAKE_INCLUDE_PATH}/xlwipconfig.h;${clean_headers}")
install(TARGETS lwip213 LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
