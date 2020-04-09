##############################################################################
# Copyright (C) 2001 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
##############################################################################
###############################################################################
#
# Modification History
#
#  Ver   Who      Date       Changes
# ----- -----  ----------  -----------------------------------------------
#  7.0   adk    01/07/14    FirstRelease
#  7.1   ms     01/16/17    Updated the parameter naming from
#                           XPAR_CFA_NUM_INSTANCES to XPAR_XCFA_NUM_INSTANCES
#                           to avoid  compilation failure for
#                           XPAR_CFA_NUM_INSTANCES as the tools are generating
#                           XPAR_XCFA_NUM_INSTANCES in the generated xcfa_g.c
#                           for fixing MISRA-C files. This is a fix for
#                           CR-966099 based on the update in the tools.
#
##############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XCFA" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_BAYER_PHASE" "C_ACTIVE_ROWS" "C_ACTIVE_COLS" "C_MAX_COLS" "C_HAS_INTC_IF" "C_HAS_DEBUG" "C_HOR_FILT" "C_FRINGE_TOL"
    ::hsi::utils::define_config_file $drv_handle "xcfa_g.c" "XCfa" "DEVICE_ID" "C_BASEADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_BAYER_PHASE" "C_ACTIVE_ROWS" "C_ACTIVE_COLS" "C_MAX_COLS" "C_HAS_INTC_IF" "C_HAS_DEBUG" "C_HOR_FILT" "C_FRINGE_TOL"
    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "CFA" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_BAYER_PHASE" "C_ACTIVE_ROWS" "C_ACTIVE_COLS" "C_MAX_COLS" "C_HAS_INTC_IF" "C_HAS_DEBUG" "C_HOR_FILT" "C_FRINGE_TOL"
}
