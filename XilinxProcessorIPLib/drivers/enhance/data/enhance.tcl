##############################################################################
# Copyright (C) 2009 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################
###############################################################################
#
# Modification History
#
#  Ver   Who      Date       Changes
# ----- -----  ----------  -----------------------------------------------
#  7.0   adk    02/19/14    First release.
#  7.1   ms     01/31/17    Updated the parameter naming from
#                           XPAR_ENHANCE_NUM_INSTANCES to
#                           XPAR_XENHANCE_NUM_INSTANCES to avoid compilation
#                           failure for XPAR_ENHANCE_NUM_INSTANCES
#                           as the tools are generating
#                           XPAR_XENHANCE_NUM_INSTANCES in the generated
#                           xenhance_g.c for fixing MISRA-C files. This is a
#                           fix for CR-967548 based on the update in the tools.
#
##############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XENHANCE" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_HAS_AXI4_LITE" "C_HAS_INTC_IF" "C_HAS_DEBUG" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_NOISE" "C_HAS_ENHANCE" "C_HAS_HALO" "C_HAS_ALIAS" "C_OPT_SIZE" "C_NOISE_THRESHOLD" "C_ENHANCE_STRENGTH" "C_HALO_SUPPRESS"
    ::hsi::utils::define_config_file $drv_handle "xenhance_g.c" "XEnhance" "DEVICE_ID" "C_BASEADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_HAS_INTC_IF" "C_HAS_DEBUG" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_NOISE" "C_HAS_ENHANCE" "C_HAS_HALO" "C_HAS_ALIAS" "C_OPT_SIZE" "C_NOISE_THRESHOLD" "C_ENHANCE_STRENGTH" "C_HALO_SUPPRESS"
    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "ENHANCE" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_HAS_AXI4_LITE" "C_HAS_INTC_IF" "C_HAS_DEBUG" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_NOISE" "C_HAS_ENHANCE" "C_HAS_HALO" "C_HAS_ALIAS" "C_OPT_SIZE" "C_NOISE_THRESHOLD" "C_ENHANCE_STRENGTH" "C_HALO_SUPPRESS"
}

