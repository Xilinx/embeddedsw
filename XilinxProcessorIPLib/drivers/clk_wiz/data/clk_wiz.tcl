###############################################################################
#
# Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
# MODIFICATION HISTORY:
# Ver Who Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 1.0 ram 02/15/16 Initial version for Clock Wizard
# 1.1 siv 08/17/16 Added support for Zynq MPSoC and 64-bit
##############################################################################

#uses "xillib.tcl"

set periph_ninstances	0

proc generate {drv_handle} {

  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XClk_Wiz" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_ENABLE_CLOCK_MONITOR" "C_ENABLE_USER_CLOCK0" "C_ENABLE_USER_CLOCK1" "C_ENABLE_USER_CLOCK2" "C_ENABLE_USER_CLOCK3" "C_REF_CLK_FREQ" "C_USER_CLK_FREQ0" "C_USER_CLK_FREQ1" "C_USER_CLK_FREQ2" "C_USER_CLK_FREQ3" "C_PRECISION" "C_Enable_PLL0" "C_Enable_PLL1"

  ::hsi::utils::define_config_file  $drv_handle "xclk_wiz_g.c" "XClk_Wiz" "DEVICE_ID" "C_BASEADDR" "C_ENABLE_CLOCK_MONITOR" "C_ENABLE_USER_CLOCK0" "C_ENABLE_USER_CLOCK1" "C_ENABLE_USER_CLOCK2" "C_ENABLE_USER_CLOCK3" "C_REF_CLK_FREQ" "C_USER_CLK_FREQ0" "C_USER_CLK_FREQ1" "C_USER_CLK_FREQ2" "C_USER_CLK_FREQ3" "C_PRECISION" "C_Enable_PLL0" "C_Enable_PLL1"

  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Clk_Wiz" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_ENABLE_CLOCK_MONITOR" "C_ENABLE_USER_CLOCK0" "C_ENABLE_USER_CLOCK1" "C_ENABLE_USER_CLOCK2" "C_ENABLE_USER_CLOCK3" "C_REF_CLK_FREQ" "C_USER_CLK_FREQ0" "C_USER_CLK_FREQ1" "C_USER_CLK_FREQ2" "C_USER_CLK_FREQ3" "C_PRECISION" "C_Enable_PLL0" "C_Enable_PLL1"
}
