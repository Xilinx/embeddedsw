###############################################################################
# Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0  mmd  05/06/19 Initial Release
#
##############################################################################

#---------------------------------------------
# nvm_drc
#---------------------------------------------
proc nvm_drc {libhandle} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set compiler [common::get_property CONFIG.compiler $proc_instance]
	set mode [common::get_property CONFIG.mode $libhandle]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os_type [hsi::get_os];
	set srddir "src/bbram/"
	set common "src/common/"
	set server "src/server/"
	set client "src/client/"

	if {$proc_type != "psu_pmc" && $proc_type != "psu_cortexa72" && \
	    $proc_type != "psv_pmc" && $proc_type != "psv_cortexa72" && \
	    $proc_type != "psv_cortexr5"} {
		error "ERROR: XilNvm library is supported only for PSU PMC, \
		      PSU Cortexa72, PSV PMC, PSV Cortexa72.";
	}

	foreach entry [glob -nocomplain -types f [file join $common *]] {
			file copy -force $entry "./src"
	}

	if {$proc_type == "psu_pmc" || $proc_type == "psv_pmc" || $mode == "server"} {
		foreach entry [glob -nocomplain -types f [file join "$server" *]] {
			file copy -force $entry "./src"
		}
	} elseif {$proc_type != "psu_pmc" || $proc_type != "psv_pmc"} {
		foreach entry [glob -nocomplain -types f [file join "$client" *]] {
			file copy -force $entry "./src"
		}
	}
	if {$mode == "server"} {
		if {$proc_type != "psu_pmc" || $proc_type != "psv_pmc"} {
			file delete -force ./src/xnvm_bbram_ipihandler.c
			file delete -force ./src/xnvm_bbram_ipihandler.h
			file delete -force ./src/xnvm_cmd.c
			file delete -force ./src/xnvm_cmd.h
			file delete -force ./src/xnvm_defs.h
			file delete -force ./src/xnvm_init.c
			file delete -force ./src/xnvm_init.h
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
#	This procedure builds the libxilnvm.a library
#-------
proc execs_generate {libhandle} {

}

proc xgen_opts_file {libhandle} {

	# Copy the include files to the include directory
	set srcdir src
	set dstdir [file join .. .. include]
	set access_puf_efuse [common::get_property CONFIG.use_puf_hd_as_user_efuse $libhandle]
	set file_handle [::hsi::utils::open_include_file "xparameters.h"]

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

	if {$access_puf_efuse == true} {
		puts $file_handle "\n#define XNVM_ACCESS_PUF_USER_DATA \n"
	}

	close $file_handle

}
