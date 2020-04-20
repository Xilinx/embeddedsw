##############################################################################
# Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XDualSplitter" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_MAX_SEGMENTS" "C_AXIS_VIDEO_MAX_TDATA_WIDTH" "C_AXIS_VIDEO_MAX_ITDATASMPLS_PER_CLK" "C_AXIS_VIDEO_MAX_OTDATASMPLS_PER_CLK" "C_MAX_OVRLAP" "C_MAX_SMPL_WIDTH" "C_HAS_AXI4_LITE" "C_HAS_IRQ"
    ::hsi::utils::define_config_file $drv_handle "xdualsplitter_g.c" "XDualSplitter" "DEVICE_ID" "C_BASEADDR" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_MAX_SEGMENTS" "C_AXIS_VIDEO_MAX_TDATA_WIDTH" "C_AXIS_VIDEO_MAX_ITDATASMPLS_PER_CLK" "C_AXIS_VIDEO_MAX_OTDATASMPLS_PER_CLK" "C_MAX_OVRLAP" "C_MAX_SMPL_WIDTH" "C_HAS_AXI4_LITE" "C_HAS_IRQ"
    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "DualSplitter" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_MAX_SEGMENTS" "C_AXIS_VIDEO_MAX_TDATA_WIDTH" "C_AXIS_VIDEO_MAX_ITDATASMPLS_PER_CLK" "C_AXIS_VIDEO_MAX_OTDATASMPLS_PER_CLK" "C_MAX_OVRLAP" "C_MAX_SMPL_WIDTH" "C_HAS_AXI4_LITE" "C_HAS_IRQ"
}
