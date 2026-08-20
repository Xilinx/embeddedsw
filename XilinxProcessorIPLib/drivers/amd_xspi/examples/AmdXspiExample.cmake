# Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT

set(AMD_XSPI_NUM_DRIVER_INSTANCES "${NUM_DRIVER_INSTANCES}" CACHE STRING "Number of driver instances")
set_property(CACHE AMD_XSPI_NUM_DRIVER_INSTANCES PROPERTY STRINGS "1" "2" "3" "4")

set(CMAKE_STATIC_LIBRARY_PREFIX "")

# Examples are named amd_xspi_<interface>_<device>_<transfer>_example, because the
# same device family can be driven over either interface: HyperRAM and
# HyperFlash are both reachable over HyperBus (Profile 2.0) and Octal xSPI
# (Profile 1.0). This release supports HyperRAM over HyperBus only.
#
# TODO: C_SPI_MODE is fixed when the IP is built, so a given design supports
# only one interface. Once further interfaces are supported, select this list
# from the configured mode instead of building all of them.
set(AMD_XSPI_EXAMPLES amd_xspi_hyperbus_hyperram_polled_example;amd_xspi_hyperbus_hyperram_intr_example)

foreach(example ${AMD_XSPI_EXAMPLES})
    set(examples ${example})
    add_library(${example} STATIC ${example}.c)
    target_link_libraries(${example} PUBLIC amd_xspi)
endforeach()
