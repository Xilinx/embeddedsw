###############################################################################
# Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
# 1.04  rb   03/09/21 Update SEM parameters as per CIPS 3.0
# 1.05  rb   03/16/21 Created server directory and handling
# 1.06  rb   03/16/21 Created R5 client directory and handling
# 1.07	hv   06/06/22 Added support for vp1902
# 1.08  gm   11/22/22 Added support for A72
# 1.09	hv   11/16/22 Added support for PL microblaze
# 1.10	hv   02/14/23 Added support to get number of SLRs from the design
# 1.11	rv   04/20/23 Added support for psxl IP name
# 1.12	rv   05/11/23 Added support for Versal-net device
##############################################################################

#---------------------------------------------
# sem_drc
#---------------------------------------------
proc sem_drc {libhandle} {


}

proc generate {libhandle} {
	set sw_proc_handle [hsi::get_sw_processor]
	set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
	set proctype [common::get_property IP_NAME $hw_proc_handle]
	set procname [common::get_property NAME    $hw_proc_handle]

	set client_dir "./src/client/"
	set server_dir "./src/server/"

	switch $proctype {
		"psu_pmc" -
		"psv_pmc" {
			copy_files_to_src $server_dir
			file delete -force ./src/libxilsem_versal_net.a
			file rename -force ./src/libxilsem_versal.a ./src/libxilsem.a
		}
		"psxl_pmc" -
		"psx_pmc" {
			copy_files_to_src $server_dir
			file delete -force ./src/libxilsem_versal.a
			file rename -force ./src/libxilsem_versal_net.a ./src/libxilsem.a
		}

		"psv_cortexr5" -
		"psxl_cortexr52" -
		"psx_cortexr52" -
        "psu_cortexa72" -
        "psv_cortexa72" {
			copy_files_to_src $client_dir
		}
		"microblaze" {
			copy_files_to_src $client_dir
		}
		"default"  {error "Error: Processor type $proctype is not supported\n"}
	}
}

proc copy_files_to_src {dir_path} {
	foreach entry [glob -directory $dir_path -nocomplain *] {
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

proc getCIPSProperty { cips_prop } {
  set pspmcCell [::hsi::get_cells -hier -filter "IP_NAME==pspmc"]
  if {$pspmcCell eq ""} {
	set pspmcCell [::hsi::get_cells -hier -filter "IP_NAME==pmcps"]
  }
  if {$pspmcCell eq ""} {
	set pspmcCell [::hsi::get_cells -hier -filter "IP_NAME==psxl"]
  }
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

	set sem_cfrscan_en [getCIPSProperty CONFIG.SEM_MEM_SCAN]
	set sem_npiscan_en [getCIPSProperty CONFIG.SEM_NPI_SCAN]

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

	  if { [info commands ::hsi::get_current_part] != ""} {
	      #Get number of SLRs from the design
	      set part [::hsi::get_current_part]
	      set SlrCount [common::get_property NUM_OF_SLRS $part]
	      puts $file_handle "\n/** Maximum number of SLRs on SSIT device */"
	      puts $file_handle "#define XSEM_SSIT_MAX_SLR_CNT       $SlrCount"
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
