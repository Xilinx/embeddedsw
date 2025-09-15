# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.16)
include(${CMAKE_SOURCE_DIR}/Empty_applicationExample.cmake)

# Include any additional CMake files here

include(${CMAKE_CURRENT_SOURCE_DIR}/UserConfig.cmake)
set(APP_NAME empty_application)
project(${APP_NAME})

find_package(common)
enable_language(C ASM CXX)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)

# Use CMAKE_LIBRARY_PATH in link_directories
link_directories(${CMAKE_LIBRARY_PATH})

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

# Add subdirectory dependency if available

collector_list (_headers PROJECT_LIB_HEADERS)
collector_list (_sources PROJECT_LIB_SOURCES)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} current_dir_sources)
list(APPEND _sources ${current_dir_sources})

foreach (source ${_sources})
    get_filename_component(ext ${source} EXT)
    list(APPEND src_ext ${ext})
endforeach()

if (NOT DEFINED PROJECT_TYPE)
find_project_type ("${src_ext}" PROJECT_TYPE)
endif()

if("${PROJECT_TYPE}" STREQUAL "c++")
collect(PROJECT_LIB_DEPS stdc++)
set(CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})
endif()
collector_list (_deps PROJECT_LIB_DEPS)
list (APPEND _deps ${USER_LINK_LIBRARIES})

string (REPLACE ";" ",-l" _deps "${_deps}")
if (CMAKE_EXPORT_COMPILE_COMMANDS AND
    (NOT ${YOCTO}))
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()
linker_gen("${CMAKE_SOURCE_DIR}/linker_files/")
string(APPEND CMAKE_C_FLAGS ${USER_COMPILE_OPTIONS})
string(APPEND CMAKE_CXX_FLAGS ${USER_COMPILE_OPTIONS})
string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINK_OPTIONS})
string(APPEND CMAKE_CXX_LINK_FLAGS ${USER_LINK_OPTIONS})
if(NOT "${_sources}" STREQUAL "")
add_dependency_on_bsp(_sources)
add_executable(${APP_NAME}.elf ${_sources})
set_target_properties(${APP_NAME}.elf PROPERTIES LINK_DEPENDS ${USER_LINKER_SCRIPT})
set(_linker_command "-Wl,-T -Wl,\"${USER_LINKER_SCRIPT}\" -L\"${CMAKE_SOURCE_DIR}/\" -L\"${CMAKE_LIBRARY_PATH}/\" -L\"${USER_LINK_DIRECTORIES}/\" -Wl,--start-group,-l${_deps} -Wl,--end-group")
target_link_libraries(${APP_NAME}.elf ${_linker_command})
target_compile_definitions(${APP_NAME}.elf PUBLIC ${USER_COMPILE_DEFINITIONS})
target_include_directories(${APP_NAME}.elf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${APP_NAME}.elf PUBLIC ${USER_INCLUDE_DIRECTORIES})
print_build_info_target(${APP_NAME}.elf "${CMAKE_C_FLAGS}" "${USER_COMPILE_DEFINITIONS}" "${CMAKE_C_LINK_FLAGS}" "${_linker_command}")
print_elf_size(CMAKE_SIZE ${APP_NAME})
endif()
