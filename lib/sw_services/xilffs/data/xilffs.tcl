###############################################################################
# Copyright (c) 2013 - 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who   Date     Changes
# ----- ----  -------  -----------------------------------------------
# 1.00a hk/sg 10/17/13 First release
# 2.0   hk    12/13/13 Modified to use new TCL API's
# 4.1   hk    11/21/18 Use additional LFN options
#
##############################################################################

#---------------------------------------------
# FFS_drc - check system configuration and make sure
# all components to run ISF are available.
#---------------------------------------------

proc ffs_drc {libhandle} {

	# Check if any IP instances that use FATFS are present
	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]

	set ffs_periphs_list [get_ffs_periphs $processor]

	set fs_interface [common::get_property CONFIG.fs_interface $libhandle]

	# No need to check if fs_interface is RAM
	if {$fs_interface != 2 && [llength $ffs_periphs_list] == 0} {
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
		if {$periphname == "ps7_sdio" || $periphname == "psu_sd" || $periphname == "psv_pmc_sd" || $periphname == "psxl_pmc_sd" || $periphname == "psxl_pmc_emmc" || $periphname == "psx_pmc_sd" || $periphname == "psx_pmc_emmc"} {
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
	set read_only [common::get_property CONFIG.read_only $libhandle]
	set enable_exfat [common::get_property CONFIG.enable_exfat $libhandle]
	set use_lfn [common::get_property CONFIG.use_lfn $libhandle]
	set use_mkfs [common::get_property CONFIG.use_mkfs $libhandle]
	set use_trim [common::get_property CONFIG.use_trim $libhandle]
	set enable_multi_partition [common::get_property CONFIG.enable_multi_partition $libhandle]
	set num_logical_vol [common::get_property CONFIG.num_logical_vol $libhandle]
	set use_strfunc [common::get_property CONFIG.use_strfunc $libhandle]
	set set_fs_rpath [common::get_property CONFIG.set_fs_rpath $libhandle]
	set word_access [common::get_property CONFIG.word_access $libhandle]
	set use_chmod [common::get_property CONFIG.use_chmod $libhandle]

	# do processor specific checks
	set proc  [hsi::get_sw_processor];
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc]]

	if {$fs_interface == 2} {
		set ramfs_size [common::get_property CONFIG.ramfs_size $libhandle]
		set ramfs_start_addr [common::get_property CONFIG.ramfs_start_addr $libhandle]

		puts $file_handle "\#define FILE_SYSTEM_INTERFACE_RAM"

		puts $file_handle "\#define RAMFS_SIZE $ramfs_size"

		# Check if ram fs start address is defined?
		# If not set default; Give warning in case of Microblaze
		if {$ramfs_start_addr == ""} {
			puts $file_handle "\#define RAMFS_START_ADDR 0x10000000"

			if {$proc_type == "microblaze"} {
				puts "WARNING : Specify RAM FS start address \
						in system.mss for Microblaze\n"
			}
		} else {
			puts $file_handle "\#define RAMFS_START_ADDR $ramfs_start_addr"
		}
	}


	# Checking if SD with FATFS is enabled.
	# This can be expanded to add more interfaces.

	global ffs_periphs_name_list
	foreach periph $ffs_periphs_name_list {

		if {$periph == "ps7_sdio" || $periph == "psu_sd" || $periph == "psv_pmc_sd" || $periph == "psxl_pmc_sd" || $periph == "psxl_pmc_emmc" || $periph == "psx_pmc_sd" || $periph == "psx_pmc_emmc"} {
			if {$fs_interface == 1} {
				puts $file_handle "\#define FILE_SYSTEM_INTERFACE_SD"
				break
			}
		}
	}

	if {$fs_interface == 1 || $fs_interface == 2} {
		if {$read_only == true} {
			puts $file_handle "\#define FILE_SYSTEM_READ_ONLY"
		}
		if {$enable_exfat == true} {
			puts $file_handle "\#define FILE_SYSTEM_FS_EXFAT"
			set use_lfn 1
		}
		if {$use_lfn > 0 && $use_lfn < 4} {
			puts $file_handle "\#define FILE_SYSTEM_USE_LFN $use_lfn"
		}
		if {$use_mkfs == true} {
			puts $file_handle "\#define FILE_SYSTEM_USE_MKFS"
		}
		if {$enable_multi_partition == true} {
			puts $file_handle "\#define FILE_SYSTEM_MULTI_PARTITION"
		}
		if {$use_chmod == true} {
			if {$read_only == false} {
				puts $file_handle "\#define FILE_SYSTEM_USE_CHMOD"
			} else {
				puts "WARNING : Cannot Enable CHMOD in \
						Read Only Mode"
			}
		}
		if {$use_trim == true} {
			puts $file_handle "\#define FILE_SYSTEM_USE_TRIM"
		}
		if {$num_logical_vol > 10} {
			puts "WARNING : File System supports only up to 10 logical drives\
					Setting back the num of vol to 10\n"
			set num_logical_vol 10
		}
		puts $file_handle "\#define FILE_SYSTEM_NUM_LOGIC_VOL $num_logical_vol"
		if {$use_strfunc > 2} {
			puts "WARNING : Invalid STRFUNC option, setting \
					back to 1\n"
			set use_strfunc 1
		}
		puts $file_handle "\#define FILE_SYSTEM_USE_STRFUNC $use_strfunc"
		if {$set_fs_rpath > 2} {
			puts "WARNING : Invalid FS_RPATH option, setting \
					back to 0\n"
			set set_fs_rpath 0
		}
		puts $file_handle "\#define FILE_SYSTEM_SET_FS_RPATH $set_fs_rpath"

		# MB does not allow word access from RAM
		if {$proc_type != "microblaze" && $word_access == true} {
			puts $file_handle "\#define FILE_SYSTEM_WORD_ACCESS"
		}
	} else {
		error  "ERROR: Invalid interface selected \n"
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
