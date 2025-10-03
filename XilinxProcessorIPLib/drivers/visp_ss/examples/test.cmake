# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
add_definitions(-DISP_HDR_STITCH_V3_1=1 -DISP_RGBIR_V2_1=1 -DNO_TERMINAL=1 -DDUMP_IMAGE=1 -DAPU_CORE=1 -DFMC_AXI_IIC=1 -DSDT=1 -DPORTING_25=1)

file(READ "${CMAKE_INCLUDE_PATH}/xparameters.h" XPARAM_CONTENT)
string(REGEX MATCH "#define XPAR_VISP_SS_00_IO_MODE \"([^\"]+)\"" _match "${XPARAM_CONTENT}")
string(REGEX REPLACE "#define XPAR_VISP_SS_00_IO_MODE \"([^\"]+)\"" "\\1" IO_MODE "${_match}")

message(STATUS "Detected IO_MODE: ${IO_MODE}")

if(IO_MODE STREQUAL "lilo")
	link_libraries("visp_lilo")
else()
	link_libraries("visp")
endif()

add_compile_options(-w)
