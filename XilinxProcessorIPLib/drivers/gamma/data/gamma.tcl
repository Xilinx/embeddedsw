###############################################################################
# Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "GAMMA" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_DATA_WIDTH" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_DATA_WIDTH" "C_M_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_INTPOL" "C_LUTS" "C_DBL_BUF"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "GAMMA" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_DATA_WIDTH" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_DATA_WIDTH" "C_M_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_INTPOL" "C_LUTS" "C_DBL_BUF"
}
