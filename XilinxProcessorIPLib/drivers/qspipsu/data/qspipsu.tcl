###############################################################################
#
# Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
# 1.0   hk  08/21/14 First release
#       sk  05/06/15 Imported Bus Width Parameter.
# 1.5	nsk 08/14/17 Added CCI support
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XQspiPsu" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_QSPI_CLK_FREQ_HZ" "C_QSPI_MODE" "C_QSPI_BUS_WIDTH"
    generate_cci_params $drv_handle "xparameters.h"

    ::hsi::utils::define_zynq_config_file $drv_handle "xqspipsu_g.c" "XQspiPsu"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_QSPI_CLK_FREQ_HZ" "C_QSPI_MODE" "C_QSPI_BUS_WIDTH" "IS_CACHE_COHERENT"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XQspiPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_QSPI_CLK_FREQ_HZ" "C_QSPI_MODE" "C_QSPI_BUS_WIDTH"

}

proc generate_cci_params {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	# Get all peripherals connected to this driver
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]

	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

	foreach ip $ips {
		set cci_enble 0
		if {$processor_type == "psu_cortexa53"} {
			set hypervisor [common::get_property CONFIG.hypervisor_guest [hsi::get_os]]
			if {[string match -nocase $hypervisor "true"]} {
				set cci_enble [common::get_property CONFIG.IS_CACHE_COHERENT $ip]
			}
		}
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_CACHE_COHERENT"] $cci_enble"
	}
	close $file_handle
}
