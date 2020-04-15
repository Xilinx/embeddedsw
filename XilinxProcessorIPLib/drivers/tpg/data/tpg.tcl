##############################################################################
# Copyright (c) 2001 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
##############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ---------------------------------------------------
# 2.0      adk    12/10/13 Updated as per the New Tcl API's
# 3.1      ms     05/22/17 Updated the parameter naming from
#                          XPAR_TPG_NUM_INSTANCES to XPAR_XTPG_NUM_INSTANCES
#                          to avoid  compilation failure as the tools
#                          are generating XPAR_XTPG_NUM_INSTANCES in the
#                          xtpg_g.c for fixing MISRA-C files.
###############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XTPG" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT"  "C_S_AXI_CLK_FREQ_HZ" "C_ACTIVE_ROWS" "C_ACTIVE_COLS" "C_PATTERN_CONTROL" "C_MOTION_SPEED" "C_CROSS_HAIRS" "C_ZPLATE_HOR_CONTROL" "C_ZPLATE_VER_CONTROL" "C_BOX_SIZE" "C_BOX_COLOR" "C_STUCK_PIXEL_THRESH" "C_NOISE_GAIN" "C_BAYER_PHASE" "C_HAS_INTC_IF" "C_ENABLE_MOTION"

    ::hsi::utils::define_config_file $drv_handle "xtpg_g.c" "XTpg" "DEVICE_ID" "C_BASEADDR"  "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_ACTIVE_ROWS" "C_ACTIVE_COLS" "C_PATTERN_CONTROL" "C_MOTION_SPEED" "C_CROSS_HAIRS" "C_ZPLATE_HOR_CONTROL" "C_ZPLATE_VER_CONTROL" "C_BOX_SIZE" "C_BOX_COLOR" "C_STUCK_PIXEL_THRESH" "C_NOISE_GAIN" "C_BAYER_PHASE" "C_HAS_INTC_IF" "C_ENABLE_MOTION"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "TPG" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_ACTIVE_ROWS" "C_ACTIVE_COLS" "C_PATTERN_CONTROL" "C_MOTION_SPEED" "C_CROSS_HAIRS" "C_ZPLATE_HOR_CONTROL" "C_ZPLATE_VER_CONTROL" "C_BOX_SIZE" "C_BOX_COLOR" "C_STUCK_PIXEL_THRESH" "C_NOISE_GAIN" "C_BAYER_PHASE" "C_HAS_INTC_IF" "C_ENABLE_MOTION"
}
