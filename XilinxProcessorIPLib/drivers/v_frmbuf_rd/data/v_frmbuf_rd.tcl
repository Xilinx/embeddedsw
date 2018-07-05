##############################################################################
#
# Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
###############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_frmbufrd" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "C_S_AXI_CTRL_HIGHADDR" "SAMPLES_PER_CLOCK" "MAX_COLS" "MAX_ROWS" "MAX_DATA_WIDTH" "AXIMM_DATA_WIDTH" "AXIMM_ADDR_WIDTH" "HAS_RGBX8" "HAS_YUVX8" "HAS_YUYV8" "HAS_RGBA8" "HAS_YUVA8" "HAS_RGBX10" "HAS_YUVX10" "HAS_Y_UV8" "HAS_Y_UV8_420" "HAS_RGB8" "HAS_YUV8" "HAS_Y_UV10" "HAS_Y_UV10_420" "HAS_Y8" "HAS_Y10" "HAS_BGRA8" "HAS_BGRX8" "HAS_UYVY8"  "HAS_BGR8" "HAS_INTERLACED"

    xdefine_config_file $drv_handle "xv_frmbufrd_g.c" "XV_frmbufrd" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "SAMPLES_PER_CLOCK" "MAX_COLS" "MAX_ROWS" "MAX_DATA_WIDTH" "AXIMM_DATA_WIDTH" "AXIMM_ADDR_WIDTH" "HAS_RGBX8" "HAS_YUVX8" "HAS_YUYV8" "HAS_RGBA8" "HAS_YUVA8" "HAS_RGBX10" "HAS_YUVX10" "HAS_Y_UV8" "HAS_Y_UV8_420" "HAS_RGB8" "HAS_YUV8" "HAS_Y_UV10" "HAS_Y_UV10_420" "HAS_Y8" "HAS_Y10" "HAS_BGRA8" "HAS_BGRX8" "HAS_UYVY8" "HAS_BGR8" "HAS_INTERLACED"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_frmbufrd" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "C_S_AXI_CTRL_HIGHADDR" "SAMPLES_PER_CLOCK" "MAX_COLS" "MAX_ROWS" "MAX_DATA_WIDTH" "AXIMM_DATA_WIDTH" "AXIMM_ADDR_WIDTH" "HAS_RGBX8" "HAS_YUVX8" "HAS_YUYV8" "HAS_RGBA8" "HAS_YUVA8" "HAS_RGBX10" "HAS_YUVX10" "HAS_Y_UV8" "HAS_Y_UV8_420" "HAS_RGB8" "HAS_YUV8" "HAS_Y_UV10" "HAS_Y_UV10_420" "HAS_Y8" "HAS_Y10" "HAS_BGRA8" "HAS_BGRX8" "HAS_UYVY8"  "HAS_BGR8" "HAS_INTERLACED"
}
