###############################################################################
# Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a ba   06/01/15 Initial Release
# 1.2   vns  08/23/16 Added support for SHA2 by adding .a files
# 2.0   vns  11/28/16 Added support for PMU
# 2.0   srm  02/16/18 Updated to pick up latest freertos port 10.0
# 3.1   vns  04/13/18 Added user configurable macro secure environment
# 4.0   vns  03/20/19 Added support for versal
# 4.1   vns  08/02/19 Added support for a72 and r5 processors of Versal
# 4.3   rpo  07/08/20 Added support to access xsecure init files only for
#                     psv_pmc and psu_pmc processor
##############################################################################

#---------------------------------------------
# secure_drc
#---------------------------------------------
proc secure_drc {libhandle} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set compiler [common::get_property CONFIG.compiler $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os_type [hsi::get_os];
	set common "src/common/"
	set zynqmp "src/zynqmp/"
	set versal "src/versal/"

	foreach entry [glob -nocomplain -types f [file join $common *]] {
			file copy -force $entry "./src"
	}

	if {$proc_type == "psu_cortexa53" ||
		$proc_type == "psu_cortexr5" || $proc_type == "psu_pmu"} {
			foreach entry [glob -nocomplain -types f [file join $zynqmp *]] {
				file copy -force $entry "./src"
			}
	} elseif {$proc_type == "psu_pmc" || $proc_type == "psu_cortexa72" ||
				$proc_type == "psv_pmc" || $proc_type == "psv_cortexa72" ||
				$proc_type == "psv_cortexr5" } {
			foreach entry [glob -nocomplain -types f [file join $versal *]] {
				file copy -force $entry "./src"
			}

			if {$proc_type != "psv_pmc" &&  $proc_type != "psu_pmc"} {
				file delete -force ./src/xsecure_init.c
				file delete -force ./src/xsecure_init.h
			}

			if {[string compare -nocase $compiler "mb-gcc"] == 0} {
				file delete -force ./src/libxilsecure_a72_64.a
				file delete -force ./src/libxilsecure_r5.a
				file rename -force ./src/libxilsecure_pmc.a ./src/libxilsecure.a
			} elseif {[string compare -nocase $compiler "aarch64-none-elf-gcc"] == 0} {
				file delete -force ./src/libxilsecure_pmc.a
				file delete -force ./src/libxilsecure_r5.a
				file rename -force ./src/libxilsecure_a72_64.a ./src/libxilsecure.a
			} elseif {[string compare -nocase $compiler "armr5-none-eabi-gcc"] == 0} {
				file delete -force ./src/libxilsecure_pmc.a
				file delete -force ./src/libxilsecure_a72_64.a
				file rename -force ./src/libxilsecure_r5.a ./src/libxilsecure.a
			}

	} else {
		error "ERROR: XilSecure library is supported only for PMU, CortexA53, CortexR5, CortexA72 and psv_pmc processors.";
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
#	This procedure builds the libxilsecure.a library
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

	# Get secure_environment value set by user, by default it is FALSE
	set value [common::get_property CONFIG.secure_environment $libhandle]
	if {$value == true} {
		# Open xparameters.h file
		set file_handle [hsi::utils::open_include_file "xparameters.h"]

		puts $file_handle "\n/* Xilinx Secure library User Settings */"
		puts $file_handle "#define XSECURE_TRUSTED_ENVIRONMENT \n"

		close $file_handle
	}
}
