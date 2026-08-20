# Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
add_definitions(-DISP_HDR_STITCH_V3_1=1 -DISP_RGBIR_V2_1=1 -DNO_TERMINAL=1 -DDUMP_IMAGE=1 -DAPU_CORE=1 -DFMC_AXI_IIC=1 -DSDT=1 -DPORTING_25=1 -DWDT_NOTIFIER)
# Enable -DWDT_SEND_TEST when you need to run the WDT self test.

include_directories(
    ${CMAKE_SOURCE_DIR}/vvbench/include
    ${CMAKE_SOURCE_DIR}/vvbench/include/submodules
    ${CMAKE_SOURCE_DIR}/hdmi_display
)


file(READ "${CMAKE_INCLUDE_PATH}/xparameters.h" XPARAM_CONTENT)
string(REGEX MATCH "#define XPAR_VISP_SS_00_IO_MODE \"([^\"]+)\"" _match "${XPARAM_CONTENT}")
string(REGEX REPLACE "#define XPAR_VISP_SS_00_IO_MODE \"([^\"]+)\"" "\\1" IO_MODE "${_match}")
message(STATUS "Detected IO_MODE: ${IO_MODE}")

# DisplayPort live-output support needs both the frame-buffer reader (the live
# source) and the DP/DC/Clock-Wizard IPs. Detect the frmbuf_rd and clk_wiz
# driver instances so the same source tree also builds on ISP-only / HDMI
# designs that lack them. When either is absent, the dp_display module is
# neither included nor compiled and DP_DISPLAY_AVAILABLE is left undefined.
string(FIND "${XPARAM_CONTENT}" "XPAR_XV_FRMBUF_RD_NUM_INSTANCES" _dp_fbrd_pos)
string(FIND "${XPARAM_CONTENT}" "XPAR_XCLK_WIZ_NUM_INSTANCES" _dp_clkwiz_pos)
if(NOT _dp_fbrd_pos EQUAL -1 AND NOT _dp_clkwiz_pos EQUAL -1)
    set(DP_DISPLAY_AVAILABLE TRUE)
    message(STATUS "DP display: frmbuf_rd + DP/DC/clk_wiz present -> enabling dp_display")
    include_directories(${CMAKE_SOURCE_DIR}/dp_display)
    add_definitions(-DDP_DISPLAY_AVAILABLE=1)
else()
    set(DP_DISPLAY_AVAILABLE FALSE)
    message(STATUS "DP display: frmbuf_rd or clk_wiz/DP IPs missing -> skipping dp_display")
endif()

link_libraries("visp")
add_compile_options(-w)

# Convert JSON files to header files using xxd
file(GLOB JSON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/reference_jsons/*.json")

foreach(JSON_FILE ${JSON_FILES})
    # Get the base name without extension
    get_filename_component(JSON_NAME ${JSON_FILE} NAME_WE)
    get_filename_component(JSON_FILENAME ${JSON_FILE} NAME)

    # Define output header file path in src folder
    set(HEADER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${JSON_NAME}.h")

    # Custom command to generate .h from .json and copy to src
    add_custom_command(
        OUTPUT ${HEADER_FILE}
        COMMAND xxd -i ${JSON_FILENAME} > ${HEADER_FILE}
        DEPENDS ${JSON_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/reference_jsons
        COMMENT "Converting ${JSON_NAME}.json to ${JSON_NAME}.h"
        VERBATIM
    )

    # Add to list of generated headers
    list(APPEND GENERATED_JSON_HEADERS ${HEADER_FILE})
endforeach()

# Convert ext_sensor.xml to header file if it exists
set(EXT_SENSOR_XML_FILE "${CMAKE_CURRENT_SOURCE_DIR}/reference_jsons/ext_sensor.xml")
if(EXISTS ${EXT_SENSOR_XML_FILE})
    set(EXT_SENSOR_HEADER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ext_sensor.h")

    add_custom_command(
        OUTPUT ${EXT_SENSOR_HEADER_FILE}
        COMMAND xxd -i ext_sensor.xml > ${EXT_SENSOR_HEADER_FILE}
        DEPENDS ${EXT_SENSOR_XML_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/reference_jsons
        COMMENT "Converting ext_sensor.xml to ext_sensor.h"
        VERBATIM
    )

    list(APPEND GENERATED_JSON_HEADERS ${EXT_SENSOR_HEADER_FILE})
endif()

# Convert image.raw to header file if it exists
set(IMAGE_RAW_FILE "${CMAKE_CURRENT_SOURCE_DIR}/reference_jsons/image.raw")
if(EXISTS ${IMAGE_RAW_FILE})
    set(IMAGE_HEADER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/image.h")

    add_custom_command(
        OUTPUT ${IMAGE_HEADER_FILE}
        COMMAND xxd -i image.raw > ${IMAGE_HEADER_FILE}
        COMMAND sed -i -e "s/unsigned char/const unsigned char/" -e "s/unsigned int/const unsigned int/" ${IMAGE_HEADER_FILE}
        DEPENDS ${IMAGE_RAW_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/reference_jsons
        COMMENT "Converting image.raw to image.h with const qualifiers"
        VERBATIM
    )

    list(APPEND GENERATED_JSON_HEADERS ${IMAGE_HEADER_FILE})
endif()


message(STATUS "Processor: ${CMAKE_SYSTEM_PROCESSOR}")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "cortexa78")
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libvisp_a78.a")
		file(RENAME
			${CMAKE_CURRENT_SOURCE_DIR}/libvisp_a78.a
			${CMAKE_CURRENT_SOURCE_DIR}/libvisp.a)
	endif()
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lscript_a78.ld")
		file(RENAME
			${CMAKE_CURRENT_SOURCE_DIR}/lscript_a78.ld
			${CMAKE_CURRENT_SOURCE_DIR}/lscript.ld)
	endif()
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lscript_r52.ld")
		file(REMOVE
		    ${CMAKE_CURRENT_SOURCE_DIR}/lscript_r52.ld)
	endif()
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libvisp_r52.a")
		file(REMOVE
		    ${CMAKE_CURRENT_SOURCE_DIR}/libvisp_r52.a)
	endif()
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "cortexr52")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-short-enums")
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libvisp_r52.a")
                file(RENAME
                        ${CMAKE_CURRENT_SOURCE_DIR}/libvisp_r52.a
                        ${CMAKE_CURRENT_SOURCE_DIR}/libvisp.a)
        endif()
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lscript_r52.ld")
                file(RENAME
                        ${CMAKE_CURRENT_SOURCE_DIR}/lscript_r52.ld
                        ${CMAKE_CURRENT_SOURCE_DIR}/lscript.ld)
        endif()
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lscript_a78.ld")
                file(REMOVE
                    ${CMAKE_CURRENT_SOURCE_DIR}/lscript_a78.ld)
        endif()
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libvisp_a78.a")
                file(REMOVE
                    ${CMAKE_CURRENT_SOURCE_DIR}/libvisp_a78.a)
        endif()

endif()


# Create a custom target for all generated headers
add_custom_target(generate_json_headers ALL DEPENDS ${GENERATED_JSON_HEADERS})
