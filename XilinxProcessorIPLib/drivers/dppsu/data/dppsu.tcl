###############################################################################
# Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  aad   01/27/17 Created
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {

    generate_dp_params $drv_handle "xparameters.h"

    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XDpPsu" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
    ::hsi::utils::define_zynq_config_file $drv_handle "xdppsu_g.c" "XDpPsu" "DEVICE_ID" "C_S_AXI_BASEADDR"
    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XDpPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
}

proc generate_dp_params {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	set periph_list [get_cells -hier]
	set ip [::hsi::utils::get_common_driver_ips $drv_handle]
	foreach periph $periph_list {
	set zynq_ultra_ps [get_property IP_NAME $periph]
		if {[string match -nocase $zynq_ultra_ps "zynq_ultra_ps_e"] } {
			set dp_sel [get_property CONFIG.PSU__DP__LANE_SEL [get_cells -hier $periph]]
			set mode [lindex $dp_sel 0]
			set lan_sel [lindex $dp_sel 1]
			if {[string match -nocase $mode "Single"]} {
				puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "LANE_COUNT"] 1"
			} elseif {[string match -nocase $mode "Dual"]} {
				puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "LANE_COUNT"] 2"
			}
		}
	}
	close $file_handle
}
