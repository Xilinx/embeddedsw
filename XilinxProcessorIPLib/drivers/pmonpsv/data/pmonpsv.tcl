###############################################################################
# Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 1.0     sd    01/20/19   Initial release
# 2.0     sd    04/22/20   Rename the APIs
# 2.3     ht    06/27/23   Added message regarding deprecation of Pmonpsv
###############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
	puts "WARNING: Pmonpsv driver is being deprecated from 2024.1 release. It will be made obsolete in 2025.1 release."
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XPmonPsv" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR"
    ::hsi::utils::define_config_file $drv_handle "xpmonpsv_g.c" "XPmonPsv" "DEVICE_ID" "C_S_AXI_BASEADDR"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "XPmonPsv" "DEVICE_ID" "C_S_AXI_BASEADDR"


}
