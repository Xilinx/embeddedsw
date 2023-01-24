###############################################################################
# Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
# Copyright (c) 2023, Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   har  01/11/23 Initial Release
#       har  01/20/23 Fixed XSCT build issue
#
##############################################################################

#---------------------------------------------
# cert_drc
#---------------------------------------------
proc cert_drc {libhandle} {
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os_type [hsi::get_os];
	set src "src"

	#For Versal devices PLM BSP contains dummy OCP library
	if { $proc_type == "psv_pmc" } {
		foreach entry [glob -nocomplain -types f [file join "$src" *.c]] {
			file delete -force $entry
		}
		return;
	}

	if {$proc_type != "psxl_pmc" && $proc_type != "psx_pmc"} {
		error "ERROR: XilCert library is supported only for psxl_pmc and psx_pmc processors.";
		return;
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
