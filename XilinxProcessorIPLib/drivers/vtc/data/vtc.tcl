##############################################################################
# Copyright (C) 2008 - 2020 Xilinx, Inc.  All rights reserved.
# Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XVTC" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_GENERATE_EN" "C_DETECT_EN" "C_DET_HSYNC_EN" "C_DET_VSYNC_EN" "C_DET_HBLANK_EN" "C_DET_VBLANK_EN" "C_DET_AVIDEO_EN" "C_DET_ACHROMA_EN"
    ::hsi::utils::define_config_file $drv_handle "xvtc_g.c" "XVtc" "DEVICE_ID" "C_BASEADDR"
    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "VTC" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_GENERATE_EN" "C_DETECT_EN" "C_DET_HSYNC_EN" "C_DET_VSYNC_EN" "C_DET_HBLANK_EN" "C_DET_VBLANK_EN" "C_DET_AVIDEO_EN" "C_DET_ACHROMA_EN"
}
