###############################################################################
#
# Copyright (C) 2004 - 2014 Xilinx, Inc.  All rights reserved.
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
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
# -------- ------ -------- ------------------------------------
# 3.0      adk    12/10/13 Updated as per the New Tcl API's
# 4.2      nsk    15/09/16 Updated device id for canonical define,
#                          when there is more than one timer perpheral
# 4.4      ms     04/18/17 Modified tcl file to add suffix U for all macros
#                          definitions of tmrctr in xparameters.h
##############################################################################


proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XTmrCtr" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "CLOCK_FREQ_HZ"
    ::hsi::utils::define_config_file $drv_handle "xtmrctr_g.c" "XTmrCtr"  "DEVICE_ID" "C_BASEADDR" "CLOCK_FREQ_HZ"
    xdefine_canonical_xpars $drv_handle "xparameters.h" "TmrCtr" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "CLOCK_FREQ_HZ"
}

#
# Given a list of arguments, define them all in an include file.
# Handles mpd and mld parameters, as well as the special parameters NUM_INSTANCES,
# DEVICE_ID
#
proc xdefine_include_file {drv_handle file_name drv_string args} {
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

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
        set periph_name [string toupper [common::get_property NAME $periph]]
        #set freq [xget_freq $periph]
	set freq [::hsi::utils::get_clk_pin_freq  $periph "S_AXI_ACLK"]

        puts $file_handle ""
        puts $file_handle "/* Definitions for peripheral $periph_name */"
        foreach arg $args {

            if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                set value $device_id
                incr device_id
            } elseif {[string compare -nocase "CLOCK_FREQ_HZ" $arg] == 0} {
                if {[llength $freq] == 0} {
                    set freq "100000000"
                    puts "WARNING: Clock frequency information is not available in the design, \
                          for peripheral $periph_name. Assuming a default frequency of 100MHz. \
                          If this is incorrect, the peripheral $periph_name will be non-functional"
                }
                set value $freq
            } else {
                set value [common::get_property CONFIG.$arg $periph]
            }
            if {[llength $value] == 0} {
                set value 0
            }
            set value [::hsi::utils::format_addr_string $value $arg]
            puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value$uSuffix"
        }
        puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}


#
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all the peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Get the names of all the peripherals connected to this driver
    foreach periph $periphs {
        set peripheral_name [string toupper [common::get_property NAME $periph]]
        lappend peripherals $peripheral_name
    }

    # Get possible canonical names for all the peripherals connected to this driver
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
    set device_id 0
    foreach periph $periphs {
        set periph_name [string toupper [common::get_property NAME $periph]]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {
            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            foreach arg $args {
                set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]

                #handle CLOCK_FREQ_HZ as a special case
                if {[string compare -nocase "CLOCK_FREQ_HZ" $arg] == 0} {
                    set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
                } elseif {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                             set rvalue $device_id
	                      incr device_id
		} else {
                    set rvalue [common::get_property CONFIG.$arg $periph]
                    if {[llength $rvalue] == 0} {
                        set rvalue 0
                    }
                    set rvalue [::hsi::utils::format_addr_string $rvalue $arg]
                }

		set uSuffix [xdefine_getSuffix $lvalue $rvalue]
                puts $file_handle "#define $lvalue $rvalue$uSuffix"
            }

            incr i
        }
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

proc xdefine_getSuffix {arg_name value} {
		set uSuffix ""
		if { [string match "*CLOCK_FREQ_HZ" $value] == 0 } {
			set uSuffix "U"
		}
		return $uSuffix
}

