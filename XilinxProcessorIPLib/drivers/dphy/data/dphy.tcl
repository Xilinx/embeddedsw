###############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
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
