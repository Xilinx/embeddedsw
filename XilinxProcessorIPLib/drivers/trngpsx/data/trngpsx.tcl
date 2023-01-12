###############################################################################
# Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  kpt  01/12/23 First release
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

	if {$proc_type != "psxl_cortexa78" && $proc_type != "psx_cortexa78"} {
		::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XTrngpsx" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

		::hsi::utils::define_zynq_config_file $drv_handle "xtrngpsx_g.c" "XTrngpsx"  "DEVICE_ID" "C_S_AXI_BASEADDR"

		::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XTrngpsx" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
	} else {
		a78_trng_generate_config "xtrngpsx_g.c"
	}
}

#---------------------------------------------
# Generating TRNG configuration for A78 proc
#---------------------------------------------
proc a78_trng_generate_config {file_name} {
	set drv_string "XTrngpsx"
	set pki_trng_baseaddress "0x20400051000"
	set pki_trng_offset "0x200"
	set pki_num_insts 8
	set total_num_instances 8
	set cfgfilename [file join "src" $file_name]
	set config_file [open $cfgfilename w]

	set pmc_trng [hsi::get_cells -hier -filter {IP_NAME == "psx_pmc_trng" || IP_NAME == "psxl_pmc_trng"}]
	set is_pmc_trng_en [string match {*pmc_trng*} $pmc_trng]

	::hsi::utils::write_c_header $config_file "Driver configuration"
	puts $config_file "#include \"xparameters.h\""
	puts $config_file "#include \"[string tolower $drv_string].h\""

	if {$is_pmc_trng_en == 1} {
		set total_num_instances [expr $pki_num_insts + 1]
	}

	# Start generating the  Config table
	puts $config_file "\n/*"
	puts $config_file "* The configuration table for devices"
	puts $config_file "*/\n"
	puts $config_file [format "%s_Config %s_ConfigTable\[%s\] =" $drv_string $drv_string $total_num_instances]
	puts $config_file "\{"
	set comma ""
	if {$is_pmc_trng_en == 1} {
		puts $config_file "\t\t{"
		puts $config_file [format "\t\t\t%sU," [common::get_property CONFIG.DEVICE_ID [hsi::get_cells -hier -filter {IP_NAME == psx_pmc_trng || IP_NAME == psxl_pmc_trng}]]]
		puts $config_file [format "\t\t\t%sU" [common::get_property CONFIG.C_S_AXI_BASEADDR [hsi::get_cells -hier -filter {IP_NAME == psx_pmc_trng || IP_NAME == psxl_pmc_trng}]]]
		puts $config_file "\t\t},"
	}
	#Harcording the PKI configuration for A78
	for {set instance 0} {$instance < $pki_num_insts} {incr instance} {
		puts $config_file "\t\t{"
		puts $config_file [format "\t\t\t%sU," [expr $instance + $is_pmc_trng_en]]
		puts $config_file [format "\t\t\t0x%xU" [expr $pki_trng_baseaddress + [expr {$instance * $pki_trng_offset}]]]
		puts $config_file "\t\t},"
	}
	puts $config_file "\};"
	close $config_file
	set file_handle [hsi::utils::open_include_file "xparameters.h"]
	puts $file_handle [format "#define XPAR_%s_NUM_INSTANCES %s" [string toupper $drv_string] $total_num_instances]
	close $file_handle
}
