###############################################################################
# Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0  mmd  05/06/19 Initial Release
# 3.0  kpt  08/25/22 Changed user configurable parameter names
# 3.1  skg  12/07/22 Added a user configuration parameter
# 3.3  vss  12/21/23 Added microblaze support for versalnet
#      vss  02/23/24 Added configuration parameters for eFuse write
#
##############################################################################

#---------------------------------------------
# nvm_drc
#---------------------------------------------
proc nvm_drc {libhandle} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set compiler [common::get_property CONFIG.compiler $proc_instance]
	set mode [common::get_property CONFIG.xnvm_mode $libhandle]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os_type [hsi::get_os];

	set versal_dir "./src/versal_gen/versal"
	set versal_net_dir    "./src/versal_gen/versal_net"
	set versal_family_dir "./src/versal_gen/common"

	set versal_client_dir "$versal_dir/client"
	set versal_server_dir "$versal_dir/server"
	set versal_common_dir "$versal_dir/common"

	set versal_net_client_dir "$versal_net_dir/client"
	set versal_net_server_dir "$versal_net_dir/server"
	set versal_net_common_dir "$versal_net_dir/common"

	set family_common_client_dir "$versal_family_dir/client"
	set family_common_server_dir "$versal_family_dir/server"
	set family_common_dir "$versal_family_dir/common"

	if {$proc_type != "psu_pmc" && $proc_type != "psu_cortexa72" && \
	    $proc_type != "psv_pmc" && $proc_type != "psv_cortexa72" && \
	    $proc_type != "psv_cortexr5" && $proc_type != "microblaze" && \
	    $proc_type != "psxl_pmc" && $proc_type != "psxl_cortexa78" && \
	    $proc_type != "psxl_cortexr52" && $proc_type != "psx_cortexa78" && \
            $proc_type != "psx_cortexr52" && $proc_type != "psx_pmc"} {
		error "ERROR: XilNvm library is supported only for PSU PMC, \
		      PSU Cortexa72, PSV PMC, PSV Cortexa72, PSX PMC, PSX Cortexa78 and PSX Cortexr52.";
	}

	if {$mode == "server"} {
		if {$proc_type == "microblaze" || $proc_type == "psxl_cortexa78" || $proc_type == "psx_cortexa78" ||
				$proc_type == "psxl_cortexr52" || $proc_type == "psx_cortexr52"} {
			error "ERROR: XilNvm library is not supported for selected $proc_type processor in $mode mode.";
			return;
		}
	}

	if {$mode == "client" &&  ($proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
                $proc_type == "psv_cortexr5" || $proc_type == "microblaze" ||
                $proc_type == "psxl_cortexa78" || $proc_type == "psxl_cortexr52" ||
                $proc_type == "psx_cortexa78" || $proc_type == "psx_cortexr52")} {
                set librarylist [hsi::get_libs -filter "NAME==xilmailbox"];
                if { [llength $librarylist] == 0 } {
                        error "This library requires xilmailbox library in the Board Support Package.";
                }
        }

	switch $proc_type {
		"psu_pmc" -
		"psv_pmc" {
			copy_files_to_src $family_common_server_dir
			copy_files_to_src $versal_server_dir
			copy_files_to_src $versal_common_dir
			copy_files_to_src $versal_family_dir
		}

		"psxl_pmc" -
		"psx_pmc" {
			copy_files_to_src $family_common_server_dir
			copy_files_to_src $versal_net_server_dir
			copy_files_to_src $versal_net_common_dir
			copy_files_to_src $versal_family_dir
		}

		"psu_cortexa72" -
		"psv_cortexa72" -
		"psv_cortexr5" {
			if {$mode == "server"} {
				copy_files_to_src $family_common_server_dir
				copy_files_to_src $versal_server_dir
				copy_files_to_src $versal_common_dir
				copy_files_to_src $versal_family_dir
			} else {
				copy_files_to_src $family_common_client_dir
				copy_files_to_src $versal_client_dir
				copy_files_to_src $versal_common_dir
				copy_files_to_src $versal_family_dir
			}
		}

		"psxl_cortexr52" -
		"psxl_cortexa78" -
		"psx_cortexr52" -
		"psx_cortexa78" {
			if {$mode == "server"} {
				copy_files_to_src $family_common_server_dir
				copy_files_to_src $versal_net_server_dir
				copy_files_to_src $versal_net_common_dir
				copy_files_to_src $versal_family_dir
			} else {
				copy_files_to_src $family_common_client_dir
				copy_files_to_src $versal_net_client_dir
				copy_files_to_src $versal_net_common_dir
				copy_files_to_src $versal_family_dir
			}
		}

		"microblaze" {
			set is_versal_net [hsi::get_cells -hier -filter {IP_NAME=="psxl_cortexr52" || IP_NAME=="psx_cortexr52" ||
				IP_NAME=="psxl_cortexa78" || IP_NAME=="psx_cortexa78"}]
			if {[llength $is_versal_net] > 0} {
					copy_files_to_src $family_common_client_dir
					copy_files_to_src $versal_net_client_dir
					copy_files_to_src $versal_net_common_dir
					copy_files_to_src $versal_family_dir
			} else {
					copy_files_to_src $family_common_client_dir
					copy_files_to_src $versal_client_dir
					copy_files_to_src $versal_common_dir
					copy_files_to_src $versal_family_dir
			}
		}
	}

	if {$mode == "server"} {
		if {$proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
			$proc_type == "psv_cortexr5" || $proc_type == "psxl_cortexa78" ||
			$proc_type == "psx_cortexa78" || $proc_type == "psxl_cortexr52" ||
			$proc_type == "psx_cortexr52"} {
			file delete -force ./src/xnvm_bbram_common_cdohandler.c
			file delete -force ./src/xnvm_bbram_common_cdohandler.h
			file delete -force ./src/xnvm_efuse_ipihandler.c
			file delete -force ./src/xnvm_efuse_ipihandler.h
			file delete -force ./src/xnvm_cmd.c
			file delete -force ./src/xnvm_cmd.h
			file delete -force ./src/xnvm_init.c
			file delete -force ./src/xnvm_init.h
		}
		if {$proc_type == "psxl_cortexa78" || $proc_type == "psx_cortexa78" ||
			$proc_type == "psxl_cortexr52" || $proc_type == "psx_cortexr52"} {
			file delete -force ./src/xnvm_bbram_cdohandler.c
			file delete -force ./src/xnvm_bbram_cdohandler.h
		}

	}
}

proc generate {libhandle} {

}

proc copy_files_to_src {dir_path} {
        foreach entry [glob -nocomplain [file join $dir_path *]] {
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
#	This procedure builds the libxilnvm.a library
#-------
proc execs_generate {libhandle} {

}

proc xgen_opts_file {libhandle} {

	# Copy the include files to the include directory
	set srcdir src
	set dstdir [file join .. .. include]
	set access_puf_efuse [common::get_property CONFIG.xnvm_use_puf_hd_as_user_efuse $libhandle]
	set add_en_ppks [common::get_property CONFIG.xnvm_en_add_ppks $libhandle]
	set access_en_write_sec_crit_efuse [common::get_property CONFIG.xnvm_en_write_sec_crit_efuse $libhandle]
	set access_en_write_user_efuse [common::get_property CONFIG.xnvm_en_write_user_efuse $libhandle]
	set access_en_write_key_management_efuse [common::get_property CONFIG.xnvm_en_write_key_management_efuse $libhandle]
	set file_handle [::hsi::utils::open_include_file "xparameters.h"]
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

	if {$access_puf_efuse == true} {
		puts $file_handle "\n#define XNVM_ACCESS_PUF_USER_DATA \n"
	}

	if { [info commands ::hsi::get_current_part] != ""} {
		#Get part name from the design
		set part [::hsi::get_current_part]

		#Enable Additional PPKs for M50 or later designs
		set PartName [string range $part 2 [expr {[string first "-" $part] - 1}]]
		if {[lsearch -exact $part_list $PartName] == -1} {
			set IsAddPpkEn true
		}	}

	if {$add_en_ppks == true || $IsAddPpkEn ==  true} {
		puts $file_handle "\n/* Enable provisioning support for additional PPKs */"
		puts $file_handle "\n#define XNVM_EN_ADD_PPKS \n"
	}

	if {$access_en_write_sec_crit_efuse == true} {
		puts $file_handle "\n/* Enable write access for security critical eFuses */"
		puts $file_handle "\n#define XNVM_WRITE_SECURITY_CRITICAL_EFUSE \n"
	}

	if {$access_en_write_user_efuse == true} {
		puts $file_handle "\n/* Enable write access for user eFuses */"
		puts $file_handle "\n#define XNVM_WRITE_USER_EFUSE \n"
	}

	if {$access_en_write_key_management_efuse == true} {
		puts $file_handle "\n/* Enable write access for key manage eFuses */"
		puts $file_handle "\n#define XNVM_WRITE_KEY_MANAGEMENT_EFUSE \n"
	}

	# Get cache_disable value set by user, by default it is FALSE
	set value [common::get_property CONFIG.xnvm_cache_disable $libhandle]
	if {$value == true} {
		#Open xparameters.h file
		if {$proc_type == "psu_cortexa72" || $proc_type == "psv_cortexa72" ||
                        $proc_type == "psv_cortexr5" || $proc_type == "microblaze" ||
                        $proc_type == "psxl_cortexa78" || $proc_type == "psxl_cortexr52" ||
                        $proc_type == "psx_cortexa78" || $proc_type == "psx_cortexr52"} {
			set file_handle [hsi::utils::open_include_file "xparameters.h"]
			puts $file_handle "#define XNVM_CACHE_DISABLE\n"
		}
	}

	close $file_handle

}
