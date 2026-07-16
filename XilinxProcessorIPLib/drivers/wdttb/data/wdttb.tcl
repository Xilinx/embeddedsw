###############################################################################
# Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 2.0      adk    12/10/13 Updated as per the New Tcl API's
# 4.0      sha    12/17/15 Updated the driver tcl to support parameters for
#                          legacy and window WDT.
# 4.2      ms     04/18/17 Modified tcl file to add suffix U for all macros
#                          definitions of wdttb in xparameters.h
# 4.4      sne    03/04/19 Added versal support.
# 4.5      mus    04/10/19 Added -hier option to get_cells command to support
#                          hierarchical designs. Fix for CR#1020269.
# 4.5	   sne	  09/24/19 Updated Tcl file for WWDT and AXI Timebase WDT.
# 5.3	   sd	  07/05/21 Updated tcl logic to read BASEADDRESS
#                         HIGHADDRESS parameters of IP blocks to
#                         support SSIT devices. Now get_param_value
#                         proc would be used instead of get_property
#                         proc to read those parameters.
# 5.5	   sne	 08/12/22  Updated tcl file to support C_WDT_CLK_FREQ_HZ
#			   property. Exports CLK information from xsa to
#			   wdttb_g.c file.
#
###############################################################################

#uses "xillib.tcl"

set periph_config_params_wdt	0
set periph_ninstances		0

# -----------------------------------------------------------------------------
# Main generate function - called by the tool
# -----------------------------------------------------------------------------
proc generate {drv_handle} {

	xdefine_wdttb_include_file $drv_handle "xparameters.h" "XWdtTb"
	xdefine_wdttb_config_file "xwdttb_g.c" "XWdtTb"
}
proc init_periph_config_struct { deviceid } {
	global periph_config_params_wdt
	set periph_config_params_wdt($deviceid) [list]
}

proc get_periph_config_struct_fields { deviceid } {
	global periph_config_params_wdt
	return $periph_config_params_wdt($deviceid)
}
proc add_field_to_periph_config_struct { deviceid fieldval } {
	global periph_config_params_wdt
	lappend periph_config_params_wdt($deviceid) $fieldval
}

# -----------------------------------------------------------------------------
# Given AXI Timebase WDT peripheral, generate all the parameters required in
# the system include file.
#
# Given AXI Timebase WDT peripheral. Figure out whether legacy or Window WDT,
# populate parameters accordingly.
#
# -----------------------------------------------------------------------------
proc xdefine_wdttb_include_file {drv_handle file_name drv_string} {
	global periph_ninstances

	# Open include file
	set file_handle [::hsi::utils::open_include_file $file_name]

	# Get all peripherals connected to this driver
	set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

	set uSuffix "U"
	# Handle NUM_INSTANCES
	set periph_ninstances 0
	puts $file_handle "/* Definitions for driver [string toupper [get_property NAME $drv_handle]] */"
	foreach periph $periphs {
		init_periph_config_struct $periph_ninstances
		incr periph_ninstances 1
	}
	puts $file_handle "\#define [::hsi::utils::get_driver_param_name $drv_string NUM_INSTANCES] $periph_ninstances$uSuffix"

	# Close include file
	close $file_handle

	# Now print all useful parameters for all peripherals
	set device_id 0
	foreach periph $periphs {
		set file_handle [::hsi::utils::open_include_file $file_name]

		xdefine_params_include_file $file_handle $periph $device_id

		# Create canonical definitions
		xdefine_params_canonical $file_handle $periph $device_id

		incr device_id
		puts $file_handle "\n"
		close $file_handle
	}

	# Open include file
	set file_handle [::hsi::utils::open_include_file $file_name]
	puts $file_handle "/******************************************************************/"
	close $file_handle
}

# -----------------------------------------------------------------------------
# Create configuration C file as required by Xilinx drivers
# Use the config field list technique.
# -----------------------------------------------------------------------------
proc xdefine_wdttb_config_file {file_name drv_string} {
	global periph_ninstances

	set filename [file join "src" $file_name]
	set config_file [open $filename w]
	::hsi::utils::write_c_header $config_file "Driver configuration"
	puts $config_file "\#include \"xparameters.h\""
	puts $config_file "\#include \"[string tolower $drv_string].h\""
	puts $config_file "\n/*"
	puts $config_file "* The configuration table for devices"
	puts $config_file "*/\n"
	puts $config_file [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
	puts $config_file "\{"

	set start_comma ""
	for {set i 0} {$i < $periph_ninstances} {incr i} {

		puts $config_file [format "%s\t\{" $start_comma]
		set comma ""
		foreach field [get_periph_config_struct_fields $i] {
			puts -nonewline $config_file [format "%s\t\t%s" $comma $field]
			set comma ",\n"
		}

		puts -nonewline $config_file "\n\t\}"
		set start_comma ",\n"
	}
	puts $config_file "\n\};\n"
	close $config_file
}

proc xdefine_params_include_file {file_handle periph device_id} {
	set uSuffix "U"
	puts $file_handle "\n/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
	set is_pl [common::get_property IS_PL $periph]
	if {$is_pl == 1} {
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DEVICE_ID"] $device_id$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] [common::get_property CONFIG.C_BASEADDR $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "HIGHADDR"] [common::get_property CONFIG.C_HIGHADDR $periph]$uSuffix"
		set enable_wdt [common::get_property CONFIG.C_ENABLE_WINDOW_WDT $periph]
                if {$enable_wdt == 1 } {
                        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "ENABLE_WINDOW_WDT"] $enable_wdt$uSuffix"
                        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "MAX_COUNT_WIDTH"] [common::get_property CONFIG.C_MAX_COUNT_WIDTH $periph]$uSuffix"
                        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "SST_COUNT_WIDTH"] [common::get_property CONFIG.C_SST_COUNT_WIDTH $periph]$uSuffix"
                } else {
                        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "ENABLE_WINDOW_WDT"] 0$uSuffix"
                        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "MAX_COUNT_WIDTH"] 0$uSuffix"
                        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "SST_COUNT_WIDTH"] 0$uSuffix"
                }
                        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "IS_PL"] 1$uSuffix"
	} else {
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DEVICE_ID"] $device_id$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] [::hsi::utils::get_param_value $periph C_S_AXI_BASEADDR]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "HIGHADDR"] [::hsi::utils::get_param_value $periph C_S_AXI_HIGHADDR]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "ENABLE_WINDOW_WDT"] 0$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "MAX_COUNT_WIDTH"] 0$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "SST_COUNT_WIDTH"] 0$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "IS_PL"] 0$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "WDT_CLK_FREQ_HZ"] [::hsi::utils::get_param_value $periph C_WDT_CLK_FREQ_HZ]"
	}
}

# -----------------------------------------------------------------------------
# This procedure creates XPARs that are canonical/normalized for the hardware
# design parameters.
# -----------------------------------------------------------------------------
proc xdefine_params_canonical {file_handle periph device_id} {
	set uSuffix "U"
	puts $file_handle "\n/* Canonical definitions for peripheral [string toupper [get_property NAME $periph]] */"
	set is_pl [common::get_property IS_PL $periph]

	set canonical_tag [string toupper [format "XPAR_WDTTB_%d" $device_id]]

	# Handle device ID argument
	set canonical_name [format "%s_DEVICE_ID" $canonical_tag]
	puts $file_handle "\#define $canonical_name $device_id$uSuffix"
	add_field_to_periph_config_struct $device_id $canonical_name

	if {$is_pl == 1} {
	# Handle BASEADDR argument
	set canonical_name [format "%s_BASEADDR" $canonical_tag]
	puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_BASEADDR]$uSuffix"
	add_field_to_periph_config_struct $device_id $canonical_name

	# Handle HIGHADDR argument
	set canonical_name [format "%s_HIGHADDR" $canonical_tag]
	puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_HIGHADDR]$uSuffix"

	set enable_wdt [::hsi::utils::get_param_value $periph C_ENABLE_WINDOW_WDT]
	if {$enable_wdt == 1 } {
		# Handle ENABLE_WINDOW_WDT argument
		set canonical_name [format "%s_ENABLE_WINDOW_WDT" $canonical_tag]
		puts $file_handle "\#define $canonical_name $enable_wdt$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		# Handle MAX_COUNT_WIDTH argument
		set canonical_name [format "%s_MAX_COUNT_WIDTH" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_MAX_COUNT_WIDTH]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		# Handle SST_COUNT_WIDTH argument
		set canonical_name [format "%s_SST_COUNT_WIDTH" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_SST_COUNT_WIDTH]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name
	} else {
		# Handle ENABLE_WINDOW_WDT argument
		set canonical_name [format "%s_ENABLE_WINDOW_WDT" $canonical_tag]
		puts $file_handle "\#define $canonical_name 0$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		# Handle MAX_COUNT_WIDTH argument
		set canonical_name [format "%s_MAX_COUNT_WIDTH" $canonical_tag]
		puts $file_handle "\#define $canonical_name 0$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		# Handle SST_COUNT_WIDTH argument
		set canonical_name [format "%s_SST_COUNT_WIDTH" $canonical_tag]
		puts $file_handle "\#define $canonical_name 0$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name
	}

	# Handle IS_PL argument
	set canonical_name [format "%s_IS_PL" $canonical_tag]
	puts $file_handle "\#define $canonical_name 1$uSuffix"
	add_field_to_periph_config_struct $device_id $canonical_name
	# Handle CLK argument
	set canonical_name [format "%s_WDT_CLK_FREQ_HZ" $canonical_tag]
	puts $file_handle "\#define $canonical_name 0$uSuffix"
	add_field_to_periph_config_struct $device_id $canonical_name
	} else {
		set canonical_name [format "%s_BASEADDR" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_S_AXI_BASEADDR]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_HIGHADDR" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_S_AXI_HIGHADDR]$uSuffix"

		set canonical_name [format "%s_ENABLE_WINDOW_WDT" $canonical_tag]
		puts $file_handle "\#define $canonical_name 0$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_MAX_COUNT_WIDTH" $canonical_tag]
		puts $file_handle "\#define $canonical_name 0$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_SST_COUNT_WIDTH" $canonical_tag]
		puts $file_handle "\#define $canonical_name 0$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_IS_PL" $canonical_tag]
		puts $file_handle "\#define $canonical_name 0$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_WDT_CLK_FREQ_HZ" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_WDT_CLK_FREQ_HZ]"
		add_field_to_periph_config_struct $device_id $canonical_name
	}
}
