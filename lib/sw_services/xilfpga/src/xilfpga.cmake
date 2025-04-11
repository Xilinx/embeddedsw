# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.3)

option(XILFPGA_secure_mode "Enable secure Bitstream loading support" ON)
option(XILFPGA_secure_environment "Which is used to Enable the secure PL configuration" OFF)
option(XILFPGA_secure_readback "Which is used to Enable the secure PL configuration Read-back support" OFF)
option(XILFPGA_debug_mode "Which is used to Enable the Debug messages in the library" OFF)
option(XILFPGA_reg_readback_en "Which is used to Enable the FPGA configuration Register Read-back support" ON)
option(XILFPGA_data_readback_en "Which is used to Enable the FPGA configuration Data Read-back support" ON)
option(XILFPGA_get_version_info_en "Which is used to Get the Xilfpga library version info" OFF)
option(XILFPGA_get_feature_list_en "Which is used to Get the Xilfpga library supported feature list info" OFF)
option(XILPFGA_skip_efuse_check_en "Which is used to skip the eFUSE checks for PL configuration" OFF)

SET(XILFPGA_ocm_address 0xfffc0000 CACHE STRING "OCM Address which is used for Bitstream Authentication")
SET(XILFPGA_base_address 0x80000 CACHE STRING "Bitstream Image Base Address")

set(zynqmp_secure_env 0)
if (("${CMAKE_MACHINE}" STREQUAL "Versal") OR ("${CMAKE_MACHINE}" STREQUAL "VersalNet"))
    set(VERSAL_INCLUDE "#include <xilfpga_versal.h>")
elseif("${CMAKE_MACHINE}" STREQUAL "ZynqMP")
    set(PLATFORM_ZYNQMP " ")
    if((NOT("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze")) AND
       ${XILFPGA_secure_environment})
        set(XFPGA_SECURE_IPI_MODE_EN " ")
        set(zynqmp_secure_env 1)
    else()
        set(PCAP_INCLUDE "#include <xilfpga_pcap.h>")
    endif()
    set(PCAP_COMMON_INCLUDE "#include <xilfpga_pcap_common.h>")
endif()

set(XFPGA_OCM_ADDRESS ${XILFPGA_ocm_address})
set(XFPGA_BASE_ADDRESS ${XILFPGA_base_address})
if (${XILFPGA_secure_mode})
	set(XFPGA_SECURE_MODE " ")
endif()
if (${XILFPGA_debug_mode})
	set(XFPGA_DEBUG " ")
endif()
if (${XILFPGA_secure_readback})
    set(XFPGA_SECURE_READBACK_MODE " ")
endif()

if (${XILFPGA_reg_readback_en})
    set(XFPGA_READ_CONFIG_REG " ")
endif()

if (${XILFPGA_data_readback_en})
    set(XFPGA_READ_CONFIG_DATA " ")
endif()

if (${XILFPGA_get_version_info_en})
    set(XFPGA_GET_VERSION_INFO " ")
endif()

if (${XILFPGA_get_feature_list_en})
    set(XFPGA_GET_FEATURE_LIST " ")
endif()

if (${XILPFGA_skip_efuse_check_en})
    set(XFPGA_SKIP_EFUSE_CHECK " ")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xfpga_config.h.in ${CMAKE_BINARY_DIR}/include/xfpga_config.h)
