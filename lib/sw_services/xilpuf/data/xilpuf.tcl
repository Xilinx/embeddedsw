###############################################################################
# Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   kal  08/01/19 Initial Release
# 2.0   kpt  08/25/22 Changed user configurable parameter names
#
##############################################################################

#---------------------------------------------
# puf_drc
#---------------------------------------------
proc puf_drc {libhandle} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set compiler [common::get_property CONFIG.compiler $proc_instance]
	set mode [common::get_property CONFIG.xpuf_mode $libhandle]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os_type [hsi::get_os];
	set server "src/server/"
	set client "src/client/"
	set common  "src/common/"

	if {$proc_type != "psv_pmc" && $proc_type != "psv_cortexa72" &&
		$proc_type != "psv_cortexr5" && $proc_type != "psu_pmc" &&
		$proc_type != "microblaze" && $proc_type != "psxl_pmc" &&
		$proc_type != "psxl_cortexa78" && $proc_type != "psxl_cortexr52" &&
		$proc_type != "psx_pmc" && $proc_type != "psx_cortexa78" &&
		$proc_type != "psx_cortexr52"} {
		error "ERROR: XilPuf library is supported only for Versal family of devices";
		return;
	}

	if {$proc_type == "microblaze" && $mode == "server"} {
		error "ERROR: XilPuf server library is not supported on microblaze";
		return;
	}
	foreach entry [glob -nocomplain -types f [file join $common *]] {
			file copy -force $entry "./src"
	}

	if {$proc_type == "psu_pmc" || $proc_type == "psv_pmc" || $proc_type == "psxl_pmc" || $proc_type == "psx_pmc" || $mode == "server"} {
		foreach entry [glob -nocomplain -types f [file join "$server" *]] {
			file copy -force $entry "./src"
		}
	} elseif {$proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
		$proc_type == "psv_cortexr5" || $proc_type == "psxl_cortexa78" ||
		$proc_type == "psxl_cortexr52" || $proc_type == "psx_cortexa78" ||
                $proc_type == "psx_cortexr52" || $proc_type == "microblaze"} {
		set librarylist [hsi::get_libs -filter "NAME==xilmailbox"];
		if { [llength $librarylist] == 0 } {
			error "This library requires xilmailbox library in the Board Support Package.";
		}
		foreach entry [glob -nocomplain -types f [file join "$client" *]] {
			file copy -force $entry "./src"
		}
	}

	if {$mode == "server"} {
		if {$proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
		$proc_type == "psv_cortexr5" || $proc_type == "psxl_cortexa78" ||
		$proc_type == "psxl_cortexr52" || $proc_type == "psx_cortexa78" ||
                $proc_type == "psx_cortexr52"} {
			file delete -force ./src/xpuf_ipihandler.c
			file delete -force ./src/xpuf_ipihandler.h
			file delete -force ./src/xpuf_cmd.c
			file delete -force ./src/xpuf_cmd.h
			file delete -force ./src/xpuf_init.c
			file delete -force ./src/xpuf_init.h
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
#	This procedure builds the libxilpuf.a library
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

	# Get cache_disable value set by user, by default it is FALSE
	set value [common::get_property CONFIG.xpuf_cache_disable $libhandle]
	if {$value == true} {
		#Open xparameters.h file
		if {$proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
                        $proc_type == "psv_cortexr5" || $proc_type == "microblaze" ||
                        $proc_type == "psxl_cortexa78" || $proc_type == "psxl_cortexr52" ||
                        $proc_type == "psx_cortexa78" || $proc_type == "psx_cortexr52"} {
			set file_handle [hsi::utils::open_include_file "xparameters.h"]

			puts $file_handle "\n/* XilPuf library User Settings */"
			puts $file_handle "#define XPUF_CACHE_DISABLE\n"

			close $file_handle
		}
	}

}
