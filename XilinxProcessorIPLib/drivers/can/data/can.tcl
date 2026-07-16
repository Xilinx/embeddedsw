###############################################################################
# Copyright (C) 2004 - 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XCan" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "c_can_rx_dpth" "c_can_tx_dpth" "c_can_num_acf" "ENABLE_ECC"
    ::hsi::utils::define_config_file $drv_handle "xcan_g.c" "XCan" "DEVICE_ID" "C_BASEADDR" "c_can_num_acf" "ENABLE_ECC"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Can" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "c_can_rx_dpth" "c_can_tx_dpth" "c_can_num_acf" "ENABLE_ECC"
}
