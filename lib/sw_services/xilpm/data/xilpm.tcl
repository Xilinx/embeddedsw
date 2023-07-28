###############################################################################
# Copyright (c) 2015 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
##############################################################################

source [file join [file dirname [info script]] cfg_gen.tcl]

proc check_platform { } {
	set cortexa53proc [hsi::get_cells -hier -filter "IP_NAME==psu_cortexa53"]
	if {[llength $cortexa53proc] > 0} {
		set iszynqmp 1
	} else {
		set iszynqmp 0
	}
	return $iszynqmp
}

proc generate {libhandle} {
	# Copy over the right set of files as src based on processor type
	set sw_proc_handle [hsi::get_sw_processor]
	set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
	set proctype [common::get_property IP_NAME $hw_proc_handle]
	set procname [common::get_property NAME    $hw_proc_handle]

	set iszynqmp [check_platform]
	global rpu0_as_power_management_master
	global rpu1_as_power_management_master
	global apu_as_power_management_master
	global rpu0_as_reset_management_master
	global rpu1_as_reset_management_master
	global apu_as_reset_management_master
	global rpu0_as_overlay_config_master
	global rpu1_as_overlay_config_master
	global apu_as_overlay_config_master
	set rpu0_as_power_management_master [common::get_property CONFIG.rpu0_as_power_management_master $libhandle]
	set rpu1_as_power_management_master [common::get_property CONFIG.rpu1_as_power_management_master $libhandle]
	set apu_as_power_management_master [common::get_property CONFIG.apu_as_power_management_master $libhandle]
	set rpu0_as_reset_management_master [common::get_property CONFIG.rpu0_as_reset_management_master $libhandle]
	set rpu1_as_reset_management_master [common::get_property CONFIG.rpu1_as_reset_management_master $libhandle]
	set apu_as_reset_management_master [common::get_property CONFIG.apu_as_reset_management_master $libhandle]
	set rpu0_as_overlay_config_master [common::get_property CONFIG.rpu0_as_overlay_config_master $libhandle]
	set rpu1_as_overlay_config_master [common::get_property CONFIG.rpu1_as_overlay_config_master $libhandle]
	set apu_as_overlay_config_master [common::get_property CONFIG.apu_as_overlay_config_master $libhandle]

	set zynqmp_dir "./src/zynqmp"
	set versal_dir "./src/versal"
	set versal_net_dir    "./src/versal_net"
	set common_dir "./src/versal_common/"
	set zynqmp_client_a53dir "$zynqmp_dir/client/apu"
	set zynqmp_client_r5dir "$zynqmp_dir/client/rpu"
	set zynqmp_client_commondir "$zynqmp_dir/client/common"
	set versal_client_dir "$versal_dir/client"
	set versal_client_common_dir "$common_dir/client"
	set versal_server_dir "$versal_dir/server"
	set versal_common_dir "$versal_dir/common"
	set versal_net_client_dir "$versal_net_dir/client"
	set versal_net_server_dir "$versal_net_dir/server"
	set versal_net_common_dir "$versal_net_dir/common"
	set versal_server_common_dir "$common_dir/server"
	set common_header_dir "$common_dir/common"


	switch $proctype {
		"psu_cortexa53" {
			copy_files_to_src $zynqmp_client_a53dir
			copy_files_to_src $zynqmp_client_commondir
		}

		"psu_cortexr5" {
			copy_files_to_src $zynqmp_client_r5dir
			copy_files_to_src $zynqmp_client_commondir
		}

		"psv_cortexr5" -
		"microblaze" -
		"psv_cortexa72" {
			if {($iszynqmp == 0)} {
				copy_files_to_src $versal_client_dir
				copy_files_to_src $versal_client_common_dir
				copy_files_to_src $versal_common_dir
				copy_files_to_src $common_header_dir
			} else {
				error "Error: Processor type $proctype is not supported in ZynqMP\n"
			}
		}

		"psxl_cortexa78" -
		"psxl_cortexr52" -
		"psx_cortexa78" -
		"psx_cortexr52" {
			if {($iszynqmp == 0)} {
				copy_files_to_src $versal_net_client_dir
				copy_files_to_src $versal_client_common_dir
				copy_files_to_src $versal_net_common_dir
				copy_files_to_src $common_header_dir
			} else {
				error "Error: Processor type $proctype is not supported in ZynqMP\n"
			}
		}

		"psu_pmc" -
		"psv_pmc" {
			copy_files_to_src $versal_server_dir
			copy_files_to_src $versal_server_common_dir
			copy_files_to_src $versal_common_dir
			copy_files_to_src $common_header_dir
		}
		"psxl_pmc" -
		"psx_pmc" {
			copy_files_to_src $versal_net_server_dir
			copy_files_to_src $versal_server_common_dir
			copy_files_to_src $versal_net_common_dir
			copy_files_to_src $common_header_dir
		}

		"default"  {error "Error: Processor type $proctype is not supported\n"}
	}

	# Generate config object
	if {($proctype == "psu_cortexa53") || ($proctype == "psu_cortexr5")} {
		pmufw::gen_cfg_data [file join src pm_cfg_obj.c] $proctype
	}
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

	set file_handle [::hsi::utils::open_include_file "xparameters.h"]
	puts $file_handle "\#define XPAR_XILPM_ENABLED"
	set part [::hsi::get_current_part]
	set part_name [string range $part 0 [expr {[string first "-" $part] - 1}]]

	# Add macro for enabling P80 related code
	if { [string match -nocase "xcvp1902" $part_name] } {
		puts $file_handle "\#define XCVP1902"
	}
	close $file_handle

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
