################################################################################
# Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
################################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XHDCP" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXI_FREQUENCY" "C_IS_RX" "C_IS_HDMI"
    ::hsi::utils::define_config_file $drv_handle "xhdcp1x_g.c" "XHdcp1x" "DEVICE_ID" "C_BASEADDR" "C_S_AXI_FREQUENCY" "C_IS_RX" "C_IS_HDMI"
    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "XHDCP" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_S_AXI_FREQUENCY" "C_IS_RX" "C_IS_HDMI"
    hier_ip_define_config_file $drv_handle "xhdcp1x_g.c" "XHDCP" "DEVICE_ID" "C_BASEADDR" "C_S_AXI_FREQUENCY" "C_IS_RX" "C_IS_HDMI"
}

proc hier_ip_define_config_file {drv_handle file_name drv_string args} {
	set filename [file join "src" $file_name]
	set fp [open $filename r]
	set filedata [read $fp]
	close $fp

	set data [split $filedata "\n"]

	set newdata "\n"
	foreach line $data {
		set wordlist [regexp -inline -all -- {\S+} $line]
		foreach word $wordlist {
			if { [regexp -nocase {.*(XPAR_XHDCP1X_NUM_INSTANCES)+.*} $word] } {
				set word_updated [string map {"XPAR_XHDCP1X_NUM_INSTANCES" "XPAR_XHDCP_NUM_INSTANCES"} $word]
				append newdata " " $word_updated
			} else {
				append newdata " " $word
			}
		}
		append newdata "\n"
	}

	set fwp [open $filename w]
	puts $fwp $newdata
	close $fwp
}
