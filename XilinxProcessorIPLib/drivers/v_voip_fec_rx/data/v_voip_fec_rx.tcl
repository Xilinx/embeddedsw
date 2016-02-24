##############################################################################
#
# Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"),to deal
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
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
###############################################################################

proc generate {drv_handle} {
    xdefine_include_file     $drv_handle "xparameters.h"    "XVoipFEC_RX" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_NUM_OF_CHANNELS" "C_MEDIA_BASE_ADDR" "C_MEDIA_BUF_DEPTH_PER_CH" "C_FEC_BASE_ADDR" "C_FEC_POOL_SIZE" "C_INCLUDE_FEC" "C_INCLUDE_SEAMLESS_PROTECT" "C_FIXED_OR_PROGRAMMABLE_ADDR"
    xdefine_config_file      $drv_handle "xvoip_fec_rx_g.c" "XVoipFEC_RX"                 "DEVICE_ID" "C_BASEADDR"              "C_NUM_OF_CHANNELS" "C_MEDIA_BASE_ADDR" "C_MEDIA_BUF_DEPTH_PER_CH" "C_FEC_BASE_ADDR" "C_FEC_POOL_SIZE" "C_INCLUDE_FEC" "C_INCLUDE_SEAMLESS_PROTECT" "C_FIXED_OR_PROGRAMMABLE_ADDR"
    xdefine_canonical_xpars  $drv_handle "xparameters.h"    "XVoipFEC_RX" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_NUM_OF_CHANNELS" "C_MEDIA_BASE_ADDR" "C_MEDIA_BUF_DEPTH_PER_CH" "C_FEC_BASE_ADDR" "C_FEC_POOL_SIZE" "C_INCLUDE_FEC" "C_INCLUDE_SEAMLESS_PROTECT" "C_FIXED_OR_PROGRAMMABLE_ADDR"
}