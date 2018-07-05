##############################################################################
#
# Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
# XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
# MODIFICATION HISTORY:
#  Ver      Who    Date       Changes
# -------- ------ -------- ----------------------------------------------------
#  1.0      rco    07/21/15 Initial version of vprocss csc subcore tcl
#  2.0      dmc    12/17/15 Include new args ENABLE_422 and ENABLE_WINDOW
#  2.2      vyc    10/04/17 Include new arg ENABLE_420
#
###############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_csc" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "V_CSC_MAX_WIDTH" \
        "V_CSC_MAX_HEIGHT" \
        "MAX_DATA_WIDTH" \
        "ENABLE_422" \
        "ENABLE_420" \
        "ENABLE_WINDOW"

    xdefine_config_file $drv_handle "xv_csc_g.c" "XV_csc" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "SAMPLES_PER_CLOCK" \
        "V_CSC_MAX_WIDTH" \
        "V_CSC_MAX_HEIGHT" \
        "MAX_DATA_WIDTH" \
        "ENABLE_422" \
        "ENABLE_420" \
        "ENABLE_WINDOW"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_csc" \
        "DEVICE_ID" \
        "C_S_AXI_CTRL_BASEADDR" \
        "C_S_AXI_CTRL_HIGHADDR" \
        "SAMPLES_PER_CLOCK" \
        "V_CSC_MAX_WIDTH" \
        "V_CSC_MAX_HEIGHT" \
        "MAX_DATA_WIDTH" \
        "ENABLE_422" \
        "ENABLE_420" \
        "ENABLE_WINDOW"
}
