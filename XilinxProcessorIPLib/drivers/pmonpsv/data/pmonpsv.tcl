###############################################################################
# Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 1.0     sd    01/20/19   Initial release
# 2.0     sd    04/22/20   Rename the APIs
###############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XPmonPsv" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR"
    ::hsi::utils::define_config_file $drv_handle "xpmonpsv_g.c" "XPmonPsv" "DEVICE_ID" "C_S_AXI_BASEADDR"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "XPmonPsv" "DEVICE_ID" "C_S_AXI_BASEADDR"


}
