###############################################################################
# Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
##############################################################################

source [file join [file dirname [info script]] cfg_gen.tcl]
proc generate {libhandle} {
	# Copy over the right set of files as src based on processor type
	set sw_proc_handle [hsi::get_sw_processor]
	set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
	set proctype [common::get_property IP_NAME $hw_proc_handle]
	set procname [common::get_property NAME    $hw_proc_handle]

	set zynqmp_dir "./src/zynqmp"
	set versal_dir "./src/versal"
	set zynqmp_client_a53dir "$zynqmp_dir/client/apu"
	set zynqmp_client_r5dir "$zynqmp_dir/client/rpu"
	set zynqmp_client_commondir "$zynqmp_dir/client/common"
	set versal_client_dir "$versal_dir/client"
	set versal_server_dir "$versal_dir/server"
	set versal_common_dir "$versal_dir/common"

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
		"psv_cortexa72" {
			copy_files_to_src $versal_client_dir
			copy_files_to_src $versal_common_dir
		}

		"psu_pmc" -
		"psv_pmc" {
			copy_files_to_src $versal_server_dir
			copy_files_to_src $versal_common_dir
		}

		"default"  {error "Error: Processor type $proctype is not supported\n"}
	}

	# Generate config object
	if {($proctype == "psu_cortexa53") || ($proctype == "psu_cortexr5")} {
		pmufw::gen_cfg_data [file join src pm_cfg_obj.c]
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
