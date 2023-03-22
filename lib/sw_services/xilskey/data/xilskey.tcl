###############################################################################
# Copyright (c) 2013 - 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a rpo  04/25/13 Initial Release
# 3.00  vns  30/07/15 Added macro in xparameters.h based on the
#                     processor
# 6.4   vns  02/27/18 Added support for virtex and virtex ultrascale plus
# 6.7	psl  03/12/19 Disabled compilation of code not required for zynqmp
# 6.8   psl  06/26/19 Added support for user to add IDCODE, IR_length, SLR Nos,
#                     device series for different devices.
# 7.1   kpt  05/11/21 Added support for PUF Fuse programming as general purpose
#                     data
##############################################################################

#---------------------------------------------
# skey_drc
#---------------------------------------------
proc skey_drc {libhandle} {

}

proc skey_open_include_file {file_name} {
	set filename [file join "../../include/" $file_name]
	if {[file exists $filename]} {
		set config_inc [open $filename a]
	} else {
		set config_inc [open $filename a]
		::hsi::utils::write_c_header $config_inc "User Supported IDCODES"
	}
	return $config_inc
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
#	This procedure builds the libxilskey.a library
#-------
proc execs_generate {libhandle} {

}

proc xgen_opts_file {libhandle} {
	set proc_instance [hsi::get_sw_processor];
	set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $proc_instance] ]
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]

	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set override_sysmon_cfg [common::get_property CONFIG.override_sysmon_cfg $libhandle]
	set access_secure_crit_efuse [common::get_property CONFIG.access_secure_crit_efuse $libhandle]
	set access_user_efuse [common::get_property CONFIG.access_user_efuse $libhandle]
	set access_key_manage_efuse [common::get_property CONFIG.access_key_manage_efuse $libhandle]
	set access_puf_user_efuse [common::get_property CONFIG.use_puf_hd_as_user_efuse $libhandle]

	set file_handle [::hsi::utils::open_include_file "xparameters.h"]

	puts $file_handle "\n/* Xilinx processor macro for Secure Library (Xilskey) */ "
	if {$proc_type == "ps7_cortexa9" || $proc_type == "psu_cortexa53" || $proc_type == "psu_cortexr5" || $proc_type == "psu_pmu"} {
		puts $file_handle "\n#define XPAR_XSK_ARM_PLATFORM 1"
	} elseif {$proc_type == "microblaze"} {
		puts $file_handle "\n#define XPAR_XSK_MICROBLAZE_PLATFORM 1"
		set mb_type [common::get_property CONFIG.C_FAMILY $hw_proc_handle]
		if {$mb_type == "kintexuplus" || $mb_type == "virtexuplus" || $mb_type == "zynquplus"} {
			puts $file_handle "\n#define XPAR_XSK_MICROBLAZE_ULTRA_PLUS 1"
		}
		if {$mb_type == "kintexu" || $mb_type == "virtexu"} {
			puts $file_handle "\n#define XPAR_XSK_MICROBLAZE_ULTRA 1"
		}
	} else {
		error "ERROR: Xilskey library is not supported for this device";
		return;
	}
	if {$override_sysmon_cfg == true} {
		puts $file_handle "\n#define XSK_OVERRIDE_SYSMON_CFG \n"
	}

	if {$proc_type == "psu_pmu" || $proc_type == "psu_cortexa53" || $proc_type == "psu_cortexr5"} {
		file delete -force ./src/xilskey_epl.c
		file delete -force ./src/xilskey_eps.c
		file delete -force ./src/xilskey_epshw.h
		file delete -force ./src/xilskey_js.h
		file delete -force ./src/xilskey_jscmd.c
		file delete -force ./src/xilskey_jscmd.h
		file delete -force ./src/xilskey_jslib.c
		file delete -force ./src/xilskey_jslib.h
		file delete -force ./src/xilskey_jtag.h
		file delete -force ./src/xilskey_bbram.c
		file delete -force ./src/include/xilskey_epl.h
		file delete -force ./src/include/xilskey_eps.h

		if {$access_secure_crit_efuse == true} {
			puts $file_handle "\n#define XSK_ACCESS_SECURE_CRITICAL_EFUSE \n"
		}

		if {$access_user_efuse == true} {
			puts $file_handle "\n#define XSK_ACCESS_USER_EFUSE \n"
		}

		if {$access_key_manage_efuse == true} {
			puts $file_handle "\n#define XSK_ACCESS_KEY_MANAGE_EFUSE \n"
		}
		if {$access_puf_user_efuse == true} {
			puts $file_handle "\n#define XSK_ACCESS_PUF_USER_EFUSE \n"
			file delete -force ./src/xilskey_eps_zynqmp_puf.c
			file delete -force ./src/include/xilskey_eps_zynqmp_puf.h
		}
        }

	if {$proc_type == "psu_pmu"} {
		file delete -force ./src/xilskey_bbramps_zynqmp.c
                file delete -force ./src/include/xilskey_bbram.h
	}

	puts $file_handle ""
	close $file_handle


#---------
# Support to add user IDCODEs
#---------
	if {$proc_type != "psu_pmu" && $proc_type != "psu_cortexa53" && $proc_type != "psu_cortexr5"} {

		set device_series  [common::get_property CONFIG.device_series $libhandle]
		set device_id [common::get_property CONFIG.device_id $libhandle]
		set device_irlen [common::get_property CONFIG.device_irlen $libhandle]
		set device_numslr [common::get_property CONFIG.device_numslr $libhandle]
		set device_masterslr [common::get_property CONFIG.device_masterslr $libhandle]

		set conffile  [skey_open_include_file "xilskey_config.h"]

		puts $conffile "\#ifndef XILSKEY_IDCODES_H"
		puts $conffile "\#define XILSKEY_IDCODES_H"
		puts $conffile ""
		puts $conffile "\#ifdef __cplusplus"
		puts $conffile "extern \"c\" \x7b\ "
		puts $conffile "\#endif"
		puts $conffile ""
		puts $conffile "\#include \"xilskey_utils.h\""
		puts $conffile ""

		if { $device_id != 0 } {
			if {$device_id != "0x0ba00477" && $device_id != "0x03822093" && $device_id != "0x03931093" && $device_id != "0x03842093" && $device_id != "0x04A62093" && $device_id != "0x04b31093" && $device_id != "0x04B51093"  && $device_id != "0x0484A093"} {
				puts $conffile "#define XSK_USER_DEVICE_SERIES	$device_series"
				puts $conffile "#define XSK_USER_DEVICE_ID	$device_id"
				puts $conffile "#define XSK_USER_DEVICE_IRLEN	$device_irlen"
				puts $conffile "#define XSK_USER_DEVICE_NUMSLR	$device_numslr"
				puts $conffile "#define XSK_USER_DEVICE_MASTER_SLR $device_masterslr"
			} else {
				puts stderr "ERROR: Device IDCODE already exist by Default."
				puts $conffile "#define XSK_USER_DEVICE_SERIES  0"
				puts $conffile "#define XSK_USER_DEVICE_ID      0"
				puts $conffile "#define XSK_USER_DEVICE_IRLEN   0"
				puts $conffile "#define XSK_USER_DEVICE_NUMSLR  0"
				 puts $conffile "#define XSK_USER_DEVICE_MASTER_SLR 0"
			}
		} else {
			puts $conffile "/* No Value specified, assigning default values */"
			puts $conffile "#define XSK_USER_DEVICE_SERIES	$device_series"
			puts $conffile "#define XSK_USER_DEVICE_ID	$device_id"
			puts $conffile "#define XSK_USER_DEVICE_IRLEN	$device_irlen"
			puts $conffile "#define XSK_USER_DEVICE_NUMSLR	$device_numslr"
			puts $conffile "#define XSK_USER_DEVICE_MASTER_SLR $device_masterslr"
		}


		puts $conffile ""
		puts $conffile "\#ifdef __cplusplus"
		puts $conffile "extern \"c\" \x7b\ "
		puts $conffile "\#endif"
		puts $conffile ""
		puts $conffile "\#endif /* XILSKEY_IDCODES_H */"

		close $conffile
	}
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
