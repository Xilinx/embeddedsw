###############################################################################
#
# Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
#
# MODIFICATION HISTORY:
# Ver Who Date     Changes
# --- --- -------- ----------------------------------------------------
# 1.0 vsa 07/03/15 Initial version
# 1.1 vsa 03/02/17 Add support for HS_SETTLE register
##############################################################################


set periph_ninstances    0

proc generate {drv_handle} {
	::hsi::utils::define_include_file $drv_handle "xparameters.h" "XDphy" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_DPHY_MODE" "C_EN_REG_IF" "C_DPHY_LANES" "C_ESC_CLK_PERIOD" "C_ESC_TIMEOUT" "C_HS_LINE_RATE" "C_HS_TIMEOUT" "C_LPX_PERIOD" "C_STABLE_CLK_PERIOD" "C_TXPLL_CLKIN_PERIOD" "C_WAKEUP" "C_EN_TIMEOUT_REGS" "C_HS_SETTLE_NS"
	::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Dphy" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_DPHY_MODE" "C_EN_REG_IF" "C_DPHY_LANES" "C_ESC_CLK_PERIOD" "C_ESC_TIMEOUT" "C_HS_LINE_RATE" "C_HS_TIMEOUT" "C_LPX_PERIOD" "C_STABLE_CLK_PERIOD" "C_TXPLL_CLKIN_PERIOD" "C_WAKEUP" "C_EN_TIMEOUT_REGS" "C_HS_SETTLE_NS"
	::hsi::utils::define_config_file  $drv_handle "xdphy_g.c" "XDphy" "DEVICE_ID" "C_BASEADDR" "C_DPHY_MODE" "C_EN_REG_IF" "C_DPHY_LANES" "C_ESC_CLK_PERIOD" "C_ESC_TIMEOUT" "C_HS_LINE_RATE" "C_HS_TIMEOUT" "C_LPX_PERIOD" "C_STABLE_CLK_PERIOD" "C_TXPLL_CLKIN_PERIOD" "C_WAKEUP" "C_EN_TIMEOUT_REGS" "C_HS_SETTLE_NS"

	set orig_dir [pwd]
	cd ../../include/

	set timestamp [clock format [clock seconds] -format {%Y%m%d%H%M%S}]

	set filename "xparameters.h"
	set temp     $filename.new.$timestamp
	set backup   $filename.bak.$timestamp

	set in  [open $filename r]
	set out [open $temp w]

	while {[gets $in line] != -1} {

		# if _DPHY_MODE is present in the string
		if { [regexp -nocase {_DPHY_MODE} $line] } {
			# replace MASTER with 1 and SLAVE with 0
			set line [string map {MASTER 1 SLAVE 0} $line]

		}

		puts $out $line
	}

	close $in
	close $out
	file delete $filename
	file rename -force $temp $filename
	cd $orig_dir
}
