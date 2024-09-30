# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(libxil)

if (YOCTO)
include_directories(${CMAKE_BINARY_DIR}/include)
add_library(libxil INTERFACE)
else()
find_package(common)
include_directories(${CMAKE_BINARY_DIR}/include)
set(DRVLIST_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(DRVLIST)
list(APPEND xil_mri "create libxil.a\n")
FOREACH(DRIVER ${DRIVER_LIST})
    STRING(REGEX REPLACE "-" "_" DRIVER ${DRIVER})
    add_subdirectory(${DRIVER}/src)
    get_property(src_files TARGET ${DRIVER} PROPERTY SOURCES)
    foreach(src ${src_files})
	set(drv_archive ${CMAKE_BINARY_DIR}/libsrc/${DRIVER}/src/lib${DRIVER}.a)
	if (NOT (${drv_archive} IN_LIST OBJS))
	    list(APPEND OBJS ${drv_archive})
	endif()
    endforeach()
ENDFOREACH()

# Reverse the OBJS list to ensure correct order
list(REVERSE OBJS)

# Append the reversed OBJS to xil_mri
foreach(drv_archive ${OBJS})
    list(APPEND xil_mri "addlib ${drv_archive}\n")
endforeach()

list(APPEND xil_mri "save\n")
list(APPEND xil_mri "end\n")
file(WRITE ${CMAKE_BINARY_DIR}/libxil.mri ${xil_mri})

set(XIL_LIB ${CMAKE_BINARY_DIR}/libxil.a)
add_custom_target(combined ALL
	COMMAND ${CMAKE_AR} -M<${CMAKE_BINARY_DIR}/libxil.mri
	COMMAND ${CMAKE_COMMAND} -E copy ${XIL_LIB} ${CMAKE_LIBRARY_PATH}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	DEPENDS ${DRIVER_LIST})
add_library(xil STATIC IMPORTED GLOBAL)
add_dependencies(xil combined)
set_target_properties(xil PROPERTIES IMPORTED_LOCATION ${XIL_LIB} LINKER_LANGUAGE C)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
endif()
