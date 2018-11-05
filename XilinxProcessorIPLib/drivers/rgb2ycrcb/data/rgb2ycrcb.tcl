##############################################################################
#
# Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
###############################################################################
###############################################################################
#
# Modification History
#
#  Ver   Who      Date       Changes
# ----- ----  -----------  -----------------------------------------------
#  7.0   adk    01/28/14    FirstRelease
#  7.1   ms     01/16/17    Updated the parameter naming from
#                           XPAR_RGB2YCRCB_NUM_INSTANCES  to
#                           XPAR_XRGB2YCRCB_NUM_INSTANCES to avoid compilation
#                           failure for XPAR_RGB2YCRCB_NUM_INSTANCES as the
#                           tools are generating XPAR_XRGB2YCRCB_NUM_INSTANCES
#                           in the generated xrgb2ycrcb_g.c for fixing MISRA-C
#                           files. This is a fix for CR-966099 based on the
#                           update in the tools.
#
##############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XRGB2YCRCB" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_CLIP" "C_HAS_CLAMP" "C_ACOEF" "C_BCOEF" "C_CCOEF" "C_DCOEF" "C_YOFFSET" "C_CBOFFSET" "C_CROFFSET" "C_YMAX" "C_YMIN" "C_CBMAX" "C_CBMIN" "C_CRMAX" "C_CRMIN" "C_S_AXI_CLK_FREQ_HZ" "C_STANDARD_SEL" "C_OUTPUT_RANGE"

    ::hsi::utils::define_config_file $drv_handle "xrgb2ycrcb_g.c" "XRgb2YCrCb" "DEVICE_ID" "C_BASEADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_CLIP" "C_HAS_CLAMP" "C_ACOEF" "C_BCOEF" "C_CCOEF" "C_DCOEF" "C_YOFFSET" "C_CBOFFSET" "C_CROFFSET" "C_YMAX" "C_YMIN" "C_CBMAX" "C_CBMIN" "C_CRMAX" "C_CRMIN" "C_S_AXI_CLK_FREQ_HZ" "C_STANDARD_SEL" "C_OUTPUT_RANGE"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "RGB2YCRCB" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_HAS_DEBUG" "C_HAS_INTC_IF" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_CLIP" "C_HAS_CLAMP" "C_ACOEF" "C_BCOEF" "C_CCOEF" "C_DCOEF" "C_YOFFSET" "C_CBOFFSET" "C_CROFFSET" "C_YMAX" "C_YMIN" "C_CBMAX" "C_CBMIN" "C_CRMAX" "C_CRMIN" "C_S_AXI_CLK_FREQ_HZ" "C_STANDARD_SEL" "C_OUTPUT_RANGE"
}
