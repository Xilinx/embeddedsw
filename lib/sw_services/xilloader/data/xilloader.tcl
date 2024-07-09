###############################################################################
# Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
##############################################################################
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  kc   09/22/18 Initial Release
# 1.01  dd   01/09/23 Added client Interface
#
##############################################################################

#---------------------------------------------
# loader_drc
#---------------------------------------------
proc loader_drc {libhandle} {
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set compiler [common::get_property CONFIG.compiler $proc_instance]
	set mode [common::get_property CONFIG.xloader_mode $libhandle]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os_type [hsi::get_os];
	set versal_net "src/versal_net/"
	set versal "src/versal/"
	set versal_aiepg2 "src/versal_aiepg2/"
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
				PSXL Cortexa78, PSXL Cortexr52, PSX Cortexa78 and PSX Cortexr52.";
	}

	if {$mode == "server"} {
		if {$proc_type != "psu_pmc" && $proc_type != "psv_pmc" && $proc_type != "psxl_pmc" &&
				$proc_type != "psx_pmc"} {
				error "ERROR: XilLoader library is not supported for selected processor in\
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
	foreach entry [glob -nocomplain -types f [file join ./src/ *]] {
		if {$entry != "./src/Makefile"} {
			file delete -force $entry
		}
	}

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
			set is_versal [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa72" ||
					IP_NAME=="psv_cortexa72" || IP_NAME=="psv_cortexr5"}]
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
	file delete -force $versal_aiepg2
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

proc xgen_opts_file {libhandle} {

	# Copy the include files to the include directory
	set srcdir src
	set dstdir [file join .. .. include]

	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

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

	# Get cache_disable value set by user, by default it is FALSE
	set value [common::get_property CONFIG.xloader_cache_disable $libhandle]
	if {$value == true} {
		#Open xparameters.h file
		if {$proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
                $proc_type == "psv_cortexr5" || $proc_type == "microblaze" ||
                $proc_type == "psxl_cortexa78" || $proc_type == "psxl_cortexr52" ||
                $proc_type == "psx_cortexa78" || $proc_type == "psx_cortexr52"} {
				set file_handle [hsi::utils::open_include_file "xparameters.h"]

				puts $file_handle "\n/* Xilloader library User Settings */"
				puts $file_handle "#define XLOADER_CACHE_DISABLE\n"

				close $file_handle
		}
	}
}
