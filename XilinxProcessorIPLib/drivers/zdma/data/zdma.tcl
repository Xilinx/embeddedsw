###############################################################################
#
# Copyright (C) 2014-2019 Xilinx, Inc.  All rights reserved.
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
# Ver   Who   Date     Changes
# ----- ----  -------- -----------------------------------------------
# 1.0   vnsld   2/24/15  First release
# 1.3   mus     08/14/17 Export cache coherency information
# 1.6	adk	07/09/18 Use -hier option while using get_cells command to
#			 support hierarchical designs.
# 1.7   adk     08/03/19 Add support for versal.
# 1.8   mus     07/30/19 Added CCI support for Versal at EL1 NS
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {

    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XZDma" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_DMA_MODE" "C_S_AXI_HIGHADDR" "C_ZDMA_CLK_FREQ_HZ"
    set is_xen [common::get_property CONFIG.hypervisor_guest [hsi::get_os]]
    generate_cci_params $drv_handle "xparameters.h"
    ::hsi::utils::define_zynq_config_file $drv_handle "xzdma_g.c" "XZDma"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_DMA_MODE" "IS_CACHE_COHERENT"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XZDma" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_DMA_MODE" "C_S_AXI_HIGHADDR" "C_ZDMA_CLK_FREQ_HZ"

}
proc generate_cci_params {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	# Get all peripherals connected to this driver
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]

	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

	foreach ip $ips {
                set is_cc 0
		set iptype [common::get_property IP_NAME [get_cells -hier $ip]]
		if {($processor_type == "psu_cortexa53" && $iptype == "psu_adma") || ($iptype == "psv_adma" && $processor_type == "psv_cortexa72")} {
                        set is_xen [common::get_property CONFIG.hypervisor_guest [hsi::get_os]]
                        set extra_flags [common::get_property CONFIG.extra_compiler_flags [hsi::get_sw_processor]]
                        set flagindex [string first {-DARMA72_EL3} $extra_flags 0]
                        if {(($is_xen == "true") && ($processor_type == "psu_cortexa53"))
				|| (($processor_type == "psv_cortexa72") && ($flagindex == -1))} {
                            set is_cc [common::get_property CONFIG.IS_CACHE_COHERENT $ip]
			}
		}
                puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_CACHE_COHERENT"] $is_cc"
	}
	close $file_handle
}
