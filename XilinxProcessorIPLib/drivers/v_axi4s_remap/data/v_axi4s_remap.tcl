###############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
# Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
###############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_axi4s_remap" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "C_S_AXI_CTRL_HIGHADDR" "NUM_VIDEO_COMPONENTS" "MAX_COLS" "MAX_ROWS" 	"IN_SAMPLES_PER_CLOCK" "OUT_SAMPLES_PER_CLOCK" "CONVERT_SAMPLES_PER_CLOCK" "IN_MAX_DATA_WIDTH" "OUT_MAX_DATA_WIDTH" "IN_HDMI_420" "OUT_HDMI_420" "IN_SAMPLE_DROP" "OUT_SAMPLE_REPEAT"

    xdefine_config_file $drv_handle "xv_axi4s_remap_g.c" "XV_axi4s_remap" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "NUM_VIDEO_COMPONENTS" "MAX_COLS" "MAX_ROWS" "IN_SAMPLES_PER_CLOCK" "OUT_SAMPLES_PER_CLOCK" "CONVERT_SAMPLES_PER_CLOCK" "IN_MAX_DATA_WIDTH" "OUT_MAX_DATA_WIDTH" "IN_HDMI_420" "OUT_HDMI_420" "IN_SAMPLE_DROP" "OUT_SAMPLE_REPEAT"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_axi4s_remap" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "C_S_AXI_CTRL_HIGHADDR" "NUM_VIDEO_COMPONENTS" "MAX_COLS" "MAX_ROWS" 	"IN_SAMPLES_PER_CLOCK" "OUT_SAMPLES_PER_CLOCK" "CONVERT_SAMPLES_PER_CLOCK" "IN_MAX_DATA_WIDTH" "OUT_MAX_DATA_WIDTH" "IN_HDMI_420" "OUT_HDMI_420" "IN_SAMPLE_DROP" "OUT_SAMPLE_REPEAT"
}
