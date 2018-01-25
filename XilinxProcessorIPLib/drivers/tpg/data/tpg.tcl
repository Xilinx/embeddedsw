##############################################################################
#
# Copyright (C) 2001 - 2014 Xilinx, Inc.  All rights reserved.
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
##############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ---------------------------------------------------
# 2.0      adk    12/10/13 Updated as per the New Tcl API's
# 3.1      ms     05/22/17 Updated the parameter naming from
#                          XPAR_TPG_NUM_INSTANCES to XPAR_XTPG_NUM_INSTANCES
#                          to avoid  compilation failure as the tools
#                          are generating XPAR_XTPG_NUM_INSTANCES in the
#                          xtpg_g.c for fixing MISRA-C files.
###############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XTPG" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT"  "C_S_AXI_CLK_FREQ_HZ" "C_ACTIVE_ROWS" "C_ACTIVE_COLS" "C_PATTERN_CONTROL" "C_MOTION_SPEED" "C_CROSS_HAIRS" "C_ZPLATE_HOR_CONTROL" "C_ZPLATE_VER_CONTROL" "C_BOX_SIZE" "C_BOX_COLOR" "C_STUCK_PIXEL_THRESH" "C_NOISE_GAIN" "C_BAYER_PHASE" "C_HAS_INTC_IF" "C_ENABLE_MOTION"

    ::hsi::utils::define_config_file $drv_handle "xtpg_g.c" "XTpg" "DEVICE_ID" "C_BASEADDR"  "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_ACTIVE_ROWS" "C_ACTIVE_COLS" "C_PATTERN_CONTROL" "C_MOTION_SPEED" "C_CROSS_HAIRS" "C_ZPLATE_HOR_CONTROL" "C_ZPLATE_VER_CONTROL" "C_BOX_SIZE" "C_BOX_COLOR" "C_STUCK_PIXEL_THRESH" "C_NOISE_GAIN" "C_BAYER_PHASE" "C_HAS_INTC_IF" "C_ENABLE_MOTION"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "TPG" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_ACTIVE_ROWS" "C_ACTIVE_COLS" "C_PATTERN_CONTROL" "C_MOTION_SPEED" "C_CROSS_HAIRS" "C_ZPLATE_HOR_CONTROL" "C_ZPLATE_VER_CONTROL" "C_BOX_SIZE" "C_BOX_COLOR" "C_STUCK_PIXEL_THRESH" "C_NOISE_GAIN" "C_BAYER_PHASE" "C_HAS_INTC_IF" "C_ENABLE_MOTION"
}
