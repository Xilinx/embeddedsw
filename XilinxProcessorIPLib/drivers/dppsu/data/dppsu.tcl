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
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XDpPsu" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
    ::hsi::utils::define_zynq_config_file $drv_handle "xdppsu_g.c" "XDpPsu" "DEVICE_ID" "C_S_AXI_BASEADDR"
    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XDpPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
}
