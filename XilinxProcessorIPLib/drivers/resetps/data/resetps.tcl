###############################################################################
#
# Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  cjp  09/05/17 First commit
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    xdefine_zynq_include_file $drv_handle "xparameters.h" "XResetPs" "NUM_INSTANCES" "DEVICE_ID" "BASEADDR"
    xdefine_zynq_config_file $drv_handle "xresetps_g.c" "XResetPs" "DEVICE_ID" "BASEADDR"
}

#
# Given a list of arguments, define them all in an include file.
# Since resetps is a dummy instance we are hardcoding NUM_INSTANCE to 1,
# DEVICE_ID to 0 and dummy BASEADDRESS to 0xFFFFFFFF
#
proc xdefine_zynq_include_file {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set uSuffix "U"
    set num_instance 1
    set device_id 0
    set baseaddr 0xFFFFFFFF

    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
	puts $file_handle "/* Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
	puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] $num_instance$uSuffix"
	set args [lreplace $args $posn $posn]
    }

    set arg "DEVICE_ID"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
	puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $drv_handle]] */"
	puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] $device_id"
	set args [lreplace $args $posn $posn]
    }

    set arg "BASEADDR"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
	puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] $baseaddr$uSuffix"
	set args [lreplace $args $posn $posn]
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

#
# Given a list of arguments, define them all in Configuration C file.
# Since resetps is a dummy instance we are hardcoding Invalid BASEADDRESS for
# proper driver functionality
#
proc xdefine_zynq_config_file {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
   set filename [file join "src" $file_name]
   #file delete $filename
   set config_file [open $filename w]
   ::hsi::utils::write_c_header $config_file "Driver configuration"
   puts $config_file "#include \"xparameters.h\""
   puts $config_file "#include \"[string tolower $drv_string].h\""
   puts $config_file "\n/*"
   puts $config_file "* The configuration table for devices"
   puts $config_file "*/\n"
   set num_insts [::hsi::utils::get_driver_param_name $drv_string "NUM_INSTANCES"]
   puts $config_file [format "%s_Config %s_ConfigTable\[%s\] =" $drv_string $drv_string $num_insts]
   puts $config_file "\{"
   puts $config_file [format "\t\{"]

    set arg "DEVICE_ID"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $config_file "\t\t[::hsi::utils::get_driver_param_name $drv_string $arg],"
	set args [lreplace $args $posn $posn]
    }

    set arg "BASEADDR"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $config_file "\t\t[::hsi::utils::get_driver_param_name $drv_string $arg],"
	set args [lreplace $args $posn $posn]
    }

   puts $config_file "\t\}"
   puts $config_file "\};";

   close $config_file
}
