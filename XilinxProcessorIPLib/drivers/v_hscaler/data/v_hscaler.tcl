##############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
#  Ver      Who    Date       Changes
# -------- ------ -------- ----------------------------------------------------
#  1.0      rco    07/21/15 Initial version of vprocss hscaler subcore tcl
#  2.0      dmc    12/17/15 Include new arg ENABLE_422
#  3.0      mpe    04/28/16 Include new arg ENABLE_420, and ENABLE_CSC
#
###############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_hscaler" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH" \
        "PHASE_SHIFT" \
        "SCALE_MODE" \
        "TAPS" \
        "ENABLE_422" \
        "ENABLE_420" \
        "ENABLE_CSC"

    xdefine_config_file $drv_handle "xv_hscaler_g.c" "XV_hscaler" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "SAMPLES_PER_CLOCK" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH" \
        "PHASE_SHIFT" \
        "SCALE_MODE" \
        "TAPS" \
        "ENABLE_422" \
        "ENABLE_420" \
        "ENABLE_CSC"


    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_hscaler" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH" \
        "PHASE_SHIFT" \
        "SCALE_MODE" \
        "TAPS" \
        "ENABLE_422" \
        "ENABLE_420" \
        "ENABLE_CSC"
}

