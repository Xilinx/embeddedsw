# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.3)

set(XPAR_XILPM_ENABLED " ")

string(COMPARE EQUAL "${DEVICE_ID}" "xcvp1902" IS_DEVICE_ID_PRESENT)
if (IS_DEVICE_ID_PRESENT)
  set(XCVP1902 " ")
endif()
string(COMPARE EQUAL "${DEVICE_ID}" "xcvr1602" IS_DEVICE_ID_PRESENT)
if (IS_DEVICE_ID_PRESENT)
  set(XCVR1602 " ")
endif()
string(COMPARE EQUAL "${DEVICE_ID}" "xcvr1652" IS_DEVICE_ID_PRESENT)
if (IS_DEVICE_ID_PRESENT)
  set(XCVR1652 " ")
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

option(XILPM_Rail_Control "true: Include power rail control feature support, false: Exclude power rail control feature support" OFF)
if (XILPM_Rail_Control)
  set(RAIL_CONTROL " ")
endif()

if("${CMAKE_MACHINE}" STREQUAL "VersalNet")
	# For Versal Net, enable by default
	option(XILPM_enable_bulk_dev_release "true: Include bulk device release feature support, false: Exclude bulk device release feature support" ON)
	option(XILPM_enable_unregister_all_notifier "true: Include unregister all notifier feature support, false: Exclude unregister all notifier feature support" ON)
elseif("${CMAKE_MACHINE}" STREQUAL "Versal")
	# For Versal, keep it disabled
	option(XILPM_enable_bulk_dev_release "true: Include bulk device release feature support, false: Exclude bulk device release feature support" OFF)
	option(XILPM_enable_unregister_all_notifier "true: Include unregister all notifier feature support, false: Exclude unregister all notifier feature support" OFF)
endif()
if (XILPM_enable_bulk_dev_release)
	set(ENABLE_BULK_DEV_RELEASE_SUPPORT " ")
endif()
if (XILPM_enable_unregister_all_notifier)
	set(ENABLE_UNREGISTER_ALL_NOTIFIER_SUPPORT " ")
endif()

string(TOUPPER "${SPEED_GRADE}" SPEED_GRADE_U)
string(COMPARE EQUAL "${SPEED_GRADE_U}" "2LLI" IS_SPEED_GRADE_2LLI)
string(COMPARE EQUAL "${SPEED_GRADE_U}" "2LI" IS_SPEED_GRADE_2LI)
option(XILPM_Versal_DVS "true: Include Versal DVS feature support, false: Exclude Versal DVS feature support" OFF)
if (XILPM_Versal_DVS OR IS_SPEED_GRADE_2LLI OR IS_SPEED_GRADE_2LI)
  set(RAIL_CONTROL " ")
  set(VERSAL_DVS " ")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xpm_config.h.in ${CMAKE_BINARY_DIR}/include/xpm_config.h)
