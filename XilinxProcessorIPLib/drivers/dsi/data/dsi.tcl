###############################################################################
# Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################
#
# MODIFICATION HISTORY:
# Ver Who Date     Changes
# --- --- ------- -------------------------------------------------------
# 1.0 ram 11/2/16 Initial Release for MIPI DSI TX subsystem
##############################################################################

#uses "xillib.tcl"

set periph_ninstances 0

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XDsi" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "DSI_LANES" "DSI_DATATYPE" "DSI_BYTE_FIFO" "DSI_CRC_GEN" "DSI_PIXELS"
  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Dsi" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "DSI_LANES" "DSI_DATATYPE" "DSI_BYTE_FIFO" "DSI_CRC_GEN" "DSI_PIXELS"
  ::hsi::utils::define_config_file  $drv_handle "xdsi_g.c" "XDsi" "DEVICE_ID" "C_BASEADDR" "DSI_LANES" "DSI_DATATYPE" "DSI_BYTE_FIFO" "DSI_CRC_GEN" "DSI_PIXELS"

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
	# if XPAR_MIPI_DSI_TX_SUBSYSTEM is present in the string
	if { [regexp -nocase {XPAR_MIPI_DSI_TX_SUBSYSTEM} $line] ||
		[regexp -nocase {XPAR_DSI} $line] } {
		# if substring DSI_CRC_GEN is present in the string
		if { [regexp -nocase {DSI_CRC_GEN} $line] } {
			# using string map to replace true with 1 and false with 0
			set line [string map {true 1 false 0} $line]
		}
		# if substring DSI_CRC_GEN is present in the string
		if { [regexp -nocase {DSI_CRC_GEN } $line] } {
			# using string map to replace true with 1 and false with 0
			set line [string map {RGB888 0x3E RGB565 0x0E RGB666_L 0x2E RGB666_P 0x1E} $line]
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
