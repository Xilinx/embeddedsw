###############################################################################
# Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   nsk  02/19/18 First release
# 1.1   mus  07/31/19 Add CCI support at EL1 NS
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XOspiPsv" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_OSPI_CLK_FREQ_HZ" "C_OSPI_MODE"

     generate_ospipsv_params $drv_handle "xparameters.h"
    ::hsi::utils::define_zynq_config_file $drv_handle "xospipsv_g.c" "XOspiPsv"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_OSPI_CLK_FREQ_HZ" "IS_CACHE_COHERENT" "C_OSPI_MODE"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XOspiPsv" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_OSPI_CLK_FREQ_HZ" "IS_CACHE_COHERENT" "C_OSPI_MODE"

}

proc generate_ospipsv_params {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	# Get all peripherals connected to this driver
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]

	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

	foreach ip $ips {
		set is_cc 0
		if {$processor_type == "psv_cortexa72"} {
			set extra_flags [common::get_property CONFIG.extra_compiler_flags [hsi::get_sw_processor]]
			set flagindex [string first {-DARMA72_EL3} $extra_flags 0]
			if {$flagindex == -1} {
				set is_cc [common::get_property CONFIG.IS_CACHE_COHERENT $ip]
			}
		}
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_CACHE_COHERENT"] $is_cc"
	}
	close $file_handle
}
