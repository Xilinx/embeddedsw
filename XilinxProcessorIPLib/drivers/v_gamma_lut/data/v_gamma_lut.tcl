##############################################################################
# Copyright (C) 1986-2021 Xilinx, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_gamma_lut" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH"

    xdefine_config_file $drv_handle "xv_gamma_lut_g.c" "XV_gamma_lut" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "SAMPLES_PER_CLOCK" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_gamma_lut" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH"
}
