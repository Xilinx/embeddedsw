###############################################################################
#
# Copyright (C) 2013 - 2019 Xilinx, Inc.  All rights reserved.
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
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# 
#
###############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a rpo  04/25/13 Initial Release
# 3.00  vns  30/07/15 Added macro in xparameters.h based on the
#                     processor
# 6.4   vns  02/27/18 Added support for virtex and virtex ultrascale plus
# 6.7	psl  03/12/19 Disabled compilation of code not required for zynqmp
##############################################################################

#---------------------------------------------
# skey_drc
#---------------------------------------------
proc skey_drc {libhandle} {

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
#	This procedure builds the libxilskey.a library
#-------
proc execs_generate {libhandle} {

}

proc xgen_opts_file {libhandle} {
	set proc_instance [hsi::get_sw_processor];
	set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $proc_instance] ]
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]

	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

	set file_handle [::hsi::utils::open_include_file "xparameters.h"]

	puts $file_handle "\n/* Xilinx processor macro for Secure Library (Xilskey) */ "
	if {$proc_type == "ps7_cortexa9" || $proc_type == "psu_cortexa53" || $proc_type == "psu_cortexr5" || $proc_type == "psu_pmu"} {
		puts $file_handle "\n#define XPAR_XSK_ARM_PLATFORM 1"
	}
	if {$proc_type == "microblaze"} {
		puts $file_handle "\n#define XPAR_XSK_MICROBLAZE_PLATFORM 1"
		set mb_type [common::get_property CONFIG.C_FAMILY $hw_proc_handle]
		if {$mb_type == "kintexuplus" || $mb_type == "virtexuplus"} {
			puts $file_handle "\n#define XPAR_XSK_MICROBLAZE_ULTRA_PLUS 1"
		}
		if {$mb_type == "kintexu" || $mb_type == "virtexu"} {
			puts $file_handle "\n#define XPAR_XSK_MICROBLAZE_ULTRA 1"
		}
	}

	if {$proc_type == "psu_pmu" || $proc_type == "psu_cortexa53" || $proc_type == "psu_cortexr5"} {
		file delete -force ./src/xilskey_epl.c
		file delete -force ./src/xilskey_eps.c
		file delete -force ./src/xilskey_epshw.h
		file delete -force ./src/xilskey_js.h
		file delete -force ./src/xilskey_jscmd.c
		file delete -force ./src/xilskey_jscmd.h
		file delete -force ./src/xilskey_jslib.c
		file delete -force ./src/xilskey_jslib.h
		file delete -force ./src/xilskey_jtag.h
		file delete -force ./src/xilskey_bbram.c
		file delete -force ./src/include/xilskey_epl.h
		file delete -force ./src/include/xilskey_eps.h
        }

	if {$proc_type == "psu_pmu"} {
		file delete -force ./src/xilskey_bbramps_zynqmp.c
                file delete -force ./src/include/xilskey_bbram.h
	}

	puts $file_handle ""
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
