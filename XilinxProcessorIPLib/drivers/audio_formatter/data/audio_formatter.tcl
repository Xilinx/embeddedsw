###############################################################################
#
# Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   vve  14/08/18 Initial release
#
##############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XAudioFormatter" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_INCLUDE_MM2S" "C_INCLUDE_S2MM" "C_MAX_NUM_CHANNELS_MM2S" "C_MAX_NUM_CHANNELS_S2MM" "C_MM2S_ADDR_WIDTH" "C_MM2S_DATAFORMAT" "C_PACKING_MODE_MM2S" "C_PACKING_MODE_S2MM" "C_S2MM_ADDR_WIDTH" "C_S2MM_DATAFORMAT"

    ::hsi::utils::define_zynq_config_file $drv_handle "xaudioformatter_g.c" "XAudioFormatter" "DEVICE_ID" "C_BASEADDR" "C_INCLUDE_MM2S" "C_INCLUDE_S2MM" "C_MAX_NUM_CHANNELS_MM2S" "C_MAX_NUM_CHANNELS_S2MM" "C_MM2S_ADDR_WIDTH" "C_MM2S_DATAFORMAT" "C_PACKING_MODE_MM2S" "C_PACKING_MODE_S2MM" "C_S2MM_ADDR_WIDTH" "C_S2MM_DATAFORMAT"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XAudioFormatter" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_INCLUDE_MM2S" "C_INCLUDE_S2MM" "C_MAX_NUM_CHANNELS_MM2S" "C_MAX_NUM_CHANNELS_S2MM" "C_MM2S_ADDR_WIDTH" "C_MM2S_DATAFORMAT" "C_PACKING_MODE_MM2S" "C_PACKING_MODE_S2MM" "C_S2MM_ADDR_WIDTH" "C_S2MM_DATAFORMAT"
}
