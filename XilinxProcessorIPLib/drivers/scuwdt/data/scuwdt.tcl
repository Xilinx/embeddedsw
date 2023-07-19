###############################################################################
# Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
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
# 1.00a sdm  11/22/11 Created
# 2.0   adk  12/10/13 Updated as per the New Tcl API's
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XScuWdt" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

    ::hsi::utils::define_zynq_config_file $drv_handle "xscuwdt_g.c" "XScuWdt" "DEVICE_ID" "C_S_AXI_BASEADDR"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "ScuWdt" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

}
