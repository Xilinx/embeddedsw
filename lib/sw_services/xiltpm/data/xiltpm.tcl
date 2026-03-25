###############################################################################
# Copyright (c) 2026, Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   tri  03/13/25 Initial Release
# 1.2   Pre  03/16/26 Added client support
#
##############################################################################

#---------------------------------------------
# tpm_drc
#---------------------------------------------
proc tpm_drc {libhandle} {
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set compiler [common::get_property CONFIG.compiler $proc_instance]
	set mode [common::get_property CONFIG.xtpm_mode $libhandle]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os_type [hsi::get_os];

	set client_dir "./src/client"
	set server_dir "./src/server"
	set common_dir "./src/common"

	if {$mode == "server"} {
		if {$proc_type != "psu_pmc" && $proc_type != "psv_pmc"} {
				error "ERROR: XilTpm library is not supported for selected processor in\
						 server mode.";
			return;
		}
	}

	if {$mode == "client" &&  ($proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
        $proc_type == "psv_cortexr5" || $proc_type == "microblaze")} {
        set librarylist [hsi::get_libs -filter "NAME==xilmailbox"];
        if { [llength $librarylist] == 0 } {
            error "This library requires xilmailbox library in the \
			Board Support Package.";
        }
    }

	switch $proc_type {
		"psu_pmc" -
		"psv_pmc" {
			copy_files_to_src $server_dir
			copy_files_to_src $common_dir
		}

		"psu_cortexa72" -
		"psv_cortexa72" -
		"psv_cortexr5" -
		"microblaze" {
			copy_files_to_src $client_dir
			copy_files_to_src $common_dir
		}
	}
}

proc generate {libhandle} {

}

proc copy_files_to_src {dir_path} {
        foreach entry [glob -nocomplain [file join $dir_path *]] {
                file copy -force $entry "./src"
        }
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
	set file_handle [::hsi::utils::open_include_file "xparameters.h"]
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
	set value [common::get_property CONFIG.xtpm_cache_disable $libhandle]
	if {$value == true} {
		#Open xparameters.h file
		if {$proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
                        $proc_type == "psv_cortexr5" || $proc_type == "microblaze" } {
			set file_handle [hsi::utils::open_include_file "xparameters.h"]
			puts $file_handle "#define XTPM_CACHE_DISABLE\n"
		}
	}

	close $file_handle
}
