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
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XScuTimer" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
    set intr_wrap [common::get_property CONFIG.xil_interrupt [hsi::get_os]]
    if { [string match -nocase $intr_wrap "true"] > 0} {
        ::hsi::utils::define_zynq_config_file $drv_handle "xscutimer_g.c" "XScuTimer" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_INTERRUPT" "C_INTR_PARENT"
    } else {
        ::hsi::utils::define_zynq_config_file $drv_handle "xscutimer_g.c" "XScuTimer" "DEVICE_ID" "C_S_AXI_BASEADDR"
    }
    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XScuTimer" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

    if { [string match -nocase $intr_wrap "true"] > 0} {
        foreach i [get_sw_cores standalone*] {
            set intr_wrapper_tcl_file "[get_property "REPOSITORY" $i]/data/intr_wrapper.tcl"
            if {[file exists $intr_wrapper_tcl_file]} {
                source $intr_wrapper_tcl_file
                break
            }
        }
	gen_intr $drv_handle "xparameters.h"
    }
}
