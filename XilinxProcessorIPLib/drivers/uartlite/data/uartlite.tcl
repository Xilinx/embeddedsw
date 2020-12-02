###############################################################################
# Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 2.00a sdm  06/18/10 Updated to not generate duplicate canonical definitions
#                     when canonical names are same as instance specific names
# 3.0   adk  12/10/13 Updated as per the New Tcl API's
# 3.4   adk  06/04/20 Updated the tcl to not to generate defines for instance
#		      where IP is not configured for uart functionality(i.e
#		      base address is not configured for IP)
#
##############################################################################
#uses "xillib.tcl"

set periph_config_params_uartlite	0
set periph_ninstances           	0
proc generate {drv_handle} {

        xdefine_uartlite_include_file $drv_handle "xparameters.h" "XUartLite"
        xdefine_uartlite_config_file "xuartlite_g.c" "XUartLite"
}
proc init_periph_config_struct { deviceid } {
        global periph_config_params_uartlite
        set periph_config_params_uartlite($deviceid) [list]
}

proc get_periph_config_struct_fields { deviceid } {
        global periph_config_params_uartlite
        return $periph_config_params_uartlite($deviceid)
}
proc add_field_to_periph_config_struct { deviceid fieldval } {
        global periph_config_params_uartlite
        lappend periph_config_params_uartlite($deviceid) $fieldval
}


proc xdefine_uartlite_include_file {drv_handle file_name drv_string} {
        global periph_ninstances

        # Open include file
        set file_handle [::hsi::utils::open_include_file $file_name]

        # Get all peripherals connected to this driver
        set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
        set tmp_periphs ""
        foreach periph $periphs {
            set is_pl [common::get_property IS_PL $periph]
            if {$is_pl == 1} {
                set isaddr_exist [common::get_property CONFIG.C_BASEADDR $periph]
            } else {
                set isaddr_exist 1
            }
            if {${isaddr_exist} != ""} {
                lappend tmp_periphs $periph
            }
        }

        set periphs $tmp_periphs
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

proc xdefine_uartlite_config_file {file_name drv_string} {
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
	set ip_name [common::get_property IP_NAME  $periph]
        if {$is_pl == 1} {
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DEVICE_ID"] $device_id$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] [common::get_property CONFIG.C_BASEADDR $periph]$uSuffix"
                puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "HIGHADDR"] [common::get_property CONFIG.C_HIGHADDR $periph]$uSuffix"
		if {$ip_name != "mdm" && $ip_name != "tmr_sem"} {
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BAUDRATE"] [common::get_property CONFIG.C_BAUDRATE $periph]$uSuffix"
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "USE_PARITY"] [common::get_property CONFIG.C_USE_PARITY $periph]$uSuffix"
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "ODD_PARITY"] [common::get_property CONFIG.C_ODD_PARITY $periph]$uSuffix"
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DATA_BITS"] [common::get_property CONFIG.C_DATA_BITS $periph]$uSuffix"
		} else {
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BAUDRATE"] 0$uSuffix"
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "USE_PARITY"] 0$uSuffix"
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "ODD_PARITY"] 0$uSuffix"
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DATA_BITS"] 0$uSuffix"
		}

	} else {
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DEVICE_ID"] $device_id$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] [common::get_property CONFIG.C_S_AXI_BASEADDR $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "HIGHADDR"] [common::get_property CONFIG.C_S_AXI_HIGHADDR $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BAUDRATE"] 0$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "USE_PARITY"] 0$uSuffix"
                puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "ODD_PARITY"] 0$uSuffix"
                puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DATA_BITS"] 0$uSuffix"
	}
}

proc xdefine_params_canonical {file_handle periph device_id} {
	set uSuffix "U"
	puts $file_handle "\n/* Canonical definitions for peripheral [string toupper [get_property NAME $periph]] */"
	set is_pl [common::get_property IS_PL $periph]
	set ip_name [common::get_property IP_NAME  $periph]

	set canonical_tag [string toupper [format "XPAR_UARTLITE_%d" $device_id]]

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

		if {$ip_name != "mdm" && $ip_name != "tmr_sem"} {
			# Handle BAUDRATE argument
			set canonical_name [format "%s_BAUDRATE" $canonical_tag]
			puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_BAUDRATE]$uSuffix"
			add_field_to_periph_config_struct $device_id $canonical_name

			# Handle USE_PARITY argument
			set canonical_name [format "%s_USE_PARITY" $canonical_tag]
			puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_USE_PARITY]$uSuffix"
			add_field_to_periph_config_struct $device_id $canonical_name

			# Handle ODD_PARITY argument
			set canonical_name [format "%s_ODD_PARITY" $canonical_tag]
			puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_ODD_PARITY]$uSuffix"
			add_field_to_periph_config_struct $device_id $canonical_name

			# Handle DATA_BITS argument
			set canonical_name [format "%s_DATA_BITS" $canonical_tag]
			puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_DATA_BITS]$uSuffix"
			add_field_to_periph_config_struct $device_id $canonical_name
		} else {
			# Handle BAUDRATE argument
			set canonical_name [format "%s_BAUDRATE" $canonical_tag]
			puts $file_handle "\#define $canonical_name 0$uSuffix"
			add_field_to_periph_config_struct $device_id $canonical_name

			# Handle USE_PARITY argument
			set canonical_name [format "%s_USE_PARITY" $canonical_tag]
			puts $file_handle "\#define $canonical_name 0$uSuffix"
			add_field_to_periph_config_struct $device_id $canonical_name

			# Handle ODD_PARITY argument
			set canonical_name [format "%s_ODD_PARITY" $canonical_tag]
			puts $file_handle "\#define $canonical_name 0$uSuffix"
			add_field_to_periph_config_struct $device_id $canonical_name

			# Handle DATA_BITS argument
			set canonical_name [format "%s_DATA_BITS" $canonical_tag]
			puts $file_handle "\#define $canonical_name 0$uSuffix"
			add_field_to_periph_config_struct $device_id $canonical_name
		}
	} else {
		# Handle BASEADDR argument
		set canonical_name [format "%s_BASEADDR" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_S_AXI_BASEADDR]$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		# Handle HIGHADDR argument
		set canonical_name [format "%s_HIGHADDR" $canonical_tag]
		puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_S_AXI_HIGHADDR]$uSuffix"

		# Handle BAUDRATE argument
		set canonical_name [format "%s_BAUDRATE" $canonical_tag]
		puts $file_handle "\#define $canonical_name 0$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		# Handle USE_PARITY argument
		set canonical_name [format "%s_USE_PARITY" $canonical_tag]
		puts $file_handle "\#define $canonical_name 0$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		# Handle ODD_PARITY argument
		set canonical_name [format "%s_ODD_PARITY" $canonical_tag]
		puts $file_handle "\#define $canonical_name 0$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name

		# Handle DATA_BITS argument
		set canonical_name [format "%s_DATA_BITS" $canonical_tag]
		puts $file_handle "\#define $canonical_name 0$uSuffix"
		add_field_to_periph_config_struct $device_id $canonical_name
	}
}
