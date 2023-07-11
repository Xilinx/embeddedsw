# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
include(${CMAKE_CURRENT_SOURCE_DIR}/UsbpsExample.cmake)
project(usbps)

find_package(common)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

SET(DRIVER_INSTANCES "${NUM_DRIVER_INSTANCES}" CACHE STRING "Driver Instances")
SET_PROPERTY(CACHE DRIVER_INSTANCES PROPERTY STRINGS "${NUM_DRIVER_INSTANCES}")
set(index 0)
LIST_INDEX(${index} ${DRIVER_INSTANCES} "${NUM_DRIVER_INSTANCES}")
list(GET TOTAL_EXAMPLE_LIST ${index} ex_list)
list(GET REG_LIST ${index} reg)
SET(XUSBPS_BASEADDRESS "${reg}")
SET(xusbps_intr_example  xusbps_intr_example.c;xusbps_class_storage.c;xusbps_class_storage.h;xusbps_ch9_storage.c;xusbps_ch9_storage.h;xusbps_ch9.c;xusbps_ch9.h)
list(APPEND EXAMPLE_LIST xusbps_intr_example)
SET(xusbps_audio_example.c xusbps_audio_example.c;xusbps_ch9_audio.c;xusbps_ch9_audio.h;xusbps_ch9.c;xusbps_ch9.h;xusbps_class_audio.c;xusbps_class_audio.h;xusbps_audiodata.h)
list(APPEND EXAMPLE_LIST xusbps_audio_example)

set(CMAKE_INFILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../scripts/linker_files/")
linker_gen(${CMAKE_INFILE_PATH})
gen_exheader(${CMAKE_INFILE_PATH} ${CMAKE_PROJECT_NAME} ${XUSBPS_BASEADDRESS} x)

foreach(EXAMPLE ${EXAMPLE_LIST})
    message("example ")
    message("${${EXAMPLE}}")
    add_executable(${EXAMPLE}.elf ${${EXAMPLE}})
    target_link_libraries(${EXAMPLE}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
endforeach()
