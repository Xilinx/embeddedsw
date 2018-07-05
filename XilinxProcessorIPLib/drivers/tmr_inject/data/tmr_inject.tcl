###############################################################################
# Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   sa   04/05/17 First release
#
##############################################################################
#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" \
        "XTMR_Inject" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" \
        "C_MASK" "C_MAGIC" "C_CPU_ID" "C_INJECT_LMB_AWIDTH"
    ::hsi::utils::define_config_file  $drv_handle "xtmr_inject_g.c" \
        "XTMR_Inject" "DEVICE_ID" "C_BASEADDR"  "C_MAGIC" "C_CPU_ID" "C_INJECT_LMB_AWIDTH"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" \
        "TMR_Inject" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_MASK" \
        "C_MAGIC" "C_CPU_ID" "C_INJECT_LMB_AWIDTH"
}
