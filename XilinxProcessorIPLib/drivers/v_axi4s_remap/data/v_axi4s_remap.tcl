###############################################################################
#
# Copyright (C) 2015 - 2018 Xilinx, Inc.  All rights reserved.
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
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#
#
###############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_axi4s_remap" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "C_S_AXI_CTRL_HIGHADDR" "NUM_VIDEO_COMPONENTS" "MAX_COLS" "MAX_ROWS" 	"IN_SAMPLES_PER_CLOCK" "OUT_SAMPLES_PER_CLOCK" "CONVERT_SAMPLES_PER_CLOCK" "IN_MAX_DATA_WIDTH" "OUT_MAX_DATA_WIDTH" "IN_HDMI_420" "OUT_HDMI_420" "IN_SAMPLE_DROP" "OUT_SAMPLE_REPEAT"

    xdefine_config_file $drv_handle "xv_axi4s_remap_g.c" "XV_axi4s_remap" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "NUM_VIDEO_COMPONENTS" "MAX_COLS" "MAX_ROWS" "IN_SAMPLES_PER_CLOCK" "OUT_SAMPLES_PER_CLOCK" "CONVERT_SAMPLES_PER_CLOCK" "IN_MAX_DATA_WIDTH" "OUT_MAX_DATA_WIDTH" "IN_HDMI_420" "OUT_HDMI_420" "IN_SAMPLE_DROP" "OUT_SAMPLE_REPEAT"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_axi4s_remap" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "C_S_AXI_CTRL_HIGHADDR" "NUM_VIDEO_COMPONENTS" "MAX_COLS" "MAX_ROWS" 	"IN_SAMPLES_PER_CLOCK" "OUT_SAMPLES_PER_CLOCK" "CONVERT_SAMPLES_PER_CLOCK" "IN_MAX_DATA_WIDTH" "OUT_MAX_DATA_WIDTH" "IN_HDMI_420" "OUT_HDMI_420" "IN_SAMPLE_DROP" "OUT_SAMPLE_REPEAT"
}
