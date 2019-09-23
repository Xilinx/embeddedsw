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
#* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
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
# 1.00  pc   01/21/19 Initial creation
##############################################################################

#---------------------------------------------
# sem_drc
#---------------------------------------------
proc sem_drc {libhandle} {


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
#-------
proc execs_generate {libhandle} {

}

proc xgen_opts_file {libhandle} {

	# Export SEM options to xparameters.h if SEM CFRAME and/or NPI reg scan is enabled

	set sem_cfrscan_en [common::get_property CONFIG.SEM_CONFIG_MEM_SCAN [::hsi::get_cells -filter "IP_NAME==versal_cips"]]
	set sem_npiscan_en [common::get_property CONFIG.SEM_CONFIG_NPI_SCAN [::hsi::get_cells -filter "IP_NAME==versal_cips"]]

	if {($sem_cfrscan_en == 1)||($sem_npiscan_en == 1)} {
	  # Open xparameters.h file
	  set file_handle [::hsi::utils::open_include_file "xparameters.h"]

	  puts $file_handle ""
	  puts $file_handle "/* Xilinx Soft Error Mitigation Library (XilSEM) User Settings */"

	  set sem_cfrtest_en [common::get_property CONFIG.SEM_MEM_ENABLE_ALL_TEST_FEATURE [::hsi::get_cells -filter "IP_NAME==versal_cips"]]
	  set sem_cfr_grant  [common::get_property CONFIG.SEM_MEM_ENABLE_SCAN_AFTER_CONFIG [::hsi::get_cells -filter "IP_NAME==versal_cips"]]
	  set sem_cfr_swecc  [common::get_property CONFIG.SEM_MEM_GOLDEN_ECC_SW [::hsi::get_cells -filter "IP_NAME==versal_cips"]]
	  set sem_cfr_errcfg [common::get_property CONFIG.SEM_ERROR_HANDLE_OPTIONS [::hsi::get_cells -filter "IP_NAME==versal_cips"]]
	  set sem_npitest_en [common::get_property CONFIG.SEM_NPI_ENABLE_ALL_TEST_FEATURE [::hsi::get_cells -filter "IP_NAME==versal_cips"]]
	  set sem_npi_swsha  [common::get_property CONFIG.SEM_NPI_GOLDEN_CHECKSUM_SW [::hsi::get_cells -filter "IP_NAME==versal_cips"]]
	  set sem_npi_grant  [common::get_property CONFIG.SEM_NPI_ENABLE_SCAN_AFTER_CONFIG [::hsi::get_cells -filter "IP_NAME==versal_cips"]]

	  if {$sem_cfrscan_en == 1} {
	     puts $file_handle "\#define XSEM_CFRSCAN_EN"
	     if {$sem_cfrtest_en == 1} {puts $file_handle "\#define XSEM_CFRTEST_EN"}
	     if {$sem_cfr_grant == 1} {puts $file_handle "\#define XSEM_CFRGRANT"}
	     if {$sem_cfr_swecc == 1} {puts $file_handle "\#define XSEM_CFRSWECC"}
	     if {$sem_cfr_errcfg eq "Detect & Correct"} {puts $file_handle "\#define XSEM_CFRCORR_EN"}
	  }

	  if {$sem_npiscan_en == 1} {
	    puts $file_handle "\#define XSEM_NPISCAN_EN"
	    if {$sem_npitest_en == 1} {puts $file_handle "\#define XSEM_NPITEST_EN"}
	    if {$sem_npi_swsha == 1} {puts $file_handle "\#define XSEM_NPISWSHA"}
	    if {$sem_npi_grant == 1} {puts $file_handle "\#define XSEM_NPIGRANT"}
	  }

	  puts $file_handle ""
	  close $file_handle
        }

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
