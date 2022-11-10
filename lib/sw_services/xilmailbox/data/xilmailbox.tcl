###############################################################################
# Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
##############################################################################
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0  adk   12/02/19 Initial Release
# 1.0  adk   22/03/19 Add versal GIC IP name in interrupt controller check
##############################################################################

#---------------------------------------------
# mailbox_drc
#---------------------------------------------
proc mailbox_drc {libhandle} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]]
        set gic [hsi::get_cells -hier -filter {IP_NAME=="psu_scugic" || IP_NAME=="psu_acpu_gic" || IP_NAME=="psu_rcpu_gic" || IP_NAME=="psv_scugic" || IP_NAME=="psv_acpu_gic" || IP_NAME=="psv_rcpu_gic"}]
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
	set psdir "src/PS/"

	# Create dstdir if it does not exist
	if { ! [file exists $dstdir] } {
		file mkdir $dstdir
	}

	foreach entry [glob -nocomplain -types f [file join $psdir *]] {
            file copy -force $entry "./src"
        }

	# Get list of files in the srcdir
	set sources [glob -join $srcdir *.h]

	# Copy each of the files in the list to dstdir
	foreach source $sources {
		file copy -force $source $dstdir
	}
}
