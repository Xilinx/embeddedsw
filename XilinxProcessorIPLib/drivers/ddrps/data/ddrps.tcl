################################################################################
 #
 # Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
 #
 # Permission is hereby granted, free of charge, to any person obtaining a copy
 # of this software and associated documentation files (the "Software"), to deal
 # in the Software without restriction, including without limitation the rights
 # to use, copy, modify, merge, publish, distribute, sublicense, and#or sell
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
 # FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 # XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 # -------- ------ -------- ------------------------------------
 # 1.0	    nsk	   08/06/15 First Release.
 # 1.0	    nsk	   08/20/15 Updated define_addr_params to support
 #			    PBD Designs (CR#876857).
 #
 ################################################################################

proc generate {drv_handle} {
      define_addr_params $drv_handle "xparameters.h"
}

proc define_addr_params {drv_handle file_name} {

   # Open include file
   set file_handle [::hsi::utils::open_include_file $file_name]

   set sw_proc [hsi::get_sw_processor]
   set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
   foreach periph $periphs {
	puts $file_handle ""
	puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
	set addr_params [list]
	set interface_base_names [get_property BASE_NAME [get_mem_ranges \
		-of_objects [get_cells -hier $sw_proc] $periph]]
	set interface_high_names [get_property HIGH_NAME [get_mem_ranges \
		-of_objects [get_cells -hier $sw_proc] $periph]]
	set i 0
	foreach interface_base $interface_base_names interface_high $interface_high_names {
		set base_name [common::get_property BASE_NAME [lindex [get_mem_ranges \
			-of_objects [get_cells -hier $sw_proc] $periph] $i]]
		set base_value [common::get_property BASE_VALUE [lindex [get_mem_ranges \
			-of_objects [get_cells -hier $sw_proc] $periph] $i]]
		set high_name [common::get_property HIGH_NAME [lindex [get_mem_ranges \
			-of_objects [get_cells -hier $sw_proc] $periph] $i]]
		set high_value [common::get_property HIGH_VALUE [lindex [get_mem_ranges \
			-of_objects [get_cells -hier $sw_proc] $periph] $i]]
		set bposn [lsearch -exact $addr_params $base_name]
		set hposn [lsearch -exact $addr_params $high_name]
		if {$bposn > -1  || $hposn > -1 } {
			continue
		}

		puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $base_name] $base_value"
		puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $high_name] $high_value"
		puts $file_handle ""
		incr i
		lappend addr_params $base_name
		lappend addr_params $high_name

	}

   }
   puts $file_handle "\n/******************************************************************/\n"
   close $file_handle
}
