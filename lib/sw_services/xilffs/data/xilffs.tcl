###############################################################################
#
# Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
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
#
# Modification History
#
# Ver   Who   Date     Changes
# ----- ----  -------  -----------------------------------------------
# 1.00a hk/sg 10/17/13 First release
# 2.0   hk    12/13/13 Modified to use new TCL API's
#
##############################################################################

#---------------------------------------------
# FFS_drc - check system configuration and make sure 
# all components to run ISF are available. 
#---------------------------------------------

proc ffs_drc {libhandle} {

	# Check if any IP instances that use FATFS are present
	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells [common::get_property HW_INSTANCE $sw_processor]]

	set ffs_periphs_list [get_ffs_periphs $processor]

	if { [llength $ffs_periphs_list] == 0 } {
		puts "WARNING : No interface that uses file system is available \n"
	}	

}

proc get_ffs_periphs {processor} {
	set periphs_list [hsi::utils::get_proc_slave_periphs $processor]
	set ffs_periphs_list {}

	global ffs_periphs_name_list
	set ffs_periphs_name_list {}

	foreach periph $periphs_list {
		set periphname [common::get_property IP_NAME $periph]
		# Checks if SD instance is present
		# This can be expanded to add more instances.
		if {$periphname == "ps7_sdio"} {
			lappend ffs_periphs_list $periph
			lappend ffs_periphs_name_list $periphname
		}
	}

	return $ffs_periphs_list
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
#	This procedure builds the libisf.a library
#-------
proc execs_generate {libhandle} {

}


proc xgen_opts_file {libhandle} {

	# Open xparameters.h file
	set file_handle [hsi::utils::open_include_file "xparameters.h"]

	# Generate parameters for "file system with SD" and "MMC support"
	puts $file_handle "/* Xilinx FAT File System Library (XilFFs) User Settings */"
	set fs_interface [common::get_property CONFIG.fs_interface $libhandle]
	set enable_mmc [common::get_property CONFIG.enable_mmc $libhandle]

	# Checking if SD with FATFS is enabled.
	# This can be expanded to add more interfaces.
	
	global ffs_periphs_name_list
	foreach periph $ffs_periphs_name_list {
	
		if {$periph == "ps7_sdio"} {
			if {$fs_interface == 1} {
				puts $file_handle "\#define FILE_SYSTEM_INTERFACE_SD"
			} else {
				error  "ERROR: Invalid interface selected \n"
			}
		}

	}
	
	# MMC support
	if {$enable_mmc == true} {
		puts $file_handle "\#define MMC_CARD"
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

