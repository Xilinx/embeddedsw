###############################################################################
#
# Copyright (C) 2011 - 2019 Xilinx, Inc. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
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
###############################################################################

#uses "xillib.tcl"

set periph_config_params_wdt	0
set periph_ninstances		0

# -----------------------------------------------------------------------------
# Main generate function - called by the tool
# -----------------------------------------------------------------------------
proc generate {drv_handle} {
set ip_name [get_property IP_NAME [get_cells $drv_handle]]
if {$ip_name == "psu_wwdt" || $ip_name == "psv_wwdt"} {
     xdefine_zynq_include_file $drv_handle "xparameters.h" "XWdtTb" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_WDT_CLK_FREQ_HZ"

    xdefine_zynq_config_file $drv_handle "xwdttb_g.c" "XWdtTb" "DEVICE_ID" "C_S_AXI_BASEADDR"

    xdefine_zynq_canonical_xpars $drv_handle "xparameters.h" "WdtTb" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_WDT_CLK_FREQ_HZ"
} else {

	xdefine_wdttb_include_file $drv_handle "xparameters.h" "XWdtTb"
	xdefine_wdttb_config_file "xwdttb_g.c" "XWdtTb"
}
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

	puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DEVICE_ID"] $device_id$uSuffix"
	puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] [common::get_property CONFIG.C_BASEADDR $periph]$uSuffix"
	puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "HIGHADDR"] [common::get_property CONFIG.C_HIGHADDR $periph]$uSuffix"

	set enable_wdt [common::get_property CONFIG.C_ENABLE_WINDOW_WDT $periph]
	if {[llength $enable_wdt] == 1 } {
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "ENABLE_WINDOW_WDT"] $enable_wdt$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "MAX_COUNT_WIDTH"] [common::get_property CONFIG.C_MAX_COUNT_WIDTH $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "SST_COUNT_WIDTH"] [common::get_property CONFIG.C_SST_COUNT_WIDTH $periph]$uSuffix"
	} else {
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "ENABLE_WINDOW_WDT"] 0$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "MAX_COUNT_WIDTH"] 0$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "SST_COUNT_WIDTH"] 0$uSuffix"
	}
}

# -----------------------------------------------------------------------------
# This procedure creates XPARs that are canonical/normalized for the hardware
# design parameters.
# -----------------------------------------------------------------------------
proc xdefine_params_canonical {file_handle periph device_id} {
	set uSuffix "U"
	puts $file_handle "\n/* Canonical definitions for peripheral [string toupper [get_property NAME $periph]] */"

	set canonical_tag [string toupper [format "XPAR_WDTTB_%d" $device_id]]

	# Handle device ID argument
	set canonical_name [format "%s_DEVICE_ID" $canonical_tag]
	puts $file_handle "\#define $canonical_name $device_id$uSuffix"
	add_field_to_periph_config_struct $device_id $canonical_name

	# Handle BASEADDR argument
	set canonical_name [format "%s_BASEADDR" $canonical_tag]
	puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_BASEADDR]$uSuffix"
	add_field_to_periph_config_struct $device_id $canonical_name

	# Handle HIGHADDR argument
	set canonical_name [format "%s_HIGHADDR" $canonical_tag]
	puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_HIGHADDR]$uSuffix"

	set enable_wdt [::hsi::utils::get_param_value $periph C_ENABLE_WINDOW_WDT]
	if {[llength $enable_wdt] == 1 } {
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
}
