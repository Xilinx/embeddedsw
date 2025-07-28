cmake_minimum_required(VERSION 3.16)

project(metal_xlnx_extention C)

add_library(metal_xlnx_extention
    STATIC
    metal_xlnx_extention.c
)

install(TARGETS metal_xlnx_extention LIBRARY DESTINATION lib)
