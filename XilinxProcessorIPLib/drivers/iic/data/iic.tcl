###############################################################################
# Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 3.0     adk    12/10/13 Updated as per the New Tcl API's
##############################################################################
## BEGIN_CHANGELOG EDK_M
##  Removed the local ::hsi::utils::define_canonical_xpars API as there is
##  a common API in the tcl of the tools
##
## END_CHANGELOG

## BEGIN_CHANGELOG EDK_LS3
##   Updated to handle the corner cases described in CR #518193 while
##   generating canonical definitions
##
## END_CHANGELOG

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XIic" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_TEN_BIT_ADR" "C_GPO_WIDTH"
    ::hsi::utils::define_config_file $drv_handle "xiic_g.c" "XIic"  "DEVICE_ID" "C_BASEADDR" "C_TEN_BIT_ADR" "C_GPO_WIDTH"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Iic" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_TEN_BIT_ADR" "C_GPO_WIDTH"
}
