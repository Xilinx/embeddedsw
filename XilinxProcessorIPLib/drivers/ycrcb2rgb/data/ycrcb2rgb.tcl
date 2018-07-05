##############################################################################
# Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################
###############################################################################
#
# Modification History
#
#  Ver   Who      Date       Changes
# ----- -----  ----------  -----------------------------------------------
#  6.0   adk    19/12/13    First release.
#  7.1   ms     01/31/17    Updated the parameter naming from
#                           XPAR_YCRCB2RGB_NUM_INSTANCES to
#                           XPAR_XYCRCB2RGB_NUM_INSTANCES to avoid compilation
#                           failure for XPAR_YCRCB2RGB_NUM_INSTANCES
#                           as the tools are generating
#                           XPAR_XYCRCB2RGB_NUM_INSTANCES in the generated
#                           xycrcb2rgb_g.c for fixing MISRA-C files. This is a
#                           fix for CR-967548 based on the update in the tools.
#
##############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XYCRCB2RGB" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_DATA_WIDTH" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_MWIDTH" "C_COEF_RANGE" "C_ACOEF" "C_BCOEF" "C_CCOEF" "C_DCOEF" "C_ROFFSET" "C_GOFFSET" "C_BOFFSET" "C_HAS_CLIP" "C_HAS_CLAMP" "C_RGBMAX" "C_RGBMIN" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_S_AXI_CLK_FREQ_HZ" "C_STANDARD_SEL" "C_OUTPUT_RANGE"
    ::hsi::utils::define_config_file $drv_handle "xycrcb2rgb_g.c" "XYCrCb2Rgb" "DEVICE_ID" "C_BASEADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_MWIDTH" "C_COEF_RANGE" "C_ACOEF" "C_BCOEF" "C_CCOEF" "C_DCOEF" "C_ROFFSET" "C_GOFFSET" "C_BOFFSET" "C_HAS_CLIP" "C_HAS_CLAMP" "C_RGBMAX" "C_RGBMIN" "C_S_AXI_CLK_FREQ_HZ" "C_STANDARD_SEL" "C_OUTPUT_RANGE"


    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "YCRCB2RGB" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_DATA_WIDTH" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_MWIDTH" "C_COEF_RANGE" "C_ACOEF" "C_BCOEF" "C_CCOEF" "C_DCOEF" "C_ROFFSET" "C_GOFFSET" "C_BOFFSET" "C_HAS_CLIP" "C_HAS_CLAMP" "C_RGBMAX" "C_RGBMIN" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_S_AXI_CLK_FREQ_HZ" "C_STANDARD_SEL" "C_OUTPUT_RANGE"
}
