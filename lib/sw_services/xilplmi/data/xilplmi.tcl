###############################################################################
# Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc. All Rights Reserved.
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
# 2.0   ng   11/11/2023 Added option to set number of user modules
#       dd   01/09/2024 Added client support
#       gm   03/01/2024 Fixed the debug level option for XilSEM
#                       for VP1902 decive.
#       gm   03/28/2024 Modified read format of configurable parameter
#                       for VP1902 device.
# 2.1   sk   06/05/2024 Added config for User Defined PLM Version
#
##############################################################################

#---------------------------------------------
# plmi_drc
#---------------------------------------------
proc plmi_drc {libhandle} {
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set compiler [common::get_property CONFIG.compiler $proc_instance]
	set mode [common::get_property CONFIG.xplmir_mode $libhandle]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os_type [hsi::get_os];
	set versal_net "src/versal_net/"
	set versal "src/versal/"
	set common "src/common/"

	set versal_client "$versal/client"
	set versal_server "$versal/server"
	set versal_common "$versal/common"

	set versal_net_client "$versal_net/client"
	set versal_net_server "$versal_net/server"
	set versal_net_common "$versal_net/common"

	set common_client "$common/client"
	set common_server "$common/server"
	set common_common "$common/common"

	if {$proc_type != "psu_pmc" && $proc_type != "psu_cortexa72" && \
	    $proc_type != "psv_pmc" && $proc_type != "psv_cortexa72" && \
	    $proc_type != "psv_cortexr5" && $proc_type != "microblaze" && \
	    $proc_type != "psxl_pmc" && $proc_type != "psxl_cortexa78" && \
	    $proc_type != "psxl_cortexr52" && $proc_type != "psx_cortexa78" && \
        $proc_type != "psx_cortexr52" && $proc_type != "psx_pmc"} {
		error "ERROR: XilLoader library is supported only for PSU PMC, PSU Cortexa72, \
			PSV PMC, PSV Cortexa72, PSV Cortexr5, PSX PMC, Microblaze, PSXL PMC, \
				PSXL Cortexa78, PSXL Cortexr52, PSX Cortexa78, PSX Cortexr52.";
	}
	if {$mode == "server"} {
		if {$proc_type != "psu_pmc" && $proc_type != "psv_pmc" && $proc_type != "psxl_pmc" &&
				$proc_type != "psx_pmc"} {
				error "ERROR: XilPlmi library is not supported for selected processor in\
						 server mode.";
			return;
		}
	}

	if {$mode == "client" &&  ($proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
	$proc_type == "psv_cortexr5" || $proc_type == "microblaze" ||
        $proc_type == "psxl_cortexa78" || $proc_type == "psxl_cortexr52" ||
        $proc_type == "psx_cortexa78" || $proc_type == "psx_cortexr52")} {
			set librarylist [hsi::get_libs -filter "NAME==xilmailbox"];
			if { [llength $librarylist] == 0 } {
		error "This library requires xilmailbox library in the \
					Board Support Package.";
            }
        }

		set is_versal [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa72" || IP_NAME=="psv_cortexa72" ||
                                IP_NAME=="psv_cortexr5"}]

		switch $proc_type {
		"psu_pmc" -
		"psv_pmc" {
			foreach entry [glob -nocomplain -types f [file join $common_server *]] {
				file copy -force $entry "./src"
			}
			foreach entry [glob -nocomplain -types f [file join $versal_server *]] {
				file copy -force $entry "./src"
			}
			foreach entry [glob -nocomplain -types f [file join $versal_common *]] {
				file copy -force $entry "./src"
			}
			foreach entry [glob -nocomplain -types f [file join $common_common *]] {
				file copy -force $entry "./src"
			}
		}

		"psxl_pmc" -
		"psx_pmc" {
			foreach entry [glob -nocomplain -types f [file join $common_server *]] {
				file copy -force $entry "./src"
			}
			foreach entry [glob -nocomplain -types f [file join $versal_net_server *]] {
				file copy -force $entry "./src"
			}
			foreach entry [glob -nocomplain -types f [file join $versal_net_common *]] {
				file copy -force $entry "./src"
			}
			foreach entry [glob -nocomplain -types f [file join $common_common *]] {
				file copy -force $entry "./src"
			}
		}

		"psu_cortexa72" -
		"psv_cortexa72" -
		"psv_cortexr5" {
			if { [llength $is_versal] > 0 } {
				foreach entry [glob -nocomplain -types f [file join $common_client *]] {
					file copy -force $entry "./src"
				}
				foreach entry [glob -nocomplain -types f [file join $versal_client *]] {
					file copy -force $entry "./src"
				}
				foreach entry [glob -nocomplain -types f [file join $versal_common *]] {
					file copy -force $entry "./src"
				}
				foreach entry [glob -nocomplain -types f [file join $common_common *]] {
					file copy -force $entry "./src"
				}
			}
		}
		"psxl_cortexr52" -
		"psxl_cortexa78" -
		"psx_cortexr52" -
		"psx_cortexa78" {
			foreach entry [glob -nocomplain -types f [file join $common_client *]] {
				file copy -force $entry "./src"
			}
			foreach entry [glob -nocomplain -types f [file join $versal_net_client *]] {
				file copy -force $entry "./src"
			}
			foreach entry [glob -nocomplain -types f [file join $versal_net_common *]] {
				file copy -force $entry "./src"
			}
			foreach entry [glob -nocomplain -types f [file join $common_common *]] {
				file copy -force $entry "./src"
			}
		}

		"microblaze" {
			if { [llength $is_versal] > 0 } {
				foreach entry [glob -nocomplain -types f [file join $common_client *]] {
					file copy -force $entry "./src"
				}
				foreach entry [glob -nocomplain -types f [file join $versal_client *]] {
					file copy -force $entry "./src"
				}
				foreach entry [glob -nocomplain -types f [file join $versal_common *]] {
					file copy -force $entry "./src"
				}
				foreach entry [glob -nocomplain -types f [file join $common_common *]] {
					file copy -force $entry "./src"
				}
				} else {
					foreach entry [glob -nocomplain -types f [file join $common_client *]] {
						file copy -force $entry "./src"
					}
					foreach entry [glob -nocomplain -types f [file join $versal_net_client *]] {
						file copy -force $entry "./src"
					}
				foreach entry [glob -nocomplain -types f [file join $versal_net_common *]] {
					file copy -force $entry "./src"
					}
				foreach entry [glob -nocomplain -types f [file join $common_common *]] {
					file copy -force $entry "./src"
					}
				}
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
	set readconfig [common::get_property $cips_prop $pspmcCell]
	set readconfig [lindex $readconfig 0]
	return $readconfig
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
	set part_list {vc1502 vc1702 vc1802 vc1902 vc2602 vc2802 ve1752 ve2202 ve2302 ve2602 ve2802
				   vh1522 vh1542 vh1582 vh1742 vh1782 vp1102 vp1202 vp1402 vp1502 vp1552 vp1702 vp1802
				   vp2502 vp2802 vm1102 vm1302 vm1402 vm1502 vm1802 vm2202 vm2302 vm2502 vm2902 vn3716}
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
	if {((($sem_cfrscan_en > 0) || ($sem_npiscan_en > 0)) && ($sem_print_flag == true))} {
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

		#Enable Additional PPKs for M50 or later designs
		set PartName [string range $part 2 [expr {[string first "-" $part] - 1}]]
		if {[lsearch -exact $part_list $PartName] == -1} {
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

	# Get user_modules count set by user, by default it is 0
	set value [common::get_property CONFIG.user_modules_count $libhandle]
	puts $file_handle "\n/* Number of User Modules */"
	puts $file_handle [format %s%d%s "#define XPAR_MAX_USER_MODULES " [expr $value]  "U"]

	# Get plm_version_user_defined set by user, by default it is 0
	set value [common::get_property CONFIG.plm_version_user_defined $libhandle]
	puts $file_handle "\n/* plm version user defined */"
	puts $file_handle [format %s%d%s "#define XPAR_PLM_VERSION_USER_DEFINED " [expr $value]  "U"]

	puts $file_handle "\n"
	close $file_handle
}
