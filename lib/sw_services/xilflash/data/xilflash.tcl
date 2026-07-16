###############################################################################
# Copyright (c) 2007 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
###############################################################################

## @BEGIN_CHANGELOG EDK_LS3
## Added XPAR_XFL_PLATFORM_FLASH to be generated in the xparameters.h.
## If the user selects Platform Flash XL device to be used then
## parameter will be set to 1.
##
## @END_CHANGELOG
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 2.03a sdm  09/24/10 updated to use Tcl commands instead of unix commands
# 3.00a sdm  03/03/11 Removed static flash parameters in the library
# 3.00a sdm  03/23/11 Added new parameters to enable support for flash families
#
##############################################################################

#---------------------------------------------
# Flash_drc
#---------------------------------------------
proc flash_drc {libhandle} {

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
#	This procedure builds the libxilflash.a library
#-------
proc execs_generate {libhandle} {

}

proc xgen_opts_file {libhandle} {

	# Open xparameters.h file
	set file_handle [::hsi::utils::open_include_file "xparameters.h"]

	# Generate parameters for Flash family support
	puts $file_handle "/* Xilinx EDK Parallel Flash Library (XilFlash) User Settings */"
	set enable_intel [common::get_property CONFIG.enable_intel $libhandle]
	set enable_amd [common::get_property CONFIG.enable_amd $libhandle]

	if {$enable_intel == false && $enable_amd == false} {
		error  "ERROR: no flash family enabled. enable at least one flash family in the bsp settings and rebuild the libraries"
	}

	if {$enable_intel == true} {
		puts $file_handle "\#define XPAR_XFL_DEVICE_FAMILY_INTEL"
	}

	if {$enable_amd == true} {
		puts $file_handle "\#define XPAR_XFL_DEVICE_FAMILY_AMD"
	}

	close $file_handle

	# Copy the include files to the include directory
	set srcdir [file join src include]
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
