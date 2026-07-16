###############################################################################
# Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
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
# 1.8   nsk     12/14/20 Modified the tcl to not to use the instance names.
# 1.10	sk	07/09/21 Update get_instance_nr proc address list to support
# 			 SSIT devices.
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
		set dma_val [get_instance_nr $ip]
		if {$dma_val != -1} {
			puts $file_handle "#define $canonical_name [expr $dma_val +1]"
		} else {
			puts $file_handle "#define $canonical_name 0"
		}
	}
	close $file_handle
}

proc get_instance_nr {drv_handle} {
	set list [get_mem_ranges -of_objects [hsi::get_cells -hier [get_sw_processor]]]
	set index [lsearch $list $drv_handle]
	set val -1
	if {$index >= 0} {
		set base_val [common::get_property BASE_VALUE [lindex [get_mem_ranges -of_objects [hsi::get_cells -hier [get_sw_processor]]] $index]]
		set base_val [string trimleft $base_val "0x"]
		set addr_list_1 "F11C0000 1011C0000 1091C0000 1111C0000 1191C0000"
	        set addr_list_2 "F11D0000 1011D0000 1091D0000 1111D0000 1191D0000"
	        if {[lsearch -nocase $addr_list_1 $base_val] >= 0} {
			set val 0
	        }
	        if {[lsearch -nocase $addr_list_2 $base_val] >= 0} {
			set val 1
	        }
	}
	return $val
}
