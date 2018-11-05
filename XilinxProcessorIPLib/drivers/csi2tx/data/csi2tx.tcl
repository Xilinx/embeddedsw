###############################################################################
#
# Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
# MODIFICATION HISTORY:
# Ver Who Date     Changes
# --- --- -------- -----------------------------------------------------------
# 1.0 sss 07/21/16 Initial version
# 1.1 vsa 02/28/18 Added Frame Generation feature
##############################################################################

#uses "xillib.tcl"

set periph_ninstances    0

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XCsi2Tx" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_CSI_LANES" "C_CSI_EN_ACTIVELANES" "C_EN_REG_BASED_FE_GEN"
  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Csi2Tx" "DEVICE_ID" "C_BASEADDR" "C_CSI_LANES" "C_CSI_EN_ACTIVELANES" "C_EN_REG_BASED_FE_GEN"
  ::hsi::utils::define_config_file  $drv_handle "xcsi2tx_g.c" "XCsi2Tx" "DEVICE_ID" "C_BASEADDR" "C_CSI_LANES" "C_CSI_EN_ACTIVELANES" "C_EN_REG_BASED_FE_GEN"
}
