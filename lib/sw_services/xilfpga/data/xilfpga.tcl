###############################################################################
# Copyright (c) 2016 - 20222 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   Nava  06/08/16 First release
# 1.1   Nava  11/16/16 Added PL power-up sequence.
# 2.0   Nava  01/10/17  Added Encrypted bitstream loading support.
# 2.0   Nava  02/16/17 Added Authenticated bitstream loading support.
# 2.1   Nava  05/06/17 Correct the check logic issues in
#                      XFpga_PL_BitStream_Load()
#                      to avoid the unwanted blocking conditions.
# 3.0   Nava  05/12/17 Added PL configuration registers readback support.
# 4.0   Nava  02/08/18 Added Authenticated and Encypted Bitstream loading support.
# 4.1	Nava  03/27/18 For Secure Bitstream loading to avoid the Security violations
#                      Need to Re-validate the User Crypto flags with the Image
#                      Crypto operation by using the internal memory.To Fix this
#                      added a new API XFpga_ReValidateCryptoFlags().
# 4.1	Nava  04/16/18  Added partial bitstream loading support.
# 4.2	Nava  05/30/18 Refactor the xilfpga library to support
#                      different PL programming Interfaces.
# 4.2	adk   07/24/18 Added proper error message if xilsecure is not enabled
#			in the bsp.
# 5.0   Nava  05/11/18  Added full bitstream loading support for versal Platform.
# 5.0	sne   03/27/19 Fixed Misra-C violations.
# 5.0   Nava  03/29/19  Removed Versal platform related changes.As per the new
#                       design, the Bitstream loading for versal platform is
#                       done by PLM based on the CDO's data exists in the PDI
#                       images. So there is no need of xilfpga API's for versal
#                       platform to configure the PL.
# 5.2   Nava  12/05/19  Added Versal platform support.
# 5.2   Nava  01/02/20  Added XFPGA_SECURE_READBACK_MODE flag to support secure
#                       PL configuration data readback.
# 5.2   Nava  03/14/20  Added Bitstream loading support by using IPI services
#                       for ZynqMP platform.
# 5.3   Nava  06/16/20  Modified the date format from dd/mm to mm/dd.
# 5.3   Nava  09/16/20  Added user configurable Enable/Disable Options for
#                       readback operations
# 6.2   Nava  01/10/22  Adds XFpga_GetVersion() and XFpga_GetFeatureList() API's
#                       to provide the access to the xilfpga library to get the
#                       xilfpga version and supported feature list info.
# 6.2  Nava   01/19/22  Added build time flag to skip eFUSE checks.
# 6.4  Nava   01/23/22  Added Versalnet support.
#
##############################################################################

#---------------------------------------------
# fpga_drc
#---------------------------------------------
proc fpga_drc {libhandle} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]

	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

}

proc xfpga_open_include_file {file_name} {
    set filename [file join "../../include/" $file_name]
    if {[file exists $filename]} {
        set config_inc [open $filename a]
    } else {
        set config_inc [open $filename a]
        ::hsi::utils::write_c_header $config_inc "MFS Parameters"
   }
    return $config_inc
}

proc generate {lib_handle} {

    file copy "src/xilfpga.h"  "../../include/xilfpga.h"

    set conffile  [xfpga_open_include_file "xfpga_config.h"]
    set zynqmp "src/interface/zynqmp/"
    set versal "src/interface/versal/"
    set interface "src/interface/"
    # check processor type
    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
    set cortexa53proc [hsi::get_cells -hier -filter "IP_NAME==psu_cortexa53"]
    set cortexa72proc [hsi::get_cells -hier -filter "IP_NAME==psv_cortexa72"]
    set cortexr5proc  [hsi::get_cells -hier -filter "IP_NAME==psv_cortexr5"]
    set cortexa78proc [hsi::get_cells -hier -filter "IP_NAME==psx_cortexa78"]
    set cortexr52proc  [hsi::get_cells -hier -filter "IP_NAME==psx_cortexr52"]
    if {[llength $cortexa53proc] > 0} {
	set iszynqmp 1
    } elseif {([llength $cortexa72proc] > 0) || ([llength $cortexr5proc] > 0) || ([llength $cortexa78proc] > 0) || ([llength $cortexr52proc] > 0)} {
	set iszynqmp 0
	set isversal 1
    } else {
	set iszynqmp 0
	set isversal 0
    }

    if { $iszynqmp == 1} {
	set value  [common::get_property CONFIG.secure_environment $lib_handle]
	if {$proc_type != "psu_pmu" && $value == true} {
		set librarylist [hsi::get_libs -filter "NAME==xilmailbox"];
		if { [llength $librarylist] == 0 } {
			error "This library requires xilmailbox library in the Board Support Package.";
		}
		set def_flags [common::get_property APP_LINKER_FLAGS [hsi::current_sw_design]]
		set new_flags "-Wl,--start-group,-lxilfpga,-lxil,-lxilmailbox,-lgcc,-lc,--end-group $def_flags"
		set_property -name APP_LINKER_FLAGS -value $new_flags -objects [current_sw_design]
		file copy -force ./src/interface/zynqmp/xilfpga_ipi_pcap.c ./src/xilfpga_ipi_pcap.c
	} else {
		if {$proc_type != "psu_pmu"} {
			puts "\nTo support secure environment bitstream loading, you must enable secure_environment in xilfpga."
		}
		set librarylist [hsi::get_libs -filter "NAME==xilsecure"];
		if { [llength $librarylist] == 0 } {
			error "This library requires xilsecure library in the Board Support Package.";
		}
		set def_flags [common::get_property APP_LINKER_FLAGS [hsi::current_sw_design]]
		set new_flags "-Wl,--start-group,-lxilfpga,-lxil,-lxilsecure,-lgcc,-lc,--end-group $def_flags"
		set_property -name APP_LINKER_FLAGS -value $new_flags -objects [current_sw_design]
		file copy -force ./src/interface/zynqmp/xilfpga_pcap.c ./src/xilfpga_pcap.c
		file copy -force ./src/interface/zynqmp/xilfpga_pcap.h ./src/xilfpga_pcap.h
	}
	file copy -force ./src/interface/zynqmp/xilfpga_pcap_common.h ./src/xilfpga_pcap_common.h
    } elseif {$isversal == 1} {
	set librarylist [hsi::get_libs -filter "NAME==xilmailbox"];
	if { [llength $librarylist] == 0 } {
	    error "This library requires xilmailbox library in the Board Support Package.";
	}
	set def_flags [common::get_property APP_LINKER_FLAGS [hsi::current_sw_design]]
	set new_flags "-Wl,--start-group,-lxilfpga,-lxil,-lxilmailbox,-lgcc,-lc,--end-group $def_flags"
	set_property -name APP_LINKER_FLAGS -value $new_flags -objects [current_sw_design]

	foreach entry [glob -nocomplain -types f [file join $versal *]] {
            file copy -force $entry "./src"
        }
    } else {
		error "This library supports Only ZyqnMP/Versal platform."
    }

    puts $conffile "#ifndef _XFPGA_CONFIG_H"
    puts $conffile "#define _XFPGA_CONFIG_H"

    if { $iszynqmp == 1} {
	set value  [common::get_property CONFIG.secure_environment $lib_handle]
	if {$proc_type != "psu_pmu" && $value == true} {
		puts $conffile "#include <xilfpga_pcap_common.h>"
		puts $conffile "#define XFPGA_SECURE_IPI_MODE_EN"
	} else {
		puts $conffile "#include <xilfpga_pcap_common.h>"
		puts $conffile "#include <xilfpga_pcap.h>"
	}
    } else {
	puts $conffile "#include <xilfpga_versal.h>"
    }

    set value  [common::get_property CONFIG.ocm_address $lib_handle]
    puts  $conffile "#define XFPGA_OCM_ADDRESS ${value}U"
    set value  [common::get_property CONFIG.base_address $lib_handle]
    puts  $conffile "#define XFPGA_BASE_ADDRESS ${value}U"
    set value  [common::get_property CONFIG.secure_mode $lib_handle]

    if {$value == true} {
	puts $conffile "#define XFPGA_SECURE_MODE"
    }

    set value  [common::get_property CONFIG.secure_readback $lib_handle]
    if {$value == true} {
	puts $conffile "#define XFPGA_SECURE_READBACK_MODE"
    }

   set value  [common::get_property CONFIG.reg_readback_en $lib_handle]
   if {$value == true} {
       puts $conffile "#define XFPGA_READ_CONFIG_REG"
   }

   set value  [common::get_property CONFIG.data_readback_en $lib_handle]
   if {$value == true} {
       puts $conffile "#define XFPGA_READ_CONFIG_DATA"
   }

   set value  [common::get_property CONFIG.get_version_info_en $lib_handle]
   if {$value == true} {
       puts $conffile "#define XFPGA_GET_VERSION_INFO"
   }

   set value  [common::get_property CONFIG.get_feature_list_en $lib_handle]
   if {$value == true} {
       puts $conffile "#define XFPGA_GET_FEATURE_LIST"
   }

   set value  [common::get_property CONFIG.skip_efuse_check_en $lib_handle]
   if {$value == true} {
	puts $conffile "#define XFPGA_SKIP_EFUSE_CHECK"
   }

   set value  [common::get_property CONFIG.debug_mode $lib_handle]

   if {$value == true} {
        puts $conffile "#define XFPGA_DEBUG	(1U)"
    } else {
	puts $conffile "#define XFPGA_DEBUG     (0U)"
    }
    puts $conffile "#endif"
    close $conffile
}
