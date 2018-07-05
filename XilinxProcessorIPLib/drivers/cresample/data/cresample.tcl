###############################################################################
# Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
###############################################################################
#
# Modification History
#
#  Ver   Who      Date       Changes
# ----- -----  ----------  -----------------------------------------------
#  4.0   adk    03/12/14    FirstRelease
#  4.1   ms     01/16/17    Updated the parameter naming from
#                           XPAR_CRESAMPLE_NUM_INSTANCES to
#                           XPAR_XCRESAMPLE_NUM_INSTANCES to avoid compilation
#                           failure for XPAR_CRESAMPLE_NUM_INSTANCES as the
#                           tools are generating XPAR_XCRESAMPLE_NUM_INSTANCES
#                           in the generated xcresample_g.c for fixing MISRA-C
#                           files. This is a fix for CR-966099 based on the
#                           update in the tools.
#
##############################################################################

proc generate {drv_handle} {
 
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XCRESAMPLE" "NUM_INSTANCES" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID" "C_S_AXIS_VIDEO_DATA_WIDTH" "C_S_AXIS_VIDEO_FORMAT" "C_S_AXIS_VIDEO_TDATA_WIDTH" "C_M_AXIS_VIDEO_DATA_WIDTH" "C_M_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_TDATA_WIDTH" "C_HAS_AXI4_LITE" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_CHROMA_PARITY" "C_FIELD_PARITY" "C_INTERLACED" "C_NUM_H_TAPS" "C_NUM_V_TAPS" "C_CONVERT_TYPE" "C_COEF_WIDTH" "C_S_AXI_CLK_FREQ_HZ"

    ::hsi::utils::define_config_file $drv_handle "xcresample_g.c" "XCresample" "DEVICE_ID" "C_BASEADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT"  "C_S_AXI_CLK_FREQ_HZ" "C_HAS_INTC_IF" "C_HAS_DEBUG" "C_MAX_COLS" "C_ACTIVE_ROWS" "C_ACTIVE_COLS"  "C_CHROMA_PARITY" "C_FIELD_PARITY" "C_INTERLACED" "C_NUM_H_TAPS" "C_NUM_V_TAPS" "C_CONVERT_TYPE" "C_COEF_WIDTH"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "CRESAMPLE" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_DATA_WIDTH" "C_S_AXIS_VIDEO_FORMAT" "C_S_AXIS_VIDEO_TDATA_WIDTH" "C_M_AXIS_VIDEO_DATA_WIDTH" "C_M_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_TDATA_WIDTH" "C_HAS_AXI4_LITE" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_CHROMA_PARITY" "C_FIELD_PARITY" "C_INTERLACED" "C_NUM_H_TAPS" "C_NUM_V_TAPS" "C_CONVERT_TYPE" "C_COEF_WIDTH" "C_S_AXI_CLK_FREQ_HZ"

}

