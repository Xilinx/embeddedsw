#/******************************************************************************
#*
#* Copyright (C) 2015-2019 Xilinx, Inc.  All rights reserved.
#*
#* Permission is hereby granted, free of charge, to any person obtaining a copy
#* of this software and associated documentation files (the "Software"), to deal
#* in the Software without restriction, including without limitation the rights
#* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#* copies of the Software, and to permit persons to whom the Software is
#* furnished to do so, subject to the following conditions:
#*
#* The above copyright notice and this permission notice shall be included in
#* all copies or substantial portions of the Software.
#*
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#* THE SOFTWARE.
#*
#*
#*
#******************************************************************************/
source [file join [file dirname [info script]] cfg_gen.tcl]
proc generate {libhandle} {
	# Copy over the right set of files as src based on processor type
	set sw_proc_handle [hsi::get_sw_processor]
	set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
	set proctype [common::get_property IP_NAME $hw_proc_handle]
	set procname [common::get_property NAME    $hw_proc_handle]

	set client_zynqmp_a53dir "./src/client_zynqmp/apu"
	set client_zynqmp_r5dir "./src/client_zynqmp/rpu"
	set client_zynqmp_commondir "./src/client_zynqmp/common"
	set client_zynqmp_dir "./src/client_zynqmp"
	set client_versal_dir "./src/client_versal"
	set server_versal_dir "./src/server_versal"

	switch $proctype {
		"psu_cortexa53" {
			copy_files_to_src $client_zynqmp_a53dir
			copy_files_to_src $client_zynqmp_commondir
		}

		"psu_cortexr5" {
			copy_files_to_src $client_zynqmp_r5dir
			copy_files_to_src $client_zynqmp_commondir
		}

		"psv_cortexr5" -
		"psv_cortexa72" {
			copy_files_to_src $client_versal_dir
		}

		"psu_pmc" -
		"psv_pmc" {
			copy_files_to_src $server_versal_dir
		}

		"default"  {error "Error: Processor type $proctype is not supported\n"}
	}

	file delete -force $client_zynqmp_dir
	file delete -force $client_versal_dir
	file delete -force $server_versal_dir

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
