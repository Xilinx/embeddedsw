###############################################################################
#
# Copyright (C) 2016 - 2019 Xilinx, Inc.  All rights reserved.
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
# 1.0   sg  06/06/16 First release
# 1.3	vak 16/08/17 Export CCI related information
# 1.4	vak 24/09/18 Added SUPER_SPEED parameter
# 1.5	vak 13/02/19 Correct the logic for setting SUPER_SPEED parameter
# 1.6	mus 07/30/19 Added CCI support for Versal at EL1 NS
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XUsbPsu" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

    generate_usb_params $drv_handle "xparameters.h"

    ::hsi::utils::define_zynq_config_file $drv_handle "xusbpsu_g.c" "XUsbPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "IS_CACHE_COHERENT" "SUPER_SPEED"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XUsbPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

}

proc generate_usb_params {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	# Get all peripherals connected to this driver
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]

	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

	foreach ip $ips {
		set is_cc 0
		if {$processor_type == "psu_cortexa53"} {
			set is_xen [common::get_property CONFIG.hypervisor_guest [hsi::get_os]]
			if {$is_xen == "true"} {
				set is_cc [common::get_property CONFIG.IS_CACHE_COHERENT $ip]
			}
		} elseif {$processor_type == "psv_cortexa72"} {
			set extra_flags [common::get_property CONFIG.extra_compiler_flags [hsi::get_sw_processor]]
			set flagindex [string first {-DARMA72_EL3} $extra_flags 0]
			if {$flagindex == -1} {
				set is_cc [common::get_property CONFIG.IS_CACHE_COHERENT $ip]
			}
		}
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_CACHE_COHERENT"] $is_cc"

		set val 0
		set peripheral [get_cells -hier -filter {IP_NAME == zynq_ultra_ps_e}]

		if {$peripheral > 0} {
			set parameters [list_property [get_cells -hier $peripheral]]

			if {[string match -nocase $ip "psu_usb_xhci_0"]} {
				if {[lsearch -nocase $parameters "CONFIG.PSU__USB3_0__PERIPHERAL__ENABLE"] >= 0} {
					set val [get_property CONFIG.PSU__USB3_0__PERIPHERAL__ENABLE [get_cells -hier $peripheral]]
				}
			}

			if {[string match -nocase $ip "psu_usb_xhci_1"]} {
				if {[lsearch -nocase $parameters "CONFIG.PSU__USB3_1__PERIPHERAL__ENABLE"] >= 0} {
					set val [get_property CONFIG.PSU__USB3_1__PERIPHERAL__ENABLE [get_cells -hier $peripheral]]
				}
			}
		}

		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "SUPER_SPEED"] $val"

	}
	close $file_handle
}
