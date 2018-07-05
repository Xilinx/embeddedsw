###############################################################################
# Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a sdm  11/22/11 Created
# 3.9   sd   02/22/20 Added clock support
# 3.9   sd   03/27/20 Added fix for hierarchial designs
# 3.9   sd   04/16/20 Remove a print
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
	set isclocking [check_clocking]
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]
	foreach ip $ips {
		set ref_tag 0xff
		puts $file_handle "/* Definition for input Clock */"
		#set canonical_name_ref [format "XPAR_%s_REF_CLK" $canonical_tag]
		if { $iszynqmp == 1 } {
			set ipname [common::get_property NAME $ip]
			set pos [string length $ipname]
			set num [ expr {$pos -1} ]
			set index [string index $ipname $num]
			set ref_tag [string toupper [format "UART%d_REF" $index ]]
		}
		if { $isclocking == 1 } {
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "REF_CLK"] $ref_tag"
		}
		#puts $file_handle "\#define $canonical_name_ref  $ref_tag"
		incr device_id
	}
	close $file_handle
}
proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XUartPs" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_UART_CLK_FREQ_HZ" "C_HAS_MODEM"

	set clocking [common::get_property CONFIG.clocking [hsi::get_os]]
	set is_zynqmp_fsbl_bsp [common::get_property CONFIG.ZYNQMP_FSBL_BSP [hsi::get_os]]
	set cortexa53proc [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa53"}]
	set isclocking [check_clocking]

	if { $isclocking == 1 &&  $is_zynqmp_fsbl_bsp != true   &&  [llength $cortexa53proc] > 0 && [string match -nocase $clocking "true"] > 0} {
    ::hsi::utils::define_zynq_config_file $drv_handle "xuartps_g.c" "XUartPs"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_UART_CLK_FREQ_HZ" "C_HAS_MODEM" "REF_CLK"
	} else {
    ::hsi::utils::define_zynq_config_file $drv_handle "xuartps_g.c" "XUartPs"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_UART_CLK_FREQ_HZ" "C_HAS_MODEM"
	}

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XUartPs" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_UART_CLK_FREQ_HZ" "C_HAS_MODEM"

    generate_ref_params $drv_handle "xparameters.h"
}
