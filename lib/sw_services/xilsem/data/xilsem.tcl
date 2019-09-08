###############################################################################
# Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  pc   01/21/19 Initial creation
# 1.01  pc   03/24/20 Update copyright year
# 1.02  pc   06/05/20 Remove unused params
# 1.03  pc   07/16/20 Update syntax to fetch CIPS HIP properties
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

proc getCIPSProperty { cips_prop } {
  set pspmcCell [::hsi::get_cells -hier -filter "IP_NAME==pspmc"]
  if {$pspmcCell ne ""} {
    return [common::get_property $cips_prop $pspmcCell]
  } else {
    set cipsCell [::hsi::get_cells -hier -filter "IP_NAME==versal_cips"]
    if {$cipsCell ne ""} {
      set isHierIp [common::get_property IS_HIERARCHICAL $cipsCell]
      if {$isHierIp} {
        set ps_pmc_config [common::get_property CONFIG.PS_PMC_CONFIG $cipsCell]
        set prop_exists [dict get $ps_pmc_config $cips_prop]
        if {$prop_exists} {
          return [dict get $ps_pmc_config $cips_prop]
        } else {
          return 0
        }
      } else {
        return [common::get_property $cips_prop $cipsCell]
      }
    }
    return 0
  }
  return 0
}

proc xgen_opts_file {libhandle} {

	# Export SEM options to xparameters.h if SEM CFRAME and/or NPI reg scan is enabled

	set sem_cfrscan_en [getCIPSProperty CONFIG.SEM_CONFIG_MEM_SCAN]
	set sem_npiscan_en [getCIPSProperty CONFIG.SEM_CONFIG_NPI_SCAN]

	if {($sem_cfrscan_en == 1)||($sem_npiscan_en == 1)} {
	  # Open xparameters.h file
	  set file_handle [::hsi::utils::open_include_file "xparameters.h"]

	  puts $file_handle ""
	  puts $file_handle "/* Xilinx Soft Error Mitigation Library (XilSEM) User Settings */"

	  if {$sem_cfrscan_en == 1} {
	     puts $file_handle "\#define XSEM_CFRSCAN_EN"
	  }

	  if {$sem_npiscan_en == 1} {
	    puts $file_handle "\#define XSEM_NPISCAN_EN"
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
