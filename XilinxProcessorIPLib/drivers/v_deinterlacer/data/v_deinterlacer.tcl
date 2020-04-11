##############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_deinterlacer" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_DATA_WIDTH"

    xdefine_config_file $drv_handle "xv_deinterlacer_g.c" "XV_deinterlacer" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_DATA_WIDTH"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_deinterlacer" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "NUM_VIDEO_COMPONENTS" \
        "MAX_DATA_WIDTH"
}

