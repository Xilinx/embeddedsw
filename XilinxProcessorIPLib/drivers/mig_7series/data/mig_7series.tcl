###############################################################################
#
# Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
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
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 2.0      adk    12/10/13 Updated as per the New Tcl API's
# 2.0 	   bss    03/04/14 Modified to generate WIDTH parameters.CR# 717423
# 2.1      ms     04/18/17 Modified tcl file to add suffix U for all macros
#                          definitions of mig_7series in xparameters.h
##############################################################################

#uses "xillib_sw.tcl"

proc generate {drv_handle} {
 
    # Definitions in xparameters.h
    xdefine_include_file $drv_handle "xparameters.h" "XMig7series" "NUM_INSTANCES" "DEVICE_ID" "ROW_WIDTH" "COL_WIDTH" "BANK_WIDTH" "DQ_WIDTH"
    
    #---------------------------------
    # memory_banks in xparameters.h
    #---------------------------------
    hsi::utils::define_addr_params $drv_handle "xparameters.h"
    
    xdefine_canonical_xpars $drv_handle "xparameters.h" "Mig7series" "DEVICE_ID" "DDR_ROW_WIDTH" "DDR_COL_WIDTH" "DDR_BANK_WIDTH" "DDR_DQ_WIDTH"
    
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

proc xdefine_include_file {drv_handle file_name drv_string args} {
    set args [hsi::utils::get_exact_arg_list $args]
    # Open include file
    set file_handle [hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [hsi::utils::get_common_driver_ips $drv_handle]

    set uSuffix "U"
    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [common::get_property name $drv_handle]] */"
        # Define NUM_INSTANCES
        puts $file_handle "#define [hsi::utils::get_driver_param_name $drv_string $arg] [llength $periphs]$uSuffix"
        set args [lreplace $args $posn $posn]
    }

    # Check if it is a driver parameter
    lappend newargs 
    foreach arg $args {
        set value [common::get_property CONFIG.$arg $drv_handle]
        if {[llength $value] == 0} {
            lappend newargs $arg
        } else {
            puts $file_handle "#define [hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property $arg $drv_handle]$uSuffix"
        }
    }
    set args $newargs

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
        puts $file_handle ""
        puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
        set memtype [common::get_property CONFIG.MEM_TYPE $periph]
        foreach arg $args {
            if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                set value $device_id
                incr device_id
            } else {
		set value [common::get_property "CONFIG.${memtype}_$arg" $periph]
            }
            if {[llength $value] == 0} {
                set value 0
            }
            set value [hsi::utils::format_addr_string $value $arg]
            if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define [hsi::utils::get_ip_param_name $periph $arg] \"$value\""
            } else {
                 if { [string match *WIDTH* $arg] } {
			puts $file_handle "#define [hsi::utils::get_ip_param_name $periph ${memtype}_$arg] $value$uSuffix"
                 } else {
			puts $file_handle "#define [hsi::utils::get_ip_param_name $periph $arg] $value$uSuffix"
                 }
            }
        }
        puts $file_handle ""
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