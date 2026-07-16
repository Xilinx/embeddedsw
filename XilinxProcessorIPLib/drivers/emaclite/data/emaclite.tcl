###############################################################################
# Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 4.0   adk  10/12/13 Updated as per the New Tcl API's
#####################################################################

## @BEGIN_CHANGELOG EDK_M
##  Removed the local ::hsi::utils::define_canonical_xpars API as there is
##  a common API in the tcl of the tools
##
## @END_CHANGELOG

## @BEGIN_CHANGELOG EDK_LS3
##   Updated to handle the corner cases described in CR #518193 while
##   generating canonical definitions
##
## @END_CHANGELOG

#uses "xillib.tcl"

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XEmacLite" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_TX_PING_PONG" "C_RX_PING_PONG" "C_INCLUDE_MDIO" "C_INCLUDE_INTERNAL_LOOPBACK"
  ::hsi::utils::define_config_file $drv_handle "xemaclite_g.c" "XEmacLite"  "DEVICE_ID" "C_BASEADDR" "C_TX_PING_PONG" "C_RX_PING_PONG" "C_INCLUDE_MDIO" "C_INCLUDE_INTERNAL_LOOPBACK"

  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "EmacLite" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_TX_PING_PONG" "C_RX_PING_PONG" "C_INCLUDE_MDIO" "C_INCLUDE_INTERNAL_LOOPBACK"
}
