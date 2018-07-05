##############################################################################
# Copyright (c) 2011 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################
###############################################################################
#
# Modification History
#
#  Ver   Who      Date       Changes
# ----- ----  -----------  -----------------------------------------------
#  7.0   adk    01/28/14    FirstRelease
#  7.1   ms     01/16/17    Updated the parameter naming from
#                           XPAR_RGB2YCRCB_NUM_INSTANCES  to
#                           XPAR_XRGB2YCRCB_NUM_INSTANCES to avoid compilation
#                           failure for XPAR_RGB2YCRCB_NUM_INSTANCES as the
#                           tools are generating XPAR_XRGB2YCRCB_NUM_INSTANCES
#                           in the generated xrgb2ycrcb_g.c for fixing MISRA-C
#                           files. This is a fix for CR-966099 based on the
#                           update in the tools.
#
##############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XRGB2YCRCB" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_CLIP" "C_HAS_CLAMP" "C_ACOEF" "C_BCOEF" "C_CCOEF" "C_DCOEF" "C_YOFFSET" "C_CBOFFSET" "C_CROFFSET" "C_YMAX" "C_YMIN" "C_CBMAX" "C_CBMIN" "C_CRMAX" "C_CRMIN" "C_S_AXI_CLK_FREQ_HZ" "C_STANDARD_SEL" "C_OUTPUT_RANGE"

    ::hsi::utils::define_config_file $drv_handle "xrgb2ycrcb_g.c" "XRgb2YCrCb" "DEVICE_ID" "C_BASEADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_CLIP" "C_HAS_CLAMP" "C_ACOEF" "C_BCOEF" "C_CCOEF" "C_DCOEF" "C_YOFFSET" "C_CBOFFSET" "C_CROFFSET" "C_YMAX" "C_YMIN" "C_CBMAX" "C_CBMIN" "C_CRMAX" "C_CRMIN" "C_S_AXI_CLK_FREQ_HZ" "C_STANDARD_SEL" "C_OUTPUT_RANGE"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "RGB2YCRCB" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_CLIP" "C_HAS_CLAMP" "C_ACOEF" "C_BCOEF" "C_CCOEF" "C_DCOEF" "C_YOFFSET" "C_CBOFFSET" "C_CROFFSET" "C_YMAX" "C_YMIN" "C_CBMAX" "C_CBMIN" "C_CRMAX" "C_CRMIN" "C_S_AXI_CLK_FREQ_HZ" "C_STANDARD_SEL" "C_OUTPUT_RANGE"
}
