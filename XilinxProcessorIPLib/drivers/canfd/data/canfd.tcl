###############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
#
# Ver	Who	Date	 Changes
# ----	-----	-------	 -----------------------------
# 1.0	  nsk	15/05/15 Updated as per RTL. TxBuffer
#			  Can be configurable(8,16,32).
# 2.1     ask   07/03/18 Added a macro to distinguish between CANFD_v1_0 and
#			 CANFD_v2_0.
# 2.1	nsk	07/11/18 Added CANFD Frequency macro to xparameters.h
# 2.1	nsk	09/03/18 Added support for all CANFD versions
#			 i.e. PS CANFD, PL CANFD 1.0 and PL CANFD 2.0
# 2.3	sne	11/28/19 Updated tcl file to support CANFD PS and PL on versal.
#
###############################################################################

#uses "xillib.tcl"


set periph_config_params_canfd	0
set periph_ninstances		0

# -----------------------------------------------------------------------------
# Main generate function - called by the tool
# -----------------------------------------------------------------------------
proc generate {drv_handle} {

         set periph [get_cells -hier $drv_handle]
          set version [string tolower [common::get_property VLNV $periph]]
          if {[string compare -nocase "xilinx.com:ip:canfd:1.0" $version] == 0} {
              set file_handle [::hsi::utils::open_include_file "xparameters.h"]
              puts $file_handle "#define CANFD_v1_0"
              close $file_handle
          }
	set is_pl [common::get_property IS_PL $periph]
          set file_handle [::hsi::utils::open_include_file "xparameters.h"]
          if {$is_pl == "0"} {
              puts $file_handle "#define XPAR_CANFD_ISPS"
          } else {
              puts $file_handle "#define XPAR_CANFD_ISPL"
          }
          close $file_handle

	xdefine_canfd_include_file $drv_handle "xparameters.h" "XCanFd"
	xdefine_canfd_config_file "xcanfd_g.c" "XCanFd"
}
proc init_periph_config_struct { deviceid } {
	global periph_config_params_canfd
	set periph_config_params_canfd($deviceid) [list]
}

proc get_periph_config_struct_fields { deviceid } {
	global periph_config_params_canfd
	return $periph_config_params_canfd($deviceid)
}
proc add_field_to_periph_config_struct { deviceid fieldval } {
	global periph_config_params_canfd
	lappend periph_config_params_canfd($deviceid) $fieldval
}

# -----------------------------------------------------------------------------
# Given CANFD peripheral, generate all the parameters required in
# the system include file.
# -----------------------------------------------------------------------------
proc xdefine_canfd_include_file {drv_handle file_name drv_string} {
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
		close $file_handle
	}

	# Open include file
	set file_handle [::hsi::utils::open_include_file $file_name]
	puts $file_handle "\n/******************************************************************/"
	close $file_handle
}

# -----------------------------------------------------------------------------
# Create configuration C file as required by Xilinx drivers
# Use the config field list technique.
# -----------------------------------------------------------------------------
proc xdefine_canfd_config_file {file_name drv_string} {
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
		set avail_param [list_property [get_cells -hier $periph]]
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DEVICE_ID"] $device_id$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] [common::get_property CONFIG.C_BASEADDR $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "HIGHADDR"] [common::get_property CONFIG.C_HIGHADDR $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "RX_MODE"] [common::get_property CONFIG.RX_MODE $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "NUM_OF_RX_MB_BUF"] [common::get_property CONFIG.NUM_OF_RX_MB_BUF $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "NUM_OF_TX_BUF"] [common::get_property CONFIG.NUM_OF_TX_BUF $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "IS_PL"] [common::get_property IS_PL $periph]$uSuffix"
		if {[lsearch -nocase $avail_param "CONFIG.C_EN_APB"] >= 0} {

		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "RX_FIFO_0_DEPTH"] [common::get_property CONFIG.C_RX_FIFO_0_DEPTH $periph]$uSuffix"
		}
	} else {
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DEVICE_ID"] $device_id$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] [common::get_property CONFIG.C_S_AXI_BASEADDR $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "HIGHADDR"] [common::get_property CONFIG.C_S_AXI_HIGHADDR $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "RX_MODE"] [common::get_property CONFIG.RX_MODE $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "NUM_OF_RX_MB_BUF"] [common::get_property CONFIG.NUM_OF_RX_MB_BUF $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "NUM_OF_TX_BUF"] [common::get_property CONFIG.NUM_OF_TX_BUF $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "C_CAN_CLK_FREQ_HZ"] [common::get_property CONFIG.C_CAN_CLK_FREQ_HZ $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "IS_PL"] [common::get_property IS_PL $periph]$uSuffix"
	}
}
# -----------------------------------------------------------------------------
# This procedure creates XPARs that are canonical/normalized for the hardware
# design parameters.
# -----------------------------------------------------------------------------
proc xdefine_params_canonical {file_handle periph device_id} {
	set uSuffix "U"
	set is_pl [common::get_property IS_PL $periph]
	puts $file_handle "\n/* Canonical definitions for peripheral [string toupper [get_property NAME $periph]] */"
	set canonical_tag [string toupper [format "XPAR_CANFD_%d" $device_id]]

	set canonical_name [format "%s_DEVICE_ID" $canonical_tag]
	puts $file_handle "\#define $canonical_name $device_id$uSuffix"
	add_field_to_periph_config_struct $device_id $canonical_name

	if {$is_pl == 1} {
		set avail_param [list_property [get_cells -hier $periph]]

		set canonical_name [format "%s_BASEADDR" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_BASEADDR]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_RX_MODE" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph RX_MODE]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_NUM_OF_RX_MB_BUF" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph NUM_OF_RX_MB_BUF]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_NUM_OF_TX_BUF" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph NUM_OF_TX_BUF]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_IS_PL" $canonical_tag]
		puts $file_handle "\#define $canonical_name [common::get_property IS_PL $periph]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_RX_FIFO_0_DEPTH" $canonical_tag]
		if {[lsearch -nocase $avail_param "CONFIG.C_EN_APB"] >= 0} {
			puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_RX_FIFO_0_DEPTH]$uSuffix"
		}
	} else {
		set canonical_name [format "%s_BASEADDR" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_S_AXI_BASEADDR]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_HIGHADDR" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_S_AXI_HIGHADDR]$uSuffix"

		set canonical_name [format "%s_RX_MODE" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph RX_MODE]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_NUM_OF_RX_MB_BUF" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph NUM_OF_RX_MB_BUF]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_NUM_OF_TX_BUF" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph NUM_OF_TX_BUF]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		set canonical_name [format "%s_CAN_CLK_FREQ_HZ" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_CAN_CLK_FREQ_HZ]$uSuffix"

		set canonical_name [format "%s_IS_PL" $canonical_tag]
		puts $file_handle "\#define $canonical_name [common::get_property IS_PL $periph]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name
		}
}
