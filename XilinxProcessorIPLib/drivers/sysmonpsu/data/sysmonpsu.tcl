###############################################################################
# Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   kvn  12/15/15 First release
# 2.3   mn   03/08/18 Get Ref Clock Frequency information from design
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XSysMonPsu" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

    get_ams_clk_info $drv_handle "xparameters.h"

    ::hsi::utils::define_zynq_config_file $drv_handle "xsysmonpsu_g.c" "XSysMonPsu"  "DEVICE_ID" "C_S_AXI_BASEADDR" "REF_FREQMHZ"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XSysMonPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

    get_ams_clk_info_canonical $drv_handle "xparameters.h"
}

proc get_ams_clk_info {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	set periph_list [get_cells -hier]
	foreach periph $periph_list {
		set zynq_ultra_ps [get_property IP_NAME $periph]
		if {[string match -nocase $zynq_ultra_ps "zynq_ultra_ps_e"] } {
			set avail_param [list_property [get_cells -hier $periph]]
			if {[lsearch -nocase $avail_param "CONFIG.PSU__CRL_APB__AMS_REF_CTRL__ACT_FREQMHZ"] >= 0} {
				set nr_freq [get_property CONFIG.PSU__CRL_APB__AMS_REF_CTRL__ACT_FREQMHZ [get_cells -hier $periph]]
				puts $file_handle "#define XPAR_PSU_AMS_REF_FREQMHZ $nr_freq"
			}
		}
	}
	close $file_handle
}

proc get_ams_clk_info_canonical {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	set periph_list [get_cells -hier]
	foreach periph $periph_list {
		set zynq_ultra_ps [get_property IP_NAME $periph]
		if {[string match -nocase $zynq_ultra_ps "zynq_ultra_ps_e"] } {
			set avail_param [list_property [get_cells -hier $periph]]
			if {[lsearch -nocase $avail_param "CONFIG.PSU__CRL_APB__AMS_REF_CTRL__ACT_FREQMHZ"] >= 0} {
				set nr_freq [get_property CONFIG.PSU__CRL_APB__AMS_REF_CTRL__ACT_FREQMHZ [get_cells -hier $periph]]
				puts $file_handle "#define XPAR_XSYSMONPSU_0_REF_FREQMHZ $nr_freq"
			}
		}
	}
	close $file_handle
}
