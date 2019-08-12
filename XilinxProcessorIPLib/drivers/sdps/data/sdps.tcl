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
# 1.00a hk   10/17/13 First release
# 2.0   adk  12/10/13 Updated as per the New Tcl API's
# 2.4	sk   12/04/14 Added CD and WP parameters
# 3.0   sk   07/16/16 Added BUS WIDTH, MIO BANK and HAS EMIO parameters.
# 3.3   mn   08/17/17 Enabled CCI support for A53 by adding cache coherency
#                     information.
# 3.6   mn   07/06/18 Generate canonical entry for IS_CACHE_COHERENT
# 3.8   mus  07/30/19 Added CCI support for Versal at EL1 NS
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XSdPs" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_SDIO_CLK_FREQ_HZ" "C_HAS_CD" "C_HAS_WP" "C_BUS_WIDTH" "C_MIO_BANK" "C_HAS_EMIO"
	generate_cci_params $drv_handle "xparameters.h"

    ::hsi::utils::define_zynq_config_file $drv_handle "xsdps_g.c" "XSdPs"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_SDIO_CLK_FREQ_HZ" "C_HAS_CD" "C_HAS_WP" "C_BUS_WIDTH" "C_MIO_BANK" "C_HAS_EMIO" "IS_CACHE_COHERENT"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XSdPs" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_SDIO_CLK_FREQ_HZ" "C_HAS_CD" "C_HAS_WP" "C_BUS_WIDTH" "C_MIO_BANK" "C_HAS_EMIO" "IS_CACHE_COHERENT"

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
		} elseif {$processor_type == "psv_cortexa72"} {
			set extra_flags [common::get_property CONFIG.extra_compiler_flags [hsi::get_sw_processor]]
			set flagindex [string first {-DARMA72_EL3} $extra_flags 0]
			if {$flagindex == -1} {
				set cci_enble [common::get_property CONFIG.IS_CACHE_COHERENT $ip]
			}
		}
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_CACHE_COHERENT"] $cci_enble"
       }
       close $file_handle
}
