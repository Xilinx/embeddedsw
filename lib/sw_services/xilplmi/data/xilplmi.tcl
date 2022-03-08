###############################################################################
# Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  kc   09/22/18 Initial Release
# 1.10  ssc  03/05/22 Added configurable options
##############################################################################

#---------------------------------------------
# plmi_drc
#---------------------------------------------
proc plmi_drc {libhandle} {
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set versal_net "src/versal_net/"
	if {$proc_type == "psxl_pmc"} {
		foreach entry [glob -nocomplain -types f [file join ./src/ *]] {
			file delete -force $entry
		}
		foreach entry [glob -nocomplain -types f [file join $versal_net *]] {
			file copy -force $entry "./src"
		}
	}
	file delete -force $versal_net
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
	puts $file_handle "\n/* Debug level option */"
	if {$value == "level0"} {
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

	puts $file_handle "\n"
	close $file_handle
}
