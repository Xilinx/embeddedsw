###############################################################################
# Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  kc   09/22/18 Initial Release
# 1.10  ssc  03/05/22 Added configurable options
# 1.2   bm   07/06/22 Added versal net support
#       dc   07/13/22 Added OCP configuration enable support for VersalNet
#       ma   07/27/22 Added configurable option for SSIT PLM to PLM
#                     communication feature
#       ma   08/10/22 Enable SSIT PLM to PLM communication feature based on
#                     user option and Number of SLRs from the design
# 1.8   skg  12/07/2022 Added plm_add_ppks_en user configuration
# 1.9   rama 08/08/2023 Added logic for overriding plm_dbg_lvl to 0
#                      for XilSEM enabled designs
#
##############################################################################

#---------------------------------------------
# plmi_drc
#---------------------------------------------
proc plmi_drc {libhandle} {
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set versal_net "src/versal_net/"
	set versal "src/versal/"
	set common "src/common/"

	foreach entry [glob -nocomplain -types f [file join ./src/ *]] {
		if {$entry != "./src/Makefile"} {
			file delete -force $entry
		}
	}

	foreach entry [glob -nocomplain -types f [file join $common *]] {
		file copy -force $entry "./src"
	}
	if {$proc_type == "psxl_pmc" || $proc_type == "psx_pmc"} {
		foreach entry [glob -nocomplain -types f [file join $versal_net *]] {
			file copy -force $entry "./src"
		}
	}

	if {$proc_type == "psv_pmc"} {
		foreach entry [glob -nocomplain -types f [file join $versal *]] {
			file copy -force $entry "./src"
		}
	}

	file delete -force $versal_net
	file delete -force $versal
	file delete -force $common
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

	# Copy the include files to the include directory
	set srcdir src
	set dstdir [file join .. .. include]
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set IsAddPpkEn false

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

	# Open xparameters.h file
	set file_handle [hsi::utils::open_include_file "xparameters.h"]

	puts $file_handle "\n/* PLM/XilPLMI configuration */"

	# Get plm_uart_dbg_en value set by user, by default it is TRUE
	set value [common::get_property CONFIG.plm_uart_dbg_en $libhandle]
	if {$value == false} {
		puts $file_handle "\n/* Disable debug prints from UART (but logged to memory) */"
		puts $file_handle "#define PLM_PRINT_NO_UART"
	}

	# Get plm_dbg_lvl value set by user, by default it is general
	set value [common::get_property CONFIG.plm_dbg_lvl $libhandle]
	set sem_print_flag [common::get_property CONFIG.sem_override_dbg_lvl $libhandle]
	set sem_cfrscan_en [getCIPSProperty CONFIG.SEM_MEM_SCAN]
	set sem_npiscan_en [getCIPSProperty CONFIG.SEM_NPI_SCAN]
	puts $file_handle "\n/* Debug level option */"
	if {((($sem_cfrscan_en == 1) || ($sem_npiscan_en == 1)) && ($sem_print_flag == true))} {
		puts "Level_0 is selected"
		puts $file_handle "#define PLM_PRINT"
	} elseif {$value == "level0"} {
		puts $file_handle "#define PLM_PRINT"
	} elseif {$value == "level1"} {
		puts $file_handle "#define PLM_DEBUG"
	} elseif {$value == "level2"} {
		puts $file_handle "#define PLM_DEBUG_INFO"
	} elseif {$value == "level3"} {
		puts $file_handle "#define PLM_DEBUG_DETAILED"
	}

	# Get plm_mode value set by user, by default it is release
	set value [common::get_property CONFIG.plm_mode $libhandle]
	if {$value == "debug"} {
		puts $file_handle "\n/* PLM mode option */"
		puts $file_handle "#define PLM_DEBUG_MODE"
	}

	# Get plm_perf_en value set by user, by default it is TRUE
	set value [common::get_property CONFIG.plm_perf_en $libhandle]
	if {$value == true} {
		puts $file_handle "\n/* Boot time measurement enable */"
		puts $file_handle "#define PLM_PRINT_PERF"
	}

	# Get plm_qspi_en value set by user, by default it is TRUE
	set value [common::get_property CONFIG.plm_qspi_en $libhandle]
	if {$value == false} {
		puts $file_handle "\n/* QSPI Boot mode support disable */"
		puts $file_handle "#define PLM_QSPI_EXCLUDE"
	}

	# Get plm_sd_en value set by user, by default it is TRUE
	set value [common::get_property CONFIG.plm_sd_en $libhandle]
	if {$value == false} {
		puts $file_handle "\n/* SD Boot mode support disable */"
		puts $file_handle "#define PLM_SD_EXCLUDE"
	}

	# Get plm_ospi_en value set by user, by default it is TRUE
	set value [common::get_property CONFIG.plm_ospi_en $libhandle]
	if {$value == false} {
		puts $file_handle "\n/* OSPI Boot mode support disable */"
		puts $file_handle "#define PLM_OSPI_EXCLUDE"
	}

	# Get plm_sem_en value set by user, by default it is TRUE
	set value [common::get_property CONFIG.plm_sem_en $libhandle]
	if {$value == false} {
		puts $file_handle "\n/* SEM feature disable */"
		puts $file_handle "#define PLM_SEM_EXCLUDE"
	}

	# Get plm_secure_en value set by user, by default it is TRUE
	set value [common::get_property CONFIG.plm_secure_en $libhandle]
	if {$value == false} {
		puts $file_handle "\n/* Secure features disable */"
		puts $file_handle "#define PLM_SECURE_EXCLUDE"
	}

	# Get plm_usb_en value set by user, by default it is FALSE
	set value [common::get_property CONFIG.plm_usb_en $libhandle]
	if {$value == false} {
		puts $file_handle "\n/* USB Boot mode support disable */"
		puts $file_handle "#define PLM_USB_EXCLUDE"
	}

	# Get plm_nvm_en value set by user, by default it is FALSE
	set value [common::get_property CONFIG.plm_nvm_en $libhandle]
	if {$value == false} {
		puts $file_handle "\n/* NVM handlers disable */"
		puts $file_handle "#define PLM_NVM_EXCLUDE"
	}

	# Get plm_puf_en value set by user, by default it is FALSE
	set value [common::get_property CONFIG.plm_puf_en $libhandle]
	if {$value == false} {
		puts $file_handle "\n/* PUF handlers disable */"
		puts $file_handle "#define PLM_PUF_EXCLUDE"
	}

	# Get plm_stl_en value set by user, by default it is FALSE
	set value [common::get_property CONFIG.plm_stl_en $libhandle]
	if {$value == true} {
		puts $file_handle "\n/* STL code enable */"
		puts $file_handle "#define PLM_ENABLE_STL"
	}

	# Get plm_ocp_en value set by user, by default it is FALSE(Valid only for VersalNet)
	set value [common::get_property CONFIG.plm_ocp_en $libhandle]
	if {$value == false} {
		if {$proc_type == "psxl_pmc" || $proc_type == "psx_pmc"} {
			puts $file_handle "\n/* OCP code disable */"6
			puts $file_handle "#define PLM_OCP_EXCLUDE"
		}
	}

	# Check if hsi::get_current_part is available
	if { [info commands ::hsi::get_current_part] != ""} {
		#Get part name from the design
		set part [::hsi::get_current_part]

		# Get number of SLRs from the design
		set SlrCount [common::get_property NUM_OF_SLRS $part]
		puts $file_handle "\n/* Number of SLRs */"
		puts $file_handle "#define NUMBER_OF_SLRS       $SlrCount"
		# Get ssit_plm_to_plm_comm_en value set by user, by default it is TRUE(Valid only for Versal)
		set value [common::get_property CONFIG.ssit_plm_to_plm_comm_en $libhandle]
		# Based on ssit_plm_to_plm_comm_en value set by user and the Number Of SLRs present
		# in the design, enable SSIT PLM-PLM communication
		if {($value == true) && ($SlrCount > 1)} {
			puts $file_handle "\n/* SSIT PLM to PLM Communication enable */"
			puts $file_handle "#define PLM_ENABLE_PLM_TO_PLM_COMM"
		}

		#Enable Additional PPKs for M50 design
		set PartName [string range $part 0 [expr {[string first "-" $part] - 1}]]
		if { [string match -nocase "xcvp1052" $PartName] } {
			set IsAddPpkEn true
		}
	}

    # Get plm_add_ppks_en value set by user, by default it is FALSE
	set value [common::get_property CONFIG.plm_add_ppks_en $libhandle]
	if {$value == true || $IsAddPpkEn == true} {
		puts $file_handle "\n/* Enable boot support for additional PPKs */"
		puts $file_handle "#define PLM_EN_ADD_PPKS"
	}

	# Get plm_ecdsa_en value set by user, by default it is FALSE
	set value [common::get_property CONFIG.plm_ecdsa_en $libhandle]
	if {$value == false} {
		if {$proc_type == "psxl_pmc" || $proc_type == "psx_pmc" || $proc_type == "psv_pmc"} {
			set file_handle [hsi::utils::open_include_file "xparameters.h"]
			puts $file_handle "\n/* ECDSA code disable */"
			puts $file_handle "#define PLM_ECDSA_EXCLUDE"
		}
	}

	# Get plm_rsa_en value set by user, by default it is FALSE
	set value [common::get_property CONFIG.plm_rsa_en $libhandle]
	if {$value == false} {
		if {$proc_type == "psxl_pmc" || $proc_type == "psx_pmc" || $proc_type == "psv_pmc"} {
			set file_handle [hsi::utils::open_include_file "xparameters.h"]
			puts $file_handle "\n/* RSA code disable */"
			puts $file_handle "#define PLM_RSA_EXCLUDE"
		}
	}

	puts $file_handle "\n"
	close $file_handle
}
