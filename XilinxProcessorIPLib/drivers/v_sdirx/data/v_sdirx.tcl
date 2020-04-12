################################################################################
# Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
################################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_SdiRx" \
    "NUM_INSTANCES" \
    "DEVICE_ID" \
    "C_S_AXI_CTRL_BASEADDR" \
    "C_S_AXI_CTRL_HIGHADDR" \
    "C_INCLUDE_EDH" \
    "C_LINE_RATE"

    xdefine_config_file $drv_handle "xv_sdirx_g.c" \
    "XV_SdiRx" \
    "DEVICE_ID" \
    "C_S_AXI_CTRL_BASEADDR" \
    "C_INCLUDE_EDH" \
    "C_LINE_RATE"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_SdiRx" \
    "NUM_INSTANCES" \
    "DEVICE_ID" \
    "C_S_AXI_CTRL_BASEADDR" \
    "C_S_AXI_CTRL_HIGHADDR" \
    "C_INCLUDE_EDH" \
    "C_LINE_RATE"

set orig_dir [pwd]
cd ../../include/

set timestamp [clock format [clock seconds] -format {%Y%m%d%H%M%S}]
set filename "xparameters.h"
set temp $filename.new.$timestamp
set backup $filename.bak.$timestamp

set in [open $filename r]
set out [open $temp w]

# line-by-line, read the original file
while {[gets $in line] != -1} {
	# if XPAR_.*V_SMPTE_UHDSDI_RX is present in the string
	if { [regexp -nocase {XPAR_.*V_SMPTE_UHDSDI_RX} $line] ||
		[regexp -nocase {XPAR_.*SDIRX} $line] } {
		# if substring LINE_RATE is present in the string
		if { [regexp -nocase {LINE_RATE } $line] } {
			# using string map to replace strings with numbers
			set line [string map {3G_SDI 0 6G_SDI 1 12G_SDI_8DS 2 12G_SDI_16DS 3} $line]
			set line [string map {3GSDI 0 6GSDI 1 12GSDI8DS 2 12GSDI16DS 3} $line]
		}
		if { [regexp -nocase {INCLUDE_EDH } $line] } {
		}
	}

		# then write the transformed line
		puts $out $line
	}
close $in
close $out

# move the new data to the proper filename
file delete $filename
file rename -force $temp $filename
cd $orig_dir

}
