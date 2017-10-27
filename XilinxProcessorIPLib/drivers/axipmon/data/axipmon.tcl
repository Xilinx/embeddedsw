###############################################################################
#
# Copyright (C) 2012 - 2015 Xilinx, Inc.  All rights reserved.
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
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
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
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 6.1     adk    16/04/14  Added two new parameters(C_S_AXI4_BASEADDR,
#			   C_S_AXI4_HIGHADDR)
# 6.2	  bss	 02/03/15  Added support to handle
#				- Zynq MP APM Baseaddress(C_S_AXI_BASEADDR)
#				- SoftIP APM Baseaddress(C_BASEADDR)
# 6.6     ms     04/18/17  Modified tcl file to add suffix U for all macros
#                          definitions of axipmon in xparameters.h
###############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {

  apm_define_include_file $drv_handle "xparameters.h" "XAxiPmon" "NUM_INSTANCES" "DEVICE_ID" C_BASEADDR C_HIGHADDR "C_GLOBAL_COUNT_WIDTH" "C_METRICS_SAMPLE_COUNT_WIDTH" "C_ENABLE_EVENT_COUNT" "C_NUM_MONITOR_SLOTS" "C_NUM_OF_COUNTERS" "C_HAVE_SAMPLED_METRIC_CNT" "C_ENABLE_EVENT_LOG" "C_FIFO_AXIS_DEPTH" "C_FIFO_AXIS_TDATA_WIDTH" "C_FIFO_AXIS_TID_WIDTH" "C_METRIC_COUNT_SCALE" "C_ENABLE_ADVANCED" "C_ENABLE_PROFILE" "C_ENABLE_TRACE" "C_S_AXI4_BASEADDR" "C_S_AXI4_HIGHADDR" "C_ENABLE_32BIT_FILTER_ID"
  ::hsi::utils::define_config_file  $drv_handle "xaxipmon_g.c" "XAxiPmon" "DEVICE_ID" C_BASEADDR "C_GLOBAL_COUNT_WIDTH" "C_METRICS_SAMPLE_COUNT_WIDTH" "C_ENABLE_EVENT_COUNT" "C_NUM_MONITOR_SLOTS" "C_NUM_OF_COUNTERS" "C_HAVE_SAMPLED_METRIC_CNT" "C_ENABLE_EVENT_LOG" "C_FIFO_AXIS_DEPTH" "C_FIFO_AXIS_TDATA_WIDTH" "C_FIFO_AXIS_TID_WIDTH" "C_METRIC_COUNT_SCALE" "C_ENABLE_ADVANCED" "C_ENABLE_PROFILE" "C_ENABLE_TRACE" "C_ENABLE_32BIT_FILTER_ID"
  apm_define_canonical_xpars $drv_handle "xparameters.h" "AxiPmon" "DEVICE_ID" C_BASEADDR C_HIGHADDR "C_GLOBAL_COUNT_WIDTH" "C_METRICS_SAMPLE_COUNT_WIDTH" "C_ENABLE_EVENT_COUNT" "C_NUM_MONITOR_SLOTS" "C_NUM_OF_COUNTERS" "C_HAVE_SAMPLED_METRIC_CNT" "C_ENABLE_EVENT_LOG" "C_FIFO_AXIS_DEPTH" "C_FIFO_AXIS_TDATA_WIDTH" "C_FIFO_AXIS_TID_WIDTH" "C_METRIC_COUNT_SCALE" "C_ENABLE_ADVANCED" "C_ENABLE_PROFILE" "C_ENABLE_TRACE" "C_S_AXI4_BASEADDR" "C_S_AXI4_HIGHADDR" "C_ENABLE_32BIT_FILTER_ID"
}

proc apm_define_include_file {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
    set uSuffix "U"
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [common::get_property name $drv_handle]] */"
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
            puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property $arg $drv_handle]$uSuffix"
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
            } elseif {[string compare -nocase "C_BASEADDR" $arg] == 0} {
		set value [common::get_property CONFIG.$arg $periph]
		if {[llength $value] == 0} {
			set value [common::get_property CONFIG.C_S_AXI_BASEADDR $periph]
		}
             } elseif {[string compare -nocase "C_HIGHADDR" $arg] == 0} {
		set value [common::get_property CONFIG.$arg $periph]
	        if {[llength $value] == 0} {
			set value [common::get_property CONFIG.C_S_AXI_HIGHADDR $periph]
	        }
            } else {
		set value [common::get_property CONFIG.$arg $periph]
	    }

	    if {[llength $value] == 0} {
                set value 0
            }
            set value [::hsi::utils::format_addr_string $value $arg]
            if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] \"$value\""
            } else {
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value$uSuffix"
            }
        }
        puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

proc apm_define_canonical_xpars {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
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

               # The commented out rvalue is the name of the instance-specific constant
               # set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
               # The rvalue set below is the actual value of the parameter

		if {[string compare -nocase "C_BASEADDR" $arg] == 0} {
			set rvalue [::hsi::utils::get_param_value $periph $arg]
			if {[llength $rvalue] == 0} {
				set rvalue [common::get_property CONFIG.C_S_AXI_BASEADDR $periph]
			}
		} elseif {[string compare -nocase "C_HIGHADDR" $arg] == 0} {
			set rvalue [::hsi::utils::get_param_value $periph $arg]
			if {[llength $rvalue] == 0} {
				set rvalue [common::get_property CONFIG.C_S_AXI_HIGHADDR $periph]
			}
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
		if { [string match "*DEVICE_ID" $value] == 0} {
			set uSuffix "U"
		}
		return $uSuffix
}
