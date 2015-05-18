###############################################################################
#
# Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
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
###############################################################################
#
# MODIFICATION HISTORY:
#
# Ver	Who	Date	 Changes
# ----	-----	-------	 -----------------------------
# 1.0	nsk	15/05/15 Updated as per RTL. TxBuffer
#			 Can be configurable(8,16,32).
#
###############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XCanfd" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_CAN_RX_DPTH" "C_CAN_TX_DPTH" "RX_MODE" "NUM_OF_RX_MB_BUF" "NUM_OF_TX_BUF"
    xdefine_config_file $drv_handle "xcanfd_g.c" "XCanFd" "DEVICE_ID" "C_BASEADDR" "RX_MODE" "NUM_OF_RX_MB_BUF" "NUM_OF_TX_BUF"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "Canfd" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_CAN_RX_DPTH" "C_CAN_TX_DPTH" "RX_MODE" "NUM_OF_RX_MB_BUF" "NUM_OF_TX_BUF"
}
