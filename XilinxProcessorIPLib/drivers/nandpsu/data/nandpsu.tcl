###############################################################################
# Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a nm   05/06/14 Created
# 1.2   ms   02/12/17 Fix for compilation warning.
# 1.3	nsk  14/08/17 Added CCI support
# 1.6	sd   26/02/20 Added Clock support
# 1.6	sd   27/03/20 Added hier design fix
#
##############################################################################

#uses "xillib.tcl"
proc check_clocking { } {
	set sw_proc_handle [hsi::get_sw_processor]
	set slaves [common::get_property   SLAVES [  hsi::get_cells -hier $sw_proc_handle]]
	foreach slave $slaves {
		if {[string compare -nocase "psu_crf_apb" $slave] == 0 } {
			return 1
		}
	}
	return 0
}


proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XNandPsu" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
    generate_cci_params $drv_handle "xparameters.h"
    set clocking [common::get_property CONFIG.clocking [hsi::get_os]]
    set is_zynqmp_fsbl_bsp [common::get_property CONFIG.ZYNQMP_FSBL_BSP [hsi::get_os]]
    set cortexa53proc [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa53"}]
    set isclocking [check_clocking]

    if { $isclocking == 1 &&  $is_zynqmp_fsbl_bsp != true   &&  [llength $cortexa53proc] > 0 && [string match -nocase $clocking "true"] > 0} {

        ::hsi::utils::define_zynq_config_file $drv_handle "xnandpsu_g.c" "XNandPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "IS_CACHE_COHERENT" "REF_CLK"
    } else {
        ::hsi::utils::define_zynq_config_file $drv_handle "xnandpsu_g.c" "XNandPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "IS_CACHE_COHERENT"
    }

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XNandPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
}

proc generate_cci_params {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	# Get all peripherals connected to this driver
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]

	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]
	set isclocking [check_clocking]

	foreach ip $ips {
		set cci_enble 0
		set ref_tag 0xff
		if {$processor_type == "psu_cortexa53"} {
			set hypervisor [common::get_property CONFIG.hypervisor_guest [hsi::get_os]]
			set ref_tag  "NAND_REF"
			if {[string match -nocase $hypervisor "true"]} {
				set cci_enble [common::get_property CONFIG.IS_CACHE_COHERENT $ip]
			}
		}
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_CACHE_COHERENT"] $cci_enble"
		if { $isclocking == 1 } {
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "REF_CLK"] $ref_tag"
		}
	}
	close $file_handle
}
