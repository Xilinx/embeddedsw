###############################################################################
# Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XCanPs" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_CAN_CLK_FREQ_HZ"

    ::hsi::utils::define_zynq_config_file $drv_handle "xcanps_g.c" "XCanPs"  "DEVICE_ID" "C_S_AXI_BASEADDR"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XCanPs" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_CAN_CLK_FREQ_HZ"

}
