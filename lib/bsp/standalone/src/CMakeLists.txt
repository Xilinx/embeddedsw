# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
project(xilstandalone)

set(Mem_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(common)
find_package(Mem)
list(APPEND defs "#ifndef XMEM_CONFIG_H_\n")
list(APPEND defs "#define XMEM_CONFIG_H_\n\n")

list(LENGTH MEM_DEF_NAMES len)
math(EXPR mem_len "${len} - 1")
foreach(val RANGE ${mem_len})
   list(GET MEM_DEF_NAMES ${val} MEM)
   list(GET MEM_RANGES ${val} VAL)
   list(APPEND defs "#define ${MEM} ${VAL}\n")
endforeach()
list(APPEND defs "\n#endif")
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/common/xmem_config.h ${defs})
include(${CMAKE_CURRENT_SOURCE_DIR}/xilstandalone.cmake NO_POLICY_SCOPE)
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

#Bring the headers into the project
include_directories(${CMAKE_BINARY_DIR}/include)

enable_language(C ASM)
add_subdirectory(common)
if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
add_subdirectory(microblaze)
else()
add_subdirectory(arm)
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")
file(RENAME ${CMAKE_BINARY_DIR}/include/xil_mpu_r5.h ${CMAKE_BINARY_DIR}/include/xil_mpu.h)
endif()
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr52")
file(RENAME ${CMAKE_BINARY_DIR}/include/xil_mpu_r52.h ${CMAKE_BINARY_DIR}/include/xil_mpu.h)
endif()
if (${NON_YOCTO})
file(COPY ${CMAKE_BINARY_DIR}/include/bspconfig.h DESTINATION ${CMAKE_INCLUDE_PATH}/)
endif()

#Compile it as a static library
if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
string(REPLACE "-Os -flto -ffat-lto-objects" "" CUSTOM_FLAGS ${CMAKE_C_FLAGS})
# Split the flags into a cmake list (; separated)
separate_arguments(CUSTOM_FLAGS UNIX_COMMAND ${CUSTOM_FLAGS})
# Custom command to build your one file.
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "FreeRTOS")
add_custom_command(
    OUTPUT microblaze_interrupt_handler.o
    COMMAND ${CMAKE_CXX_COMPILER}
    ARGS ${CUSTOM_FLAGS} -c ${CMAKE_CURRENT_SOURCE_DIR}/microblaze/microblaze_interrupt_handler.c
			 -I ${CMAKE_BINARY_DIR}/include
		         -o ${CMAKE_CURRENT_BINARY_DIR}/microblaze_interrupt_handler.o
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/microblaze/microblaze_interrupt_handler.c)
add_library(xilstandalone STATIC ${_sources} microblaze_interrupt_handler.o)
else()
add_library(xilstandalone STATIC ${_sources})
endif()
else()
add_library(xilstandalone STATIC ${_sources})
endif()
if (${NON_YOCTO}
    AND ("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeRTOS"))
    target_compile_definitions(xilstandalone PRIVATE FREERTOS_BSP)
endif()
set_target_properties(xilstandalone PROPERTIES LINKER_LANGUAGE C)

install(TARGETS xilstandalone LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
