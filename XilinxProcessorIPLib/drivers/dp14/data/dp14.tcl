##******************************************************************************
##
## Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in
## all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
## WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
## OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## Except as contained in this notice, the name of the Xilinx shall not be used
## in advertising or otherwise to promote the sale, use or other dealings in
## this Software without prior written authorization from Xilinx.
##
##*****************************************************************************/
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 5.3   ms   04/18/17 Modified tcl file to add suffix U for all macros
#                     definitions of dp in xparameters.h
##############################################################################
proc generate {drv_handle} {
        xdefine_include_file $drv_handle "xparameters.h" "XDp" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_LANE_COUNT" "C_LINK_RATE" "C_MAX_BITS_PER_COLOR" "C_QUAD_PIXEL_ENABLE" "C_DUAL_PIXEL_ENABLE" "C_YCRCB_ENABLE" "C_YONLY_ENABLE" "C_GT_DATAWIDTH" "C_SECONDARY_SUPPORT" "C_AUDIO_CHANNELS" "C_MST_ENABLE" "C_NUMBER_OF_MST_STREAMS" "C_PROTOCOL_SELECTION" "C_FLOW_DIRECTION" "S_AXI_ACLK"
        ::hsi::utils::define_config_file $drv_handle "xdp_g.c" "XDp" "DEVICE_ID" "C_BASEADDR" "S_AXI_ACLK" "C_LANE_COUNT" "C_LINK_RATE" "C_MAX_BITS_PER_COLOR" "C_QUAD_PIXEL_ENABLE" "C_DUAL_PIXEL_ENABLE" "C_YCRCB_ENABLE" "C_YONLY_ENABLE" "C_GT_DATAWIDTH" "C_SECONDARY_SUPPORT" "C_AUDIO_CHANNELS" "C_MST_ENABLE" "C_NUMBER_OF_MST_STREAMS" "C_PROTOCOL_SELECTION" "C_FLOW_DIRECTION"
        xdefine_canonical_xpars $drv_handle "xparameters.h" "XDp" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_LANE_COUNT" "C_LINK_RATE" "C_MAX_BITS_PER_COLOR" "C_QUAD_PIXEL_ENABLE" "C_DUAL_PIXEL_ENABLE" "C_YCRCB_ENABLE" "C_YONLY_ENABLE" "C_GT_DATAWIDTH" "C_SECONDARY_SUPPORT" "C_AUDIO_CHANNELS" "C_MST_ENABLE" "C_NUMBER_OF_MST_STREAMS" "C_PROTOCOL_SELECTION" "C_FLOW_DIRECTION" "S_AXI_ACLK"
}

#
# Given a list of arguments, define them all in an include file.
# Handles IP model/user parameters, as well as the special parameters NUM_INSTANCES,
# DEVICE_ID
# Will not work for a processor.
#
proc xdefine_include_file {drv_handle file_name drv_string args} {
    set args [get_exact_arg_list $args]
    # Open include file
    set file_handle [xopen_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [xget_sw_iplist_for_driver $drv_handle]

    set uSuffix "U"
    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [get_property name $drv_handle]] */"
        # Define NUM_INSTANCES
        puts $file_handle "#define [xget_dname $drv_string $arg] [llength $periphs]$uSuffix"
        set args [lreplace $args $posn $posn]
    }

    # Check if it is a driver parameter
    lappend newargs
    foreach arg $args {
        set value [get_property CONFIG.$arg $drv_handle]
        if {[llength $value] == 0} {
            lappend newargs $arg
        } else {
            puts $file_handle "#define [xget_dname $drv_string $arg] [get_property $arg $drv_handle]$uSuffix"
        }
    }
    set args $newargs

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
        puts $file_handle ""
        puts $file_handle "/* Definitions for peripheral [string toupper [get_property NAME $periph]] */"

        foreach arg $args {
            if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                set value $device_id
                incr device_id
            } elseif {[string compare -nocase "S_AXI_ACLK" $arg] == 0} {
                set freq [xget_ip_clk_pin_freq $periph $arg]
                if {[llength $freq] == 0} {
                        set freq 0
                        puts "WARNING: Clock frequency information is not available in the design, \
                              for peripheral $periph_name. This will not work."
                }
                set value $freq
            } else {
                set value [get_property CONFIG.$arg $periph]
            }
            if {[llength $value] == 0} {
                set value 0
            }
            set value [xformat_addr_string $value $arg]
            if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define [xget_name $periph $arg] \"$value\""
            } else {
                puts $file_handle "#define [xget_name $periph $arg] $value$uSuffix"
            }
        }
        puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

#
# xdefine_canonical_xpars - Used to print out canonical defines for a driver.
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
    set args [get_exact_arg_list $args]
   # Open include file
   set file_handle [xopen_include_file $file_name]

   # Get all the peripherals connected to this driver
   set periphs [xget_sw_iplist_for_driver $drv_handle]

   # Get the names of all the peripherals connected to this driver
   foreach periph $periphs {
       set peripheral_name [string toupper [get_property NAME $periph]]
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
       set periph_name [string toupper [get_property NAME $periph]]

       # Generate canonical definitions only for the peripherals whose
       # canonical name is not the same as hardware instance name
       if { [lsearch $canonicals $periph_name] < 0 } {
           puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
           set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

           foreach arg $args {
               set lvalue [xget_dname $canonical_name $arg]

               # The commented out rvalue is the name of the instance-specific constant
               # set rvalue [xget_name $periph $arg]
               # The rvalue set below is the actual value of the parameter
                if {[string compare -nocase "S_AXI_ACLK" $arg] == 0} {
                        set freq [xget_ip_clk_pin_freq $periph $arg]
                        if {[llength $freq] == 0} {
                            set freq 0
                            puts "WARNING: Clock frequency information is not available in the design, \
                                  for peripheral $periph_name. This will not work."
                        }
                        set rvalue $freq
                } else {
                    set rvalue [xget_param_value $periph $arg]
                    if {[llength $rvalue] == 0} {
                        set rvalue 0
                    }
                    set rvalue [xformat_addr_string $rvalue $arg]
                }
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
