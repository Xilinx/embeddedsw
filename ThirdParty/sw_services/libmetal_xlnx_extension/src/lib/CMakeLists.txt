cmake_minimum_required(VERSION 3.16)

project(metal_xlnx_extension C)

add_library(metal_xlnx_extension
    STATIC
    metal_xlnx_extension.c
)

install(TARGETS metal_xlnx_extension LIBRARY DESTINATION lib)
