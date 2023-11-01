##############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
# Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
#  Ver      Who    Date       Changes
# -------- ------ -------- ----------------------------------------------------
#  1.0      rco    07/21/15 Initial version of vprocss csc subcore tcl
#  2.0      dmc    12/17/15 Include new args ENABLE_422 and ENABLE_WINDOW
#  2.2      vyc    10/04/17 Include new arg ENABLE_420
#
###############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_csc" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "V_CSC_MAX_WIDTH" \
        "V_CSC_MAX_HEIGHT" \
        "MAX_DATA_WIDTH" \
        "ENABLE_422" \
        "ENABLE_420" \
        "ENABLE_WINDOW"

    xdefine_config_file $drv_handle "xv_csc_g.c" "XV_csc" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "SAMPLES_PER_CLOCK" \
        "V_CSC_MAX_WIDTH" \
        "V_CSC_MAX_HEIGHT" \
        "MAX_DATA_WIDTH" \
        "ENABLE_422" \
        "ENABLE_420" \
        "ENABLE_WINDOW"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_csc" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "V_CSC_MAX_WIDTH" \
        "V_CSC_MAX_HEIGHT" \
        "MAX_DATA_WIDTH" \
        "ENABLE_422" \
        "ENABLE_420" \
        "ENABLE_WINDOW"
}
