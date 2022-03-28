# ==============================================================
# Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
# ==============================================================

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_multi_scaler" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "MAX_DATA_WIDTH" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "PHASE_SHIFT" \
        "SCALE_MODE" \
        "TAPS" \
        "MAX_OUTS"

    xdefine_config_file $drv_handle "xv_multi_scaler_g.c" "XV_multi_scaler" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "SAMPLES_PER_CLOCK" \
        "MAX_DATA_WIDTH" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "PHASE_SHIFT" \
        "SCALE_MODE" \
        "TAPS" \
        "MAX_OUTS"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_multi_scaler" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "MAX_DATA_WIDTH" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "PHASE_SHIFT" \
        "SCALE_MODE" \
        "TAPS" \
        "MAX_OUTS"
}

