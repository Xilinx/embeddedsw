##############################################################################
#
# Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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
###############################################################################
###############################################################################
#
# Modification History
#
#  Ver   Who      Date       Changes
# ----- -----  ----------  -----------------------------------------------
#  7.0   adk    02/19/14    First release.
#  7.1   ms     01/31/17    Updated the parameter naming from
#                           XPAR_ENHANCE_NUM_INSTANCES to
#                           XPAR_XENHANCE_NUM_INSTANCES to avoid compilation
#                           failure for XPAR_ENHANCE_NUM_INSTANCES
#                           as the tools are generating
#                           XPAR_XENHANCE_NUM_INSTANCES in the generated
#                           xenhance_g.c for fixing MISRA-C files. This is a
#                           fix for CR-967548 based on the update in the tools.
#
##############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XENHANCE" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_HAS_AXI4_LITE" "C_HAS_INTC_IF" "C_HAS_DEBUG" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_NOISE" "C_HAS_ENHANCE" "C_HAS_HALO" "C_HAS_ALIAS" "C_OPT_SIZE" "C_NOISE_THRESHOLD" "C_ENHANCE_STRENGTH" "C_HALO_SUPPRESS"
    ::hsi::utils::define_config_file $drv_handle "xenhance_g.c" "XEnhance" "DEVICE_ID" "C_BASEADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_HAS_INTC_IF" "C_HAS_DEBUG" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_NOISE" "C_HAS_ENHANCE" "C_HAS_HALO" "C_HAS_ALIAS" "C_OPT_SIZE" "C_NOISE_THRESHOLD" "C_ENHANCE_STRENGTH" "C_HALO_SUPPRESS"
    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "ENHANCE" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXIS_VIDEO_FORMAT" "C_M_AXIS_VIDEO_FORMAT" "C_S_AXI_CLK_FREQ_HZ" "C_HAS_AXI4_LITE" "C_HAS_INTC_IF" "C_HAS_DEBUG" "C_MAX_COLS" "C_ACTIVE_COLS" "C_ACTIVE_ROWS" "C_HAS_NOISE" "C_HAS_ENHANCE" "C_HAS_HALO" "C_HAS_ALIAS" "C_OPT_SIZE" "C_NOISE_THRESHOLD" "C_ENHANCE_STRENGTH" "C_HALO_SUPPRESS"
}

