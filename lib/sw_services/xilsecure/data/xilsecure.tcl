###############################################################################
# Copyright (c) 2013 - 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a ba   06/01/15 Initial Release
# 1.2   vns  08/23/16 Added support for SHA2 by adding .a files
# 2.0   vns  11/28/16 Added support for PMU
# 2.0   srm  02/16/18 Updated to pick up latest freertos port 10.0
# 3.1   vns  04/13/18 Added user configurable macro secure environment
# 4.0   vns  03/20/19 Added support for versal
# 4.1   vns  08/02/19 Added support for a72 and r5 processors of Versal
# 4.3   rpo  07/08/20 Added support to access xsecure init files only for
#                     psv_pmc and psu_pmc processor
# 4.5   bsv  04/01/21 Added support for XSECURE_TPM_ENABLE macro
#       kal  04/21/21 Added server side support for A72/R5 processors for
#                     Versal
#       har  05/17/21 Added support for non-secure access of Xilsecure IPIs
# 4.9   bm   07/06/22 Refactor versal and versal_net code
#       am   07/24/22 Added support for a78 and r52 processors of VersalNet
#
##############################################################################

#---------------------------------------------
# secure_drc
#---------------------------------------------
proc secure_drc {libhandle} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set compiler [common::get_property CONFIG.compiler $proc_instance]
	set mode [common::get_property CONFIG.mode $libhandle]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os_type [hsi::get_os];
	set common "src/common/"
	set zynqmp "src/zynqmp/"
	set versal "src/versal/"
	set versal_net "src/versal_net/"
	set common_all "$common/all"
	set versal_common "$common/versal_common"
	set versal_client_dir "$versal/client"
	set versal_server_dir "$versal/server"
	set versal_common_dir "$versal/common"

	foreach entry [glob -nocomplain -types f [file join $common_all *]] {
			file copy -force $entry "./src"
	}

	if {$proc_type == "psu_cortexa53" ||
		$proc_type == "psu_cortexr5" || $proc_type == "psu_pmu"} {
			foreach entry [glob -nocomplain -types f [file join $zynqmp *]] {
				file copy -force $entry "./src"
			}
	} elseif {$proc_type == "psu_pmc" || $proc_type == "psu_cortexa72" ||
				$proc_type == "psv_pmc" || $proc_type == "psv_cortexa72" ||
				$proc_type == "psv_cortexr5" || $proc_type == "microblaze" ||
				$proc_type == "psxl_pmc" || $proc_type == "psxl_cortexa78" ||
				$proc_type == "psxl_cortexr52" || $proc_type == "psx_pmc" ||
				$proc_type == "psx_cortexa78" || $proc_type == "psx_cortexr52"} {
		if {$proc_type == "microblaze" && $mode == "server"} {
			error "ERROR: XilSecure server library is not supported for microblaze";
			return;
		}
		if {$proc_type == "psu_pmc" || $proc_type == "psv_pmc"  || $proc_type == "psxl_pmc" || $proc_type == "psx_pmc"|| $mode == "server"} {
			foreach entry [glob -nocomplain -types f [file join "$versal_common/server" *]] {
				file copy -force $entry "./src"
			}
			foreach entry [glob -nocomplain -types f [file join "$versal_common/common" *]] {
				file copy -force $entry "./src"
			}
			if {$proc_type == "psxl_pmc" || $proc_type == "psxl_cortexa78" ||
			    $proc_type == "psxl_cortexr52" || $proc_type == "psx_pmc" ||
			    $proc_type == "psx_cortexa78" || $proc_type == "psx_cortexr52"} {
				foreach entry [glob -nocomplain -types f [file join "$versal_net/server" *]] {
					file copy -force $entry "./src"
				}
				foreach entry [glob -nocomplain -types f [file join "$versal_net/common" *]] {
					file copy -force $entry "./src"
				}
			} else {
				foreach entry [glob -nocomplain -types f [file join "$versal/server" *]] {
					file copy -force $entry "./src"
				}
				foreach entry [glob -nocomplain -types f [file join "$versal/common" *]] {
					file copy -force $entry "./src"
				}
			}

			if {$mode == "server"} {
				if {$proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
				    $proc_type == "psv_cortexr5" || $proc_type == "psxl_cortexa78" ||
				    $proc_type == "psxl_cortexr52" || $proc_type == "psx_cortexa78" ||
				    $proc_type == "psx_cortexr52"} {
					file delete -force ./src/xsecure_rsa_ipihandler.c
					file delete -force ./src/xsecure_rsa_ipihandler.h
					file delete -force ./src/xsecure_elliptic_ipihandler.c
					file delete -force ./src/xsecure_elliptic_ipihandler.h
					file delete -force ./src/xsecure_sha_ipihandler.c
					file delete -force ./src/xsecure_sha_ipihandler.h
					file delete -force ./src/xsecure_aes_ipihandler.c
					file delete -force ./src/xsecure_aes_ipihandler.h
					file delete -force ./src/xsecure_kat_ipihandler.c
					file delete -force ./src/xsecure_kat_ipihandler.h
					file delete -force ./src/xsecure_cmd.c
					file delete -force ./src/xsecure_cmd.h
					file delete -force ./src/xsecure_defs.h
					if {$proc_type == "psxl_cortexa78" ||
					    $proc_type == "psxl_cortexr52" || $proc_type == "psx_cortexa78" ||
					    $proc_type == "psx_cortexr52"} {
						file delete -force ./src/xsecure_trng_ipihandler.c
						file delete -force ./src/xsecure_trng_ipihandler.h
					}
				}
			}

			 if {[string compare -nocase $compiler "mb-gcc"] == 0} {
				if {$proc_type == "psu_pmc" || $proc_type == "psv_pmc"} {
					file delete -force ./src/libxilsecure_a72_64.a
					file delete -force ./src/libxilsecure_r5.a
				} elseif {$proc_type == "psxl_pmc" || $proc_type == "psx_pmc"} {
					file delete -force ./src/libxilsecure_a78_64.a
					file delete -force ./src/libxilsecure_r52.a
				}
				file rename -force ./src/libxilsecure_pmc.a ./src/libxilsecure.a
	                } elseif {[string compare -nocase $compiler "aarch64-none-elf-gcc"] == 0} {
				file delete -force ./src/libxilsecure_pmc.a
				if {$proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72"} {
					file rename -force ./src/libxilsecure_a72_64.a ./src/libxilsecure.a
					file delete -force ./src/libxilsecure_r5.a
					file delete -force ./src/libxilsecure_a78_64.a
				} elseif {$proc_type == "psxl_cortexa78" || $proc_type == "psx_cortexa78"} {
					file rename -force ./src/libxilsecure_a78_64.a ./src/libxilsecure.a
					file delete -force ./src/libxilsecure_r52.a
					file delete -force ./src/libxilsecure_a72_64.a
				}
			} elseif {[string compare -nocase $compiler "armr5-none-eabi-gcc"] == 0} {
				file delete -force ./src/libxilsecure_pmc.a
				if {$proc_type == "psu_cortexr5" || $proc_type == "psv_cortexr5"} {
					file rename -force ./src/libxilsecure_r5.a ./src/libxilsecure.a
					file delete -force ./src/libxilsecure_a72_64.a
					file delete -force ./src/libxilsecure_r52.a
				} elseif {$proc_type == "psxl_cortexr52" || $proc_type == "psx_cortexr52"} {
					file rename -force ./src/libxilsecure_r52.a ./src/libxilsecure.a
					file delete -force ./src/libxilsecure_a78_64.a
					file delete -force ./src/libxilsecure_r5.a
				}
			}
		} elseif {$proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
			$proc_type == "psv_cortexr5" || $proc_type == "psxl_cortexa78" ||
			$proc_type == "psxl_cortexr52" || $proc_type == "psx_cortexa78" ||
			$proc_type == "psx_cortexr52" || $proc_type == "microblaze"} {
			set librarylist [hsi::get_libs -filter "NAME==xilmailbox"];
			if { [llength $librarylist] == 0 } {
				error "This library requires xilmailbox library in the Board Support Package.";
			}
			foreach entry [glob -nocomplain -types f [file join "$versal_common/client" *]] {
				file copy -force $entry "./src"
			}
			foreach entry [glob -nocomplain -types f [file join "$versal_common/common" *]] {
				file copy -force $entry "./src"
			}
			if {$proc_type == "psxl_cortexa78" || $proc_type == "psxl_cortexr52" ||
			    $proc_type == "psx_cortexa78" || $proc_type == "psx_cortexr52"} {
				foreach entry [glob -nocomplain -types f [file join "$versal_net/client" *]] {
					file copy -force $entry "./src"
				}
				foreach entry [glob -nocomplain -types f [file join "$versal_net/common" *]] {
					file copy -force $entry "./src"
				}
			} else {
				foreach entry [glob -nocomplain -types f [file join "$versal/client" *]] {
					file copy -force $entry "./src"
				}
				foreach entry [glob -nocomplain -types f [file join "$versal/common" *]] {
					file copy -force $entry "./src"
				}
			}
			file delete -force ./src/xsecure_utils.c
			file delete -force ./src/xsecure_utils.h
			file delete -force ./src/xsecure_rsa.c
			file delete -force ./src/xsecure_rsa.h
		}

		if {$proc_type != "psv_pmc" &&  $proc_type != "psu_pmc" && $proc_type != "psxl_pmc" && $proc_type != "psx_pmc"} {
			file delete -force ./src/xsecure_init.c
			file delete -force ./src/xsecure_init.h
		}

	} else {
		error "ERROR: XilSecure library is supported only for PMU, CortexA53, CortexR5, CortexA72, CortexA78, CortexR52, psv_pmc and psx_pmc processors.";
		return;
	}
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
#	This procedure builds the libxilsecure.a library
#-------
proc execs_generate {libhandle} {

}

proc xgen_opts_file {libhandle} {

	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

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

	# Get secure_environment value set by user, by default it is FALSE
	set value [common::get_property CONFIG.secure_environment $libhandle]
	if {$value == true} {
		# Open xparameters.h file
		set file_handle [hsi::utils::open_include_file "xparameters.h"]

		puts $file_handle "\n/* Xilinx Secure library User Settings */"
		puts $file_handle "#define XSECURE_TRUSTED_ENVIRONMENT \n"

		close $file_handle
	}
	# Get tpm_support environment value set by user, by default it is FALSE
	set value [common::get_property CONFIG.tpm_support $libhandle]
	if {$value == true} {
		# Open xparameters.h file
		set file_handle [hsi::utils::open_include_file "xparameters.h"]

		puts $file_handle "\n/* TPM Settings */"
		puts $file_handle "#define XSECURE_TPM_ENABLE\n"

		close $file_handle
	}
	# Get nonsecure_ipi_access value set by user, by default it is FALSE
	set value [common::get_property CONFIG.nonsecure_ipi_access $libhandle]
	if {$value == true} {
		#Open xparameters.h file
		set file_handle [hsi::utils::open_include_file "xparameters.h"]

		puts $file_handle "\n/* Xilinx Secure library User Settings */"
		puts $file_handle "#define XSECURE_NONSECURE_IPI_ACCESS\n"

		close $file_handle
	}
	# Get cache_disable value set by user, by default it is FALSE
	set value [common::get_property CONFIG.cache_disable $libhandle]
	if {$value == true} {
		#Open xparameters.h file
		if {$proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
			$proc_type == "psv_cortexr5" || $proc_type == "microblaze"} {
			set file_handle [hsi::utils::open_include_file "xparameters.h"]

			puts $file_handle "\n/* Xilinx Secure library User Settings */"
			puts $file_handle "#define XSECURE_CACHE_DISABLE\n"

			close $file_handle
		}
	}

	if {$proc_type == "psxl_pmc"} {
		set file_handle [hsi::utils::open_include_file "xparameters.h"]
		puts $file_handle "\n/* Xilinx Secure library TRNG User Settings */"
		set value [common::get_property CONFIG.seedlife $libhandle]
		puts $file_handle "\n/* TRNG seed life */"
		puts $file_handle [format %s%d%s "#define XSECURE_TRNG_USER_CFG_SEED_LIFE " [expr $value]  "U"]
		set value [common::get_property CONFIG.dlen $libhandle]
		puts $file_handle "\n/* TRNG DF length */"
		puts $file_handle [format %s%d%s "#define XSECURE_TRNG_USER_CFG_DF_LENGTH " [expr $value]  "U"]
		set value [common::get_property CONFIG.adaptproptestcutoff $libhandle]
		puts $file_handle "\n/* TRNG adaptive prop test cutoff value*/"
		puts $file_handle [format %s%d%s "#define XSECURE_TRNG_ADAPT_TEST_CUTOFF " [expr $value]  "U"]
		set value [common::get_property CONFIG.repcounttestcutoff $libhandle]
		puts $file_handle "\n/* TRNG repetitive prop test cutoff value*/"
		puts $file_handle [format %s%d%s "#define XSECURE_TRNG_REP_TEST_CUTOFF " [expr $value]  "U"]
	}
}
