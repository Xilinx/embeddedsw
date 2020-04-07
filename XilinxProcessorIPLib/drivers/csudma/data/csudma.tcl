###############################################################################
# Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   vnsld   22/10/14 First release
# 1.4	adk	09/19/18 Use -hier option while using get_cells command to
#			 support hierarchical designs.
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XCsuDma" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_CSUDMA_CLK_FREQ_HZ"

    generate_dmatype_param $drv_handle "xparameters.h"
    ::hsi::utils::define_zynq_config_file $drv_handle "xcsudma_g.c" "XCsuDma"  "DEVICE_ID" "C_S_AXI_BASEADDR" "DMATYPE"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XCsuDma" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_CSUDMA_CLK_FREQ_HZ"

}

proc generate_dmatype_param {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]

	# Get all peripherals connected to this driver
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]
	foreach ip $ips {
		set iptype [common::get_property IP_NAME [get_cells -hier $ip]]
                set canonical_name [string toupper [format "XPAR_%s_DMATYPE" $ip]]
		if {$iptype == "psu_pmcdma" || $iptype == "psv_pmc_dma"} {
			if {$ip == "psu_pmcdma_0" || $ip == "psv_pmc_dma_0" } {
				puts $file_handle "#define $canonical_name 1"
			} elseif {$ip == "psu_pmcdma_1" || $ip == "psv_pmc_dma_1"} {
				puts $file_handle "#define $canonical_name 2"
			}
		} else {
			puts $file_handle "#define $canonical_name 0"
		}
	}
	close $file_handle
}
