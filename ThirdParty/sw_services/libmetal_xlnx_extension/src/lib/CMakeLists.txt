cmake_minimum_required(VERSION 3.16)

project(metal_xlnx_extension C)

set (SRC metal_xlnx_extension.c)

if ("${PROJECT_MACHINE}" STREQUAL "microblaze_generic" OR
    "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze")
    set (SRC metal_xlnx_extension_mb.c)
endif()

add_library(metal_xlnx_extension
    STATIC
    ${SRC}
)

install(TARGETS metal_xlnx_extension LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
get_filename_component(INSTALL_INCLUDE_PARENT "${CMAKE_INCLUDE_PATH}" DIRECTORY)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${INSTALL_INCLUDE_PARENT})
