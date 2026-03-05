# Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
add_definitions(-DISP_HDR_STITCH_V3_1=1 -DISP_RGBIR_V2_1=1 -DNO_TERMINAL=1 -DDUMP_IMAGE=1 -DAPU_CORE=1 -DFMC_AXI_IIC=1 -DSDT=1 -DPORTING_25=1)

include_directories(
    ${CMAKE_SOURCE_DIR}/vvbench/include
    ${CMAKE_SOURCE_DIR}/vvbench/include/submodules
)


file(READ "${CMAKE_INCLUDE_PATH}/xparameters.h" XPARAM_CONTENT)
string(REGEX MATCH "#define XPAR_VISP_SS_00_IO_MODE \"([^\"]+)\"" _match "${XPARAM_CONTENT}")
string(REGEX REPLACE "#define XPAR_VISP_SS_00_IO_MODE \"([^\"]+)\"" "\\1" IO_MODE "${_match}")
message(STATUS "Detected IO_MODE: ${IO_MODE}")
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

# Create a custom target for all generated headers
add_custom_target(generate_json_headers ALL DEPENDS ${GENERATED_JSON_HEADERS})
