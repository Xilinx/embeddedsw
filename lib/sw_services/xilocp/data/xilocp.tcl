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
#       am   01/10/23 Added xocp_cache_disable configurable parameter
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

	#For Versal devices PLM BSP contains dummy OCP library
	if { $proc_type == "psv_pmc" } {
		foreach entry [glob -nocomplain -types f [file join $common *]] {
			file copy -force $entry "./src"
		}
		return;
	}

	if { $proc_type != "psxl_pmc" && $proc_type != "psxl_cortexa78" &&
		$proc_type != "psxl_cortexr52" && $proc_type != "psx_pmc" &&
		$proc_type != "psx_cortexa78" && $proc_type != "psx_cortexr52" &&
		$proc_type != "microblaze"} {
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
		$proc_type == "psx_cortexa78" || $proc_type == "psx_cortexr52" || $proc_type == "microblaze"} {
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
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

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

	# Get cache_disable value set by user, by default it is TRUE
	set value [common::get_property CONFIG.xocp_cache_disable $libhandle]
	if {$value == true} {
		#Open xparameters.h file
		if {$proc_type == "psxl_cortexa78" || $proc_type == "psx_cortexa78" ||
			$proc_type == "psxl_cortexr52" || $proc_type == "psx_cortexr52" ||
			$proc_type == "microblaze"} {
			set file_handle [hsi::utils::open_include_file "xparameters.h"]

			puts $file_handle "\n/* XilOcp library User Settings */"
			puts $file_handle "#define XOCP_CACHE_DISABLE\n"

			close $file_handle
		}
	}
}
