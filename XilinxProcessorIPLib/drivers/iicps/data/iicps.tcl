###############################################################################
#
# Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
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
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# 
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
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XIicPs" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_I2C_CLK_FREQ_HZ"

    ::hsi::utils::define_zynq_config_file $drv_handle "xiicps_g.c" "XIicPs"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_I2C_CLK_FREQ_HZ" "REF_CLK"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XIicPs" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_I2C_CLK_FREQ_HZ"

    generate_ref_params $drv_handle "xparameters.h"
}

proc check_platform { } {
	set cortexa53proc [hsi::get_cells -hier -filter "IP_NAME==psu_cortexa53"]
	if {[llength $cortexa53proc] > 0} {
		set iszynqmp 1
	} else {
		set iszynqmp 0
	}
	return $iszynqmp
}

proc generate_ref_params {drv_handle file_name} {
	set device_id 0
	set file_handle [::hsi::utils::open_include_file $file_name]
	set iszynqmp [check_platform]
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]
	puts $ips
	foreach ip $ips {
		set ref_tag 0xff
		puts $file_handle "/* Definition for input Clock */"
		if { $iszynqmp == 1 } {
			set ipname [common::get_property NAME $ip]
			set pos [string length $ipname]
			set num [ expr {$pos -1} ]
			set index [string index $ipname $num]
			set ref_tag [string toupper [format "I2C%d_REF" $index ]]
		}
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "REF_CLK"] $ref_tag"
		incr device_id
	}
	close $file_handle
}
