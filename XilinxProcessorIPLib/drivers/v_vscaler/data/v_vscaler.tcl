##############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_vscaler" \
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
        "ENABLE_420"

    xdefine_config_file $drv_handle "xv_vscaler_g.c" "XV_vscaler" \
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
        "ENABLE_420"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_vscaler" \
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
        "ENABLE_420"
}

