###############################################################################
# Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a sdm  11/22/11 Created
# 2.0   adk  12/10/13 Updated as per the New Tcl API's
# 2.3   ms   04/18/17 Modified tcl file to add suffix U for all macros
#                     definitions of nandps in xparameters.h
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    xdefine_zynq_include_file $drv_handle "xparameters.h" "XNandPs" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_NAND_CLK_FREQ_HZ" "C_SMC_BASEADDR" "C_NAND_WIDTH"

    ::hsi::utils::define_zynq_config_file $drv_handle "xnandps_g.c" "XNandPs"  "DEVICE_ID" "C_SMC_BASEADDR" "C_S_AXI_BASEADDR" "C_NAND_WIDTH"

    xdefine_zynq_canonical_xpars $drv_handle "xparameters.h" "XNandPs" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_NAND_CLK_FREQ_HZ" "C_SMC_BASEADDR" "C_NAND_WIDTH"

}


#
# Given a list of arguments, define them all in an include file.
# Similar to proc xdefine_include_file, except that uses regsub
# to replace "S_AXI_" with "".
#
proc xdefine_zynq_include_file {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Handle special cases
    set arg "NUM_INSTANCES"
    set uSuffix "U"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
	puts $file_handle "/* Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
	# Define NUM_INSTANCES
	puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [llength $periphs]$uSuffix"
	set args [lreplace $args $posn $posn]
    }
    # Check if it is a driver parameter

    lappend newargs
    foreach arg $args {
	set value [common::get_property CONFIG.$arg $drv_handle]
	if {[llength $value] == 0} {
	    lappend newargs $arg
	} else {
	    puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property CONFIG.$arg $drv_handle]$uSuffix"
	}
    }
    set args $newargs

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
	puts $file_handle ""
	puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
	foreach arg $args {
	    if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
		set value $device_id
		incr device_id
	    } elseif {[string compare -nocase "C_SMC_BASEADDR" $arg] == 0} {
		set value 0xE000E000
	    } else {
		set value [::hsi::utils::get_param_value $periph $arg]
	    }
	    if {[llength $value] == 0} {
		set value 0
	    }
	    set value [::hsi::utils::format_addr_string $value $arg]
	    set arg_name [::hsi::utils::get_ip_param_name $periph $arg]
	    regsub "S_AXI_" $arg_name "" arg_name
	    if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define $arg_name \"$value\""
	    } else {
                puts $file_handle "#define $arg_name $value$uSuffix"
            }
	}
	puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

#-----------------------------------------------------------------------------
# xdefine_zynq_canonical_xpars - Used to print out canonical defines for a driver.
# Similar to proc xdefine_config_file, except that uses regsub to replace "S_AXI_"
# with "".
#-----------------------------------------------------------------------------
proc xdefine_zynq_canonical_xpars {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all the peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Get the names of all the peripherals connected to this driver
    foreach periph $periphs {
        set peripheral_name [string toupper [common::get_property NAME $periph]]
        lappend peripherals $peripheral_name
    }

    # Get possible canonical names for all the peripherals connected to this
    # driver
    set device_id 0
    foreach periph $periphs {
        set canonical_name [string toupper [format "%s_%s" $drv_string $device_id]]
        lappend canonicals $canonical_name

        # Create a list of IDs of the peripherals whose hardware instance name
        # doesn't match the canonical name. These IDs can be used later to
        # generate canonical definitions
        if { [lsearch $peripherals $canonical_name] < 0 } {
            lappend indices $device_id
        }
        incr device_id
    }

    set i 0
    foreach periph $periphs {
        set periph_name [string toupper [common::get_property NAME $periph]]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {
            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            foreach arg $args {
                set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]
                # replace S_SXI_ with CPU_. This is a temporary fix. Revist when the
                # S_AXI_DIST_BASEADDR is generated by the tools
                regsub "S_AXI_" $lvalue "CPU_" lvalue

                # The commented out rvalue is the name of the instance-specific constant
                # set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
                # The rvalue set below is the actual value of the parameter
                if {[string compare -nocase "C_SMC_BASEADDR" $arg] == 0} {
                    set rvalue 0xE000E000
                } else {
                    set rvalue [::hsi::utils::get_param_value $periph $arg]
                }
                if {[llength $rvalue] == 0} {
                    set rvalue 0
                }
                set rvalue [::hsi::utils::format_addr_string $rvalue $arg]

		set uSuffix [xdefine_getSuffix $lvalue $rvalue]
                puts $file_handle "#define $lvalue $rvalue$uSuffix"

            }
            puts $file_handle ""
            incr i
        }
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

proc xdefine_getSuffix {arg_name value} {
	set uSuffix ""
	if { [string match "*DEVICE_ID" $value] == 0 } {
		set uSuffix "U"
	}
	return $uSuffix
}