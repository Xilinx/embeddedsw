#/******************************************************************************
#*
#* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
#* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
#* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#* THE SOFTWARE.
#*
#*
#*
#******************************************************************************/
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  ww   05/31/18 Initial Release
##############################################################################

#---------------------------------------------
# libpm_drc
#---------------------------------------------
proc libpm_drc {libhandle} {


}

proc generate {libhandle} {
	# Copy over the right set of files as src based on processor type
	set sw_proc_handle [hsi::get_sw_processor]
	set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
	set proctype [common::get_property IP_NAME $hw_proc_handle]
	set procname [common::get_property NAME    $hw_proc_handle]

	set clientSrcDir "./src/client"
	set serverSrcDir "./src/server"
	set commonSrcdir "./src/common"

	foreach entry [glob -nocomplain [file join $commonSrcdir *]] {
		file copy -force $entry "./src"
	}

	switch $proctype {
		"psv_cortexr5"  -
		"psv_cortexa72"  {
			foreach entry [glob -nocomplain [file join $clientSrcDir *]] {
				file copy -force $entry "./src/"
			}
		}

		"psu_pmc" -
	        "psv_pmc"	{
			foreach entry [glob -nocomplain [file join $serverSrcDir *]] {
				file copy -force $entry "./src/"
			}
		}

		"default"  {error "Error: Processor type $proctype is not supported\n"}
	}

	file delete -force $clientSrcDir
	file delete -force $serverSrcDir
	file delete -force $commonSrcdir
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
