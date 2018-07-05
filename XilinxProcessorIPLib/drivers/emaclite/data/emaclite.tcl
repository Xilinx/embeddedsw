###############################################################################
#
# Copyright (C) 2004 - 2014 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
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
