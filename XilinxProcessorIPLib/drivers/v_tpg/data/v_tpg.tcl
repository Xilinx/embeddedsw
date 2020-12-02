##############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_tpg" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "HAS_AXI4S_SLAVE" \
        "SAMPLES_PER_CLOCK" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH" \
        "SOLID_COLOR" \
        "RAMP" \
        "COLOR_BAR" \
        "DISPLAY_PORT" \
        "COLOR_SWEEP" \
        "ZONE_PLATE" \
        "FOREGROUND"

    xdefine_config_file $drv_handle "xv_tpg_g.c" "XV_tpg" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "HAS_AXI4S_SLAVE" \
        "SAMPLES_PER_CLOCK" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH" \
        "SOLID_COLOR" \
        "RAMP" \
        "COLOR_BAR" \
        "DISPLAY_PORT" \
        "COLOR_SWEEP" \
        "ZONE_PLATE" \
        "FOREGROUND"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_tpg" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "HAS_AXI4S_SLAVE" \
        "SAMPLES_PER_CLOCK" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_COLS" \
        "MAX_ROWS" \
        "MAX_DATA_WIDTH" \
        "SOLID_COLOR" \
        "RAMP" \
        "COLOR_BAR" \
        "DISPLAY_PORT" \
        "COLOR_SWEEP" \
        "ZONE_PLATE" \
        "FOREGROUND"
}
