##############################################################################
# Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################
###############################################################################
#
# Modification History
#
#  Ver   Who     Date       Changes
# ----- -----  ----------  -----------------------------------------------
#  6.0   adk    03/06/14    FirstRelease
#  6.1   ms     01/16/17    Updated the parameter naming from
#                           XPAR_CCM_NUM_INSTANCES to XPAR_XCCM_NUM_INSTANCES
#                           to avoid  compilation failure for
#                           XPAR_CCM_NUM_INSTANCES as the tools are generating
#                           XPAR_XCCM_NUM_INSTANCES in the generated xccm_g.c
#                           for fixing MISRA-C files. This is a fix for
#                           CR-966099 based on the update in the tools.
#
##############################################################################

proc generate {drv_handle} {
    
     ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XCCM" "C_BASEADDR" "NUM_INSTANCES" "DEVICE_ID" "C_HIGHADDR" "C_S_AXIS_VIDEO_DATA_WIDTH" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_DATA_WIDTH" "C_M_AXIS_VIDEO_FORMAT" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_CLIP" "C_CLAMP"  "C_K11" "C_K12" "C_K13" "C_K21" "C_K22" "C_K23" "C_K31" "C_K32" "C_K33" "C_ROFFSET" "C_GOFFSET" "C_BOFFSET" "C_MAX_COLS" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_S_AXI_CLK_FREQ_HZ"

    ::hsi::utils::define_config_file $drv_handle "xccm_g.c" "XCcm" "DEVICE_ID" "C_BASEADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_CLIP" "C_CLAMP"  "C_K11" "C_K12" "C_K13" "C_K21" "C_K22" "C_K23" "C_K31" "C_K32" "C_K33" "C_ROFFSET" "C_GOFFSET" "C_BOFFSET" "C_S_AXI_CLK_FREQ_HZ"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "CCM" "C_BASEADDR" "NUM_INSTANCES" "DEVICE_ID" "C_HIGHADDR" "C_S_AXIS_VIDEO_DATA_WIDTH" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_DATA_WIDTH" "C_M_AXIS_VIDEO_FORMAT" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_CLIP" "C_CLAMP"  "C_K11" "C_K12" "C_K13" "C_K21" "C_K22" "C_K23" "C_K31" "C_K32" "C_K33" "C_ROFFSET" "C_GOFFSET" "C_BOFFSET" "C_MAX_COLS" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_S_AXI_CLK_FREQ_HZ"
}
