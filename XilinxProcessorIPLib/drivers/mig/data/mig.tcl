###############################################################################
# Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 1.0 	   adk    01/08/16 Initial
##############################################################################

#uses "xillib_sw.tcl"

proc generate {drv_handle} {


    #---------------------------------
    # memory_banks in xparameters.h
    #---------------------------------
    hsi::utils::define_addr_params $drv_handle "xparameters.h"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "Mig"

}

proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [hsi::utils::open_include_file $file_name]

    # Get all the peripherals connected to this driver
    set periphs [hsi::utils::get_common_driver_ips $drv_handle]

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

            set addr_params ""
            set addr_params [hsi::utils::find_addr_params $periph]
            set arguments [concat $args $addr_params]
            set memtype [common::get_property CONFIG.MEM_TYPE $periph]

	    foreach arg $arguments {
                set lvalue [hsi::utils::get_driver_param_name $canonical_name $arg]

                # The commented out rvalue is the name of the instance-specific constant
                # set rvalue [hsi::utils::get_ip_param_name $periph $arg]
                # The rvalue set below is the actual value of the parameter
                if { [string match *WIDTH* $arg] } {
			set vars [split $arg "_"]
			lassign $vars var1 var2 var3
			set arg "${memtype}_${var2}_WIDTH"
                }
                set rvalue [hsi::utils::get_param_value $periph $arg]
                if {[llength $rvalue] == 0} {
                    set rvalue 0
                }
                set rvalue [hsi::utils::format_addr_string $rvalue $arg]

                puts $file_handle "#define $lvalue $rvalue"
            }

            puts $file_handle ""
            incr i
        }
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}
