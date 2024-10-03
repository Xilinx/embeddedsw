###############################################################################
# Copyright (c) 2017 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  kc  11/16/17 Initial Release
#       kal 09/25/24 Remove deleting folders which are set in secure_drc
#
##############################################################################

#---------------------------------------------
# pdi_drc
#---------------------------------------------
proc pdi_drc {libhandle} {
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set versal_net "src/versal_net/"
	set versal "src/versal/"
	set common "src/common/"

	foreach entry [glob -nocomplain -types f [file join ./src/ *]] {
		if {$entry != "./src/Makefile"} {
			file delete -force $entry
		}
	}

	foreach entry [glob -nocomplain -types f [file join $common *]] {
		file copy -force $entry "./src"
	}
	if {$proc_type == "psxl_pmc" || $proc_type == "psx_pmc"} {
		foreach entry [glob -nocomplain -types f [file join $versal_net *]] {
			file copy -force $entry "./src"
		}
	}

	if {$proc_type == "psv_pmc"} {
		foreach entry [glob -nocomplain -types f [file join $versal *]] {
			file copy -force $entry "./src"
		}
	}

}

proc generate {libhandle} {

}

#-------
# post_generate: called after generate called on all libraries
#-------
proc post_generate {libhandle} {

	xgen_opts_file $libhandle
}

#-------
# execs_generate: called after BSP's, libraries and drivers have been compiled
#-------
proc execs_generate {libhandle} {

}

proc xgen_opts_file {libhandle} {

	# Copy the include files to the include directory
	set srcdir src
	set dstdir [file join .. .. include]

	# Create dstdir if it does not exist
	if { ! [file exists $dstdir] } {
		file mkdir $dstdir
	}

	# Get list of files in the srcdir
	set sources [glob -join $srcdir *.h]

	# Copy each of the files in the list to dstdir
	foreach source $sources {
		file copy -force $source $dstdir
	}
}
