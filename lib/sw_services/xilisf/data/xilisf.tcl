###############################################################################
# Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 2.01a sdm  06/17/10 Updated to support axi_spi
# 2.02a sdm  09/24/10 updated to use Tcl commands instead of unix commands
# 2.03a sdm  04/17/11 Updated to support axi_quad_spi
# 2.04a sdm  08/01/11 Added new parameter for Numonyx quad flash devices.
# 3.00a srt  06/20/12 Updated to support interfaces SPI PS and QSPI PS.
#		      Added support to SST flash.
# 3.02a srt  05/13/13 Removed compiler errors when not selecting proper
#		      interface for Zynq. (CR 716451)
# 5.4   sk   08/07/15 Updated to support QSPIPSU interface.
#                     Updated to support SPIPS interface in ZynqMP.
# 5.13	sk   02/11/19 Added OSPI interface support.
# 5.14  akm  09/09/19 Added message regarding deprecation of Xilisf.
#
##############################################################################

#---------------------------------------------
# ISF_drc - check system configuration and make sure
# all components to run ISF are available.
#---------------------------------------------

proc isf_drc {libhandle} {
	puts "Running DRC for XilIsf library... \n"

	# find the list of xps or opb spi cores
	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]

	set spi_periphs_list [get_spi_periphs $processor]

	if { [llength $spi_periphs_list] == 0 } {
		set cpuname [common::get_property NAME $processor]
		error  "ERROR: No SPI core is addressable from processor $cpuname. \
			XilIsf library requires a SPI Core \n"
		return
	}
}

proc get_spi_periphs {processor} {
	set periphs_list [::hsi::utils::get_proc_slave_periphs $processor]
	set spi_periphs_list {}

	global spi_periphs_name_list
	set spi_periphs_name_list {}

	foreach periph $periphs_list {
		set periphname [common::get_property IP_NAME $periph]
		if {$periphname == "xps_spi"
			|| $periphname == "opb_spi"
			|| $periphname == "xps_insystem_flash"
			|| $periphname == "axi_spi"
			|| $periphname == "axi_quad_spi"
			|| $periphname == "ps7_spi"
			|| $periphname == "ps7_qspi"
			|| $periphname == "psu_qspi"
			|| $periphname == "psu_spi"
			|| $periphname == "psu_ospi"
			|| $periphname == "psv_pmc_qspi"
			|| $periphname == "psv_spi"
			|| $periphname == "psv_pmc_ospi"} {
			lappend spi_periphs_list $periph
			lappend spi_periphs_name_list $periphname
		}
	}

	return $spi_periphs_list
}

#--------
# Check the following h/w requirements for XilIsf:
#--------
proc isf_hw_drc {libhandle spi_list} {
}


# SPI hw requirements
proc isf_spi_hw_drc {libhandle spi} {

}


proc generate {libhandle} {

	puts "WARNING: Xilisf library is being deprecated from 2020.1 release. It will be made obsolete in 2021.1 release."
}


#-------
# post_generate: called after generate called on all libraries
#-------
proc post_generate {libhandle} {
	xgen_opts_file $libhandle

}

#-------
# execs_generate: called after BSP's, libraries and drivers have been compiled
#	This procedure builds the libisf.a library
#-------
proc execs_generate {libhandle} {

}


proc xgen_opts_file {libhandle} {

	# Open xparameters.h file
	set file_handle [::hsi::utils::open_include_file "xparameters.h"]

	# -----------------------------
	# Generate Flash options
	# -----------------------------
	puts $file_handle "/* Xilinx EDK In-system and Serial Flash Library (XilIsf) User Settings */"
	set serial_flash_family [common::get_property CONFIG.serial_flash_family $libhandle]
	puts $file_handle "\#define XPAR_XISF_FLASH_FAMILY	$serial_flash_family"

	set serial_flash_interface [common::get_property CONFIG.serial_flash_interface $libhandle]
	set ifaceselect 0
	set ps7qspi 0
	global spi_periphs_name_list
	foreach periph $spi_periphs_name_list {
		if {$periph == "axi_spi" || $periph == "axi_quad_spi"
			|| $periph == "opb_spi"
			|| $periph == "xps_insystem_flash"
			|| $periph == "xps_spi"} {
			if {$serial_flash_interface == 1} {
				puts $file_handle "\#define XPAR_XISF_INTERFACE_AXISPI	1"
				set ifaceselect 1
			}
		} elseif {($periph == "ps7_spi" || $periph == "psu_spi" || $periph == "psv_spi") &&
			$serial_flash_interface == 2} {
			puts $file_handle "\#define XPAR_XISF_INTERFACE_PSSPI	1"
			set ifaceselect 1
		} elseif {$periph == "ps7_qspi" &&
			$serial_flash_interface == 3} {
			puts $file_handle "\#define XPAR_XISF_INTERFACE_PSQSPI	1"
			set ifaceselect 1
		} elseif {$periph == "psu_qspi" && $serial_flash_interface == 3} {
			puts $file_handle "\#define XPAR_XISF_INTERFACE_QSPIPSU	1"
			set ifaceselect 1
		} elseif {$periph == "psv_pmc_qspi" && $serial_flash_interface == 3} {
			puts $file_handle "\#define XPAR_XISF_INTERFACE_QSPIPSU	1"
			set ifaceselect 1
		} elseif {$periph == "psu_ospi" && $serial_flash_interface == 4} {
			puts $file_handle "\#define XPAR_XISF_INTERFACE_OSPIPSV	1"
			set ifaceselect 1
		} elseif {$periph == "psv_pmc_ospi" && $serial_flash_interface == 4} {
			puts $file_handle "\#define XPAR_XISF_INTERFACE_OSPIPSV	1"
			set ifaceselect 1
		} elseif {$periph == "ps7_qspi"} {
			set ps7qspi 1
		}
	}

	if {$ps7qspi == 1 && $ifaceselect == 0} {
		puts "WARN: Improper Flash Interface from BSP Settings!!!! Defaulting on 'ps7_qspi' interface"
		puts $file_handle "\#define XPAR_XISF_INTERFACE_PSQSPI	1"
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
