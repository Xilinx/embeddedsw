###############################################################################
# Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00 mjr  02/03/15  Created
# 2.3  ms   04/11/17  Modified tcl file to add U suffix for all macros
#                     of ipipsu in xparameters.h
# 2.5  sd   04/01/19  Added support for the the no buffer ipi
#      sd   09/03/19  Add support for versal ip name
# 2.8  nsk  12/14/20  Modified the tcl to not to gnerate the instance
#                     names.
# 2.8  nsk  01/19/21  Updated to use IP_NAME for IPIs mapped.
# 2.10  sd  07/02/21  Updated tcl logic to read BASEADDRESS
#                         HIGHADDRESS parameters of IP blocks to
#                         support SSIT devices. Now get_param_value
#                         proc would be used instead of get_property
#                         proc to read those parameters.
# 2.11  sd  01/07/22 Updated tcl to support microblaze
# 2.12  sd  02/24/22 Added support for VERSAL NET
# 2.13  sd  11/03/22 Add hack to support cores
# 2.15  sd  11/03/23 Add support for name change of ACP
# 2.15  ap  12/11/23 Added support for microblaze risc-v
##############################################################################

#uses "xillib.tcl"

proc check_platform { } {
	set cortexa53proc [hsi::get_cells -hier -filter "IP_NAME==psu_cortexa53"]
	if {[llength $cortexa53proc] > 0} {
		set iszynqmp 1
	} else {
		set iszynqmp 0
	}
	return $iszynqmp
}

proc ipi_format_hexmask {bitpos} {
	return [format "0x%08X" [expr 1<<$bitpos]]
}
proc ipi_define_xpar {inst param} {
	set uSuffix "U"
	set name [string range $param 2 end]
	set param_value [::hsi::utils::get_param_value [hsi::get_cells -hier $inst] $param]
	if { [string compare $name "BUFFER_INDEX"] == 0} {
		if { [string compare $param_value "NIL"] == 0} {
			set param_value 0xFFFF
		}
	}
	if { [string compare $name "BIT_POSITION"] == 0} {
		set name "BIT_MASK"
		set param_value [ipi_format_hexmask $param_value]
	}
	return  [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $inst] $name $param_value]
}

proc ipi_find_cpu {ipi_list param hw_proc} {
	set proc_ipi_slave ""
	foreach ipi $ipi_list {
		set param_name [string range $param [string length "CONFIG."] [string length $param]]
		set param_value [common::get_property $param [hsi::get_cells -hier $ipi]]
		set ip_name [common::get_property IP_NAME [hsi::get_cells -hier $hw_proc]]
		set index [string index $hw_proc end]
		if {[string match -nocase $ip_name "microblaze"] || [string match -nocase $ip_name "microblaze_riscv"]} {
			set is_pl [common::get_property IS_PL [hsi::get_cells -hier $hw_proc]]
			if {$is_pl == 1} {
				if { [string match -nocase "*$param_value*" S_AXI_GP4] || [string match -nocase "*$param_value*" PL_AXI_LPD] } {
					lappend proc_ipi_slave $ipi
				}
			}
		}
		if {[string match -nocase $ip_name "psv_cortexr5"] || [string match -nocase $ip_name "psxl_cortexr52"] || [string match -nocase $ip_name "psx_cortexr52"]} {
			set ip_name "${ip_name}_$index"
		}
		if { [string match -nocase  "*A78*"  "*$param_value*" ] } {
			set param_value "A78"
		}
		if { [string match -nocase "*$param_value*" $ip_name] } {
			lappend proc_ipi_slave $ipi
		}
	}
	return $proc_ipi_slave
}

#Generate Config file with data structures describing the HW
proc ipi_generate_config {drv_handle file_name iszynqmp} {

	#Driver Prefix String
	set drv_string "XIpiPsu"

	#The current processor
	set sw_proc_handle [::hsi::get_sw_processor]
	set hw_proc_handle [::hsi::get_cells -hier [common::get_property hw_instance $sw_proc_handle]]

	# List of all IPIs on SoC
	set ipi_list [get_cells -hier -filter { IP_NAME == "psu_ipi" || IP_NAME == "psv_ipi" || IP_NAME == "psxl_ipi" || IP_NAME == "psx_ipi"}]
	
	# List of IPIs owned by this processor

	if { $iszynqmp == 1 } {
		set proc_ipi_list [lsearch -all -inline [get_property SLAVES $hw_proc_handle] psu_ipi_*]
	} else {
		set proc_ipi_list [ipi_find_cpu $ipi_list CONFIG.C_CPU_NAME $hw_proc_handle]
	}

	set cfgfilename [file join "src" $file_name]
	set config_file [open $cfgfilename w]

	# Common Header
	::hsi::utils::write_c_header $config_file "Driver configuration"
	puts $config_file "#include \"xparameters.h\""
	puts $config_file "#include \"[string tolower $drv_string].h\""

	# Start generating the  Config table
	puts $config_file "\n/*"
	puts $config_file "* The configuration table for devices"
	puts $config_file "*/\n"
	set num_insts [::hsi::utils::get_driver_param_name $drv_string "NUM_INSTANCES"]
	puts $config_file [format "%s_Config %s_ConfigTable\[%s\] =" $drv_string $drv_string $num_insts]
	puts $config_file "\{"

	set comma ""
	foreach ipi_inst $proc_ipi_list {
		puts $config_file $comma
		puts $config_file "\t\{"
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $ipi_inst] "DEVICE_ID,"]
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $ipi_inst] "S_AXI_BASEADDR,"]
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $ipi_inst] "BIT_MASK,"]
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $ipi_inst] "BUFFER_INDEX,"]
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $ipi_inst] "INT_ID,"]

		# Target Count
		puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $drv_string] "NUM_TARGETS,"]

		# Start generating Target Table with list if all IPI slots on SoC
		puts $config_file "\t\t\{"

		set comma ""
		foreach ipi_inst $ipi_list {
			puts $config_file $comma
			puts $config_file "\t\t\t\{"
			puts $config_file [format "\t\t\t\tXPAR_%s_%s" [string toupper $ipi_inst] "BIT_MASK,"]
			puts $config_file [format "\t\t\t\tXPAR_%s_%s" [string toupper $ipi_inst] "BUFFER_INDEX"]
			puts -nonewline $config_file "\t\t\t\}"
			set comma ","
		}
		puts $config_file "\n\t\t\}"
		puts -nonewline $config_file "\t\}"
		set comma ",\n"
	}
	puts $config_file "\n\};"

	close $config_file
}
proc ipi_generate_params {file_name iszynqmp} {
	#Driver Prefix String
	set drv_string "XIpiPsu"
	set uSuffix "U"

	# open the xparameters.h file
	set file_handle [::hsi::utils::open_include_file $file_name]

	# List of all IPIs on SoC
	set ipi_list [get_cells -hier -filter { IP_NAME == "psu_ipi" || IP_NAME == "psv_ipi" || IP_NAME == "psxl_ipi" || IP_NAME == "psx_ipi"}]

	#List of all processors on SoC
	set proc_list [get_cells -hier -filter { IP_TYPE == "PROCESSOR" }]

	#The current processor
	set sw_proc_handle [::hsi::get_sw_processor]
	set hw_proc_handle [::hsi::get_cells -hier [common::get_property hw_instance $sw_proc_handle]]


	# List of IPIs owned by this processor
	if { $iszynqmp == 1 } {
		set proc_ipi_list [lsearch -all -inline [get_property SLAVES $hw_proc_handle] psu_ipi_*]
	} else {
		set proc_ipi_list [ipi_find_cpu $ipi_list CONFIG.C_CPU_NAME $hw_proc_handle]
	}

	#Total number of IPIs assigned to this proc
	puts $file_handle [format "#define  XPAR_XIPIPSU_NUM_INSTANCES  %s$uSuffix" [llength $proc_ipi_list]]
	puts $file_handle ""

	# Generate all params for IPIs owned by this proc
	#Idx is used to track DEVICE_ID as we loop through the list
	set idx 0

	foreach ipi_inst $proc_ipi_list {
		puts $file_handle [format "/* Parameter definitions for peripheral %s */" $ipi_inst]
		puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $ipi_inst] "DEVICE_ID" $idx]
		puts $file_handle [ipi_define_xpar $ipi_inst C_S_AXI_BASEADDR]
		puts $file_handle [ipi_define_xpar $ipi_inst C_BIT_POSITION]
		puts $file_handle [ipi_define_xpar $ipi_inst C_BUFFER_INDEX]
		puts $file_handle [ipi_define_xpar $ipi_inst C_INT_ID]
		puts $file_handle ""
		incr idx
	}

	set idx 0
	foreach ipi_inst $proc_ipi_list {
		puts $file_handle [format "/* Canonical definitions for peripheral %s */" $ipi_inst]
		puts $file_handle [format "#define  XPAR_%s_%s_%s	XPAR_%s_%s" [string toupper $drv_string] $idx "DEVICE_ID" [string toupper $ipi_inst] "DEVICE_ID"]
		puts $file_handle [format "#define  XPAR_%s_%s_%s	XPAR_%s_%s" [string toupper $drv_string] $idx "BASE_ADDRESS" [string toupper $ipi_inst] "S_AXI_BASEADDR"]
		puts $file_handle [format "#define  XPAR_%s_%s_%s	XPAR_%s_%s" [string toupper $drv_string] $idx "BIT_MASK" [string toupper $ipi_inst] "BIT_MASK"]
		puts $file_handle [format "#define  XPAR_%s_%s_%s	XPAR_%s_%s" [string toupper $drv_string] $idx "BUFFER_INDEX" [string toupper $ipi_inst] "BUFFER_INDEX"]
		puts $file_handle [format "#define  XPAR_%s_%s_%s	XPAR_%s_%s" [string toupper $drv_string] $idx "INT_ID" [string toupper $ipi_inst] "INT_ID"]
		puts $file_handle ""
		incr idx
	}

	#Total number of IPIs assigned to this proc
	puts $file_handle [format "#define  XPAR_XIPIPSU_NUM_TARGETS  %s$uSuffix" [llength $ipi_list]]
	puts $file_handle ""

	foreach ipi_inst $ipi_list {
		puts $file_handle [ipi_define_xpar $ipi_inst C_BIT_POSITION]
		puts $file_handle [ipi_define_xpar $ipi_inst C_BUFFER_INDEX]
	}
	# Generate Canonical definitions to map IPI instance -> Processors
	puts $file_handle "/* Target List for referring to processor IPI Targets */"
	puts $file_handle ""
	set a72_nr 0
	set r5_nr 0
	foreach proc $proc_list {
		# List of IPIs owned by this processor
		set ipname [common::get_property IP_NAME [hsi::get_cells -hier $proc]]
		if {[string match -nocase $ipname "psv_cortexa72"] || [string match -nocase $ipname "psu_cortexa53"] || [string match -nocase $ipname "psxl_cortexa78"] || [string match -nocase $ipname "psx_cortexa78"]} {
			set ipname "${ipname}_${a72_nr}"
			incr a72_nr
		} elseif {[string match -nocase $ipname "psv_cortexr5"] || [string match -nocase $ipname "psu_cortexr5"] || [string match -nocase $ipname "psxl_cortexr52"] || [string match -nocase $ipname "psx_cortexr52"]} {
			set ipname "${ipname}_${r5_nr}"
			incr r5_nr
		} elseif {[string match -nocase $ipname "psv_pmc"] || [string match -nocase $ipname "psv_psm"] || [string match -nocase $ipname "psu_pmu"] || [string match -nocase $ipname "psxl_pmc"] || [string match -nocase $ipname "psxl_psm"] || [string match -nocase $ipname "psx_pmc"] || [string match -nocase $ipname "psx_psm"]} {
			set ipname "${ipname}_0"
		} else {
			set ipname "${proc}"
		}
		if { $iszynqmp == 1 } {
			set proc_slave_list [lsearch -all -inline [get_property SLAVES $proc] psu_ipi_*]
		} else {
			set proc_slave_list [ipi_find_cpu $ipi_list CONFIG.C_CPU_NAME $proc]
		}

		set idx 0
		foreach ipi_slave $proc_slave_list {
			puts $file_handle [format "#define  XPAR_XIPIPS_TARGET_%s_CH%s_MASK  XPAR_%s_BIT_MASK" [string toupper $ipname] $idx [string toupper $ipi_slave]]
			puts $file_handle [format "#define  XPAR_XIPIPS_TARGET_%s_CH%s_INDEX  %s$uSuffix" [string toupper $ipname] $idx [lsearch $ipi_list $ipi_slave]]
			incr idx
		}
		puts $file_handle ""
	}

	# close the include file
	close $file_handle

}


#This is called by HSI while generating the driver
proc generate {drv_handle} {
	set iszynqmp [check_platform]
	ipi_generate_params "xparameters.h" $iszynqmp
	ipi_generate_config $drv_handle "xipipsu_g.c" $iszynqmp
}
