# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.3)

set(XPAR_XILPM_ENABLED " ")

string(COMPARE EQUAL "${DEVICE_ID}" "xcvp1902" IS_DEVICE_ID_PRESENT)
if (IS_DEVICE_ID_PRESENT)
  set(XCVP1902 " ")
endif()


option(XILPM_rpu0_as_power_management_master "true: RPU0 as power management master, false: Disable RPU0 as power management master." ON)
option(XILPM_rpu1_as_power_management_master "true: RPU1 as power management master, false: Disable RPU1 as power management master." ON)
option(XILPM_apu_as_power_management_master "true: APU as power management master, false: Disable APU as power management master." ON)

option(XILPM_rpu0_as_reset_management_master "true: RPU0 as reset management master, false: Disable RPU0 as reset management master" ON)
option(XILPM_rpu1_as_reset_management_master "true: RPU1 as reset management master, false: Disable RPU1 as reset management master" ON)
option(XILPM_apu_as_reset_management_master "true: APU as reset management master, false: Disable APU as reset management master" ON)

option(XILPM_rpu0_as_overlay_config_master "true: RPU0 has permission to load overlay config objects, false: RPU0 doesn't have permission to load overlay config objects" OFF)
option(XILPM_rpu1_as_overlay_config_master "true: RPU1 has permission to load overlay config objects, false: RPU1 doesn't have permission to load overlay config objects" OFF)
option(XILPM_apu_as_overlay_config_master "true: APU has permission to load overlay config objects, false: APU doesn't have permission to load overlay config objects" OFF)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xpm_config.h.in ${CMAKE_BINARY_DIR}/include/xpm_config.h)
