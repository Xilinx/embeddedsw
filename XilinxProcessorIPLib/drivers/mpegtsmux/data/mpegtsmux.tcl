#******************************************************************************
# Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
##*****************************************************************************/
proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XMpegtsmux" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR"

    xdefine_config_file $drv_handle "xmpegtsmux_g.c" "XMpegtsmux" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XMpegtsmux" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR"
}
