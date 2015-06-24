###############################################################################
#
# Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
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
		error  "ERROR: no flash family enabled. enable atleast one flash family in the bsp settings and rebuild the libraries"
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
