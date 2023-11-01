##############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
# Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_vcresampler" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH" \
        "CONVERT_TYPE" \
        "NUM_V_TAPS"

    xdefine_config_file $drv_handle "xv_vcresampler_g.c" "XV_vcresampler" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "SAMPLES_PER_CLOCK" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH" \
        "CONVERT_TYPE" \
        "NUM_V_TAPS"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_vcresampler" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH" \
        "CONVERT_TYPE" \
        "NUM_V_TAPS"
}

