###############################################################################
# Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   vns  06/27/22 Initial Release
# 1.1   am   12/21/22 Changed user configurable parameter names
#
##############################################################################

#---------------------------------------------
# ocp_drc
#---------------------------------------------
proc ocp_drc {libhandle} {
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set compiler [common::get_property CONFIG.compiler $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os_type [hsi::get_os];
	set server "src/server"
	set client "src/client/"
	set common "src/common"

	if { $proc_type != "psxl_pmc" && $proc_type != "psxl_cortexa78" &&
		$proc_type != "psxl_cortexr52" && $proc_type != "psx_pmc" &&
		$proc_type != "psx_cortexa78" && $proc_type != "psx_cortexr52"} {
		error "ERROR: XilOcp library is supported only for Versal Net devices";
		return;
	}

	foreach entry [glob -nocomplain -types f [file join $common *]] {
			file copy -force $entry "./src"
	}

	if {$proc_type == "psxl_pmc" || $proc_type == "psx_pmc"} {
		foreach entry [glob -nocomplain -types f [file join "$server" *]] {
			file copy -force $entry "./src"
		}
	} elseif { $proc_type == "psxl_cortexa78" || $proc_type == "psxl_cortexr52" ||
		$proc_type == "psx_cortexa78" || $proc_type == "psx_cortexr52" } {
		set librarylist [hsi::get_libs -filter "NAME==xilmailbox"];
		if { [llength $librarylist] == 0 } {
			error "This library requires xilmailbox library in the Board Support Package.";
		}
		foreach entry [glob -nocomplain -types f [file join "$client" *]] {
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
