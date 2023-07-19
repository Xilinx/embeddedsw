###############################################################################
# Copyright (c) 2015 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
##############################################################################

namespace eval ::pmufw {

proc get_template { file_name } {
	set fp [open $file_name r]
	set tmpl_data [read $fp]
	close $fp
	return $tmpl_data
}

#Lookout and load the template from this script's directory
set cfg_template [get_template [file join [file dirname [info script]] cfg_data.tpl]]

proc is_rpu_lockstep {} {

	# Get the property that describes sub-system in design
	set zusp [get_cells -hier -filter "IP_NAME == zynq_ultra_ps_e"]
	#if we don't find a valid ZU+ IP
	if { [llength $zusp] == 0 } {
		#default to split mode
		return 0
	}
	set subsys_str [get_property CONFIG.PSU__PROTECTION__SUBSYSTEMS [lindex $zusp 0]]
	set found_rpu0 0
	set found_rpu1 0

	#Split it to get the sub-ssytem list
	set subsys_list [split $subsys_str "|"]

	#Split each element in list into Name:Masters
	foreach subsys $subsys_list {
			if { [lsearch [split [lindex [split $subsys ":"] 1] ";"] RPU0] >= 0 } {
				set found_rpu0 1
			}
			if { [lsearch [split [lindex [split $subsys ":"] 1] ";"] RPU1] >= 0} {
				set found_rpu1 1
			}
	}
	# If RPU0 is found AND RPU1 is not found, its Lock-Step
	# All other cases are considered Split mode
	if { $found_rpu0 == 1 && $found_rpu1 == 0 } {
		set ret_val 1
	} else {
		set ret_val 0
	}

	return $ret_val
}

set master_map [dict create]
dict set master_map psu_cortexa53_0 { label psu_cortexa53_0 name APU }
dict set master_map psu_cortexr5_0 { label psu_cortexr5_0 name RPU0 }
dict set master_map psu_cortexr5_1 { label psu_cortexr5_1 name RPU1 }

proc is_master_defined { master } {
	# Get the property that describes sub-system in design
	set zusp [get_cells -hier -filter "IP_NAME == zynq_ultra_ps_e"]

	#if we don't find a valid ZU+ IP
	if { [llength $zusp] == 0 } {
		return 0
	}
	set subsys_str [get_property CONFIG.PSU__PROTECTION__SUBSYSTEMS [lindex $zusp 0]]
	set found 0

	#Split it to get the sub-ssytem list
	set subsys_list [split $subsys_str "|"]

	set proc_name [dict get [dict get $pmufw::master_map $master] name]
	foreach subsys $subsys_list {
		if { [lsearch [split [lindex [split $subsys ":"] 1] ";"] $proc_name] >= 0 } {
			set found 1
			break
		}
	}

	return $found
}

proc is_ipi_defined { master } {
	#Get the slave list for this master
	set slave_list [get_mem_ranges -of_objects [get_cells -hier $master]];
	#Find the first IPI slave in the list
	set ipi [lsearch -inline $slave_list psu_ipi_*];
	if { $ipi != "" } {
		set ret_val 1
	} else {
		set ret_val 0
	}
	return $ret_val
}

global master_list
set master_list ""

set master_temp_list {psu_cortexa53_0 psu_cortexr5_0 psu_cortexr5_1}

foreach master $pmufw::master_temp_list {
	if {[is_master_defined $master] && [is_ipi_defined $master]} {
		append master_list "$master "
	}
}

if { $master_list == "" } {
	set master_list $master_temp_list
}

#=============================================================================#
# Get the IPI mask for a given master
#=============================================================================#
proc get_ipi_mask { master } {
	#Get the slave list for this master
	set slave_list [get_mem_ranges -of_objects [get_cells -hier $master]];
	#Find the first IPI slave in the list
	set ipi [lsearch -inline $slave_list psu_ipi_* ];
	if { $ipi != "" } {
		#Get the bit position property for the IPI instance
		set bit_pos [get_property CONFIG.C_BIT_POSITION -object [get_cells -hier $ipi]];
		#Convert the bit position into MASK and return it
		return [format 0x%08X [expr 1<<$bit_pos]];
	} else {
		return 0;
	}
}

#=============================================================================#
# Get the Permission mask for a given slave node
#=============================================================================#
proc get_slave_perm_mask { slave } {
	 #List of Masters in the System
	 global master_list
	 set perm_mask 0x00000000
	 foreach master $pmufw::master_list {
		 #Get the slave list for this master
		 set slave_list [get_mem_ranges -of_objects [get_cells $master]];
		 #Search for the save in list
		 set slave_index [lsearch $slave_list $slave]
		 #if found, OR the master IPI mask to PERM mask
		 if { $slave_index >=0 } {
			set perm_mask [expr $perm_mask|[get_ipi_mask $master]]
		 }
	 }
	 #Return the mask in hex
	 return [format 0x%08X $perm_mask];
}


proc get_tcm_r5_perm_mask { r5_proc tcm_bank } {
	set perm_mask 0x00000000
	switch -glob $r5_proc {
		"psu_cortexr5_0" {
			# In case of LS, all TCMs are accessible to R5_0
			if { [is_rpu_lockstep]} {
				set perm_mask [get_ipi_mask $r5_proc]
			} else {
			# In split, r5_0 only related banks
				if {$tcm_bank in [list psu_r5_0_atcm_global psu_r5_0_btcm_global]} {
					set perm_mask [get_ipi_mask $r5_proc]
				}
			}
		}
		"psu_cortexr5_1" {
			if { [is_rpu_lockstep]} {
				set perm_mask 0x00000000
			} else {
				if {$tcm_bank in [list psu_r5_1_atcm_global psu_r5_1_btcm_global]} {
					set perm_mask [get_ipi_mask $r5_proc]
				}
			}
		}
		default {
			set perm_mask 0x00000000
		}
	}
	return $perm_mask
}



#=============================================================================#
# Get the Permission mask for a given TCM node
#=============================================================================#
proc get_tcm_perm_mask { tcm } {
	set perm_mask 0x00000000
	foreach master $pmufw::master_list {

		switch -glob $master {
		#For R5s, TCMs are always accessible
		"psu_cortexr5_*" {
			set perm_mask [expr $perm_mask|[get_tcm_r5_perm_mask $master $tcm]]
		}
		#for others it same as any other slave
		default {
			#Get the slave list for this master
			set slave_list [get_mem_ranges -of_objects [get_cells -hier $master]];
			#Search for the slave in list
			set slave_index [lsearch $slave_list $tcm]
			#if found, OR the master IPI mask to PERM mask
			if { $slave_index >=0 } {
				set perm_mask [expr $perm_mask|[get_ipi_mask $master]]
			}
		}
		}
	 }
	#Return the mask in hex
	return [format 0x%08X $perm_mask];
}


set ocm_map [dict create]
dict set ocm_map psu_ocm_0 { label OCM_BANK_0 base 0xFFFC0000 high 0xFFFCFFFF }
dict set ocm_map psu_ocm_1 { label OCM_BANK_1 base 0xFFFD0000 high 0xFFFDFFFF }
dict set ocm_map psu_ocm_2 { label OCM_BANK_2 base 0xFFFE0000 high 0xFFFEFFFF }
dict set ocm_map psu_ocm_3 { label OCM_BANK_3 base 0xFFFF0000 high 0xFFFFFFFF }

#=============================================================================#
# Get the Permission mask for a given OCM node
#=============================================================================#
proc get_ocm_perm_mask { ocm } {
	set perm_mask 0x00000000
	#OCM island mem map
	#get the island_base and island_hi vals for ocm_label
	set island_base [dict get [dict get $pmufw::ocm_map $ocm] base]
	set island_high [dict get [dict get $pmufw::ocm_map $ocm] high]

	foreach master $pmufw::master_list {
		set plist [get_mem_ranges -of_objects [get_cells -hier $master] psu_ocm_ram_0]
		if { [llength $plist] > 0} {
			foreach ocm_instance $plist {
				set base_val [get_property -object $ocm_instance -name BASE_VALUE]
				set high_val [get_property -object $ocm_instance -name HIGH_VALUE]
				# if island vals ffall in the instance range, then return the mask
				if { [expr ($island_base >= $base_val) && ($island_base <= $high_val)] || \
					[expr ($island_high >= $base_val) && ($island_high <= $high_val)]} {
					set perm_mask [expr $perm_mask|[get_ipi_mask $master]]
					break;
				}
			}
		}
	}
	#Return the mask in hex
	return [format 0x%08X $perm_mask];
}




#=============================================================================#
# Get the Permission mask for a given MEMORY node
#=============================================================================#
proc get_mem_perm_mask { mem } {
	set perm_mask 0x00000000

	switch -glob $mem {
		"psu_ddr" {
			set perm_mask [expr [get_slave_perm_mask psu_ddr_?]|[get_slave_perm_mask psu_r5_ddr_?]]
		}
		"psu_ocm_?" {
			set perm_mask [get_ocm_perm_mask $mem]
		}
		"psu_r5_*tcm_global" {
			set perm_mask [get_tcm_perm_mask $mem]
		}
		default {
			set perm_mask "0x00"
		}
	}
	#Return the mask in hex
	return [format 0x%08X $perm_mask];
}



proc convert_ipi_mask_to_txt { ipi_mask } {
	set macro_list {}
	foreach master $pmufw::master_list {
		if { [expr (($ipi_mask & [get_ipi_mask $master]) != 0)] &&
		     [is_ipi_defined $master] == 1 } {
			lappend macro_list [get_ipi_mask_txt $master]
		}
	}
	#Return the ORed macro list
	if { [llength $macro_list] >0 } {
		return [join $macro_list "| "];
	} else {
		return "0U";
	}
}


#=============================================================================#
# Return the Macro text for IPI mask of a Master
#=============================================================================#
proc get_ipi_mask_txt { master } {

	if { ([lsearch $pmufw::master_list $master] != -1) && [is_ipi_defined $master] } {
		return "PM_CONFIG_IPI_[string toupper $master]_MASK"
	} else {
		return ""
	}
}

#=============================================================================#
# Return the Macro text for IPI masks of a All Masters
#=============================================================================#
proc get_all_masters_mask_txt { } {
	set macro_list {}
	foreach master $pmufw::master_list {
		if { [is_ipi_defined $master] == 1 } {
			lappend macro_list [get_ipi_mask_txt $master]
		}
	}
	return [join $macro_list " | "];
}


#=============================================================================#
# Return the Macro text for IPI masks of a all Masters except given master
#=============================================================================#
proc get_all_other_masters_mask_txt { master_name } {
	set macro_list {}
	global rpu0_as_power_management_master
	global rpu1_as_power_management_master
	global apu_as_power_management_master

	foreach master $pmufw::master_list {
		if { $master !=  $master_name && [is_ipi_defined $master] == 1 } {
			if { ("psu_cortexa53_0" == $master_name) } {
				if { ($rpu0_as_power_management_master == false) &&
				     ("psu_cortexr5_0" == $master) } {
					continue
				}
				if { ($rpu1_as_power_management_master == false) &&
				     (0 == [is_rpu_lockstep]) &&
				     ("psu_cortexr5_1" == $master) } {
					continue
				}
			} elseif { ("psu_cortexr5_0" == $master_name) } {
				if { ($apu_as_power_management_master == false) &&
				     ("psu_cortexa53_0" == $master) } {
					continue
				}
				if { ($rpu1_as_power_management_master == false) &&
				     (0 == [is_rpu_lockstep]) &&
				     ("psu_cortexr5_1" == $master) } {
					continue
				}
			} elseif { (0 == [is_rpu_lockstep]) && ("psu_cortexr5_1" == $master_name) } {
				if { ($apu_as_power_management_master == false) &&
				     ("psu_cortexa53_0" == $master) } {
					continue
				}
				if { ($rpu0_as_power_management_master == false) &&
				     ("psu_cortexr5_0" == $master) } {
					continue
				}
			}
			lappend macro_list [get_ipi_mask_txt $master]
		}
	}

	#Return the ORed macro list
	if { [llength $macro_list] > 0 } {
		return [join $macro_list " | "];
	} else {
		return "0U";
	}
}


#=============================================================================#
# Get the ORed list of macros as Permission mask for a given slave node
#=============================================================================#
proc get_slave_perm_mask_txt { slave } {
	set macro_list {}
	foreach master $pmufw::master_list {
		#Get the slave list for this master
		set slave_list [get_mem_ranges -of_objects [get_cells -hier $master]];
		#Search for the slave in list
		set slave_index [lsearch $slave_list $slave]
		#if found, add the macro to list
		if { $slave_index >=0 && [is_ipi_defined $master] == 1 } {
			lappend macro_list [get_ipi_mask_txt $master]
		}
	}
	#Return the ORed macro list
	if { [llength $macro_list] >0 } {
		return [join $macro_list " | "];
	} else {
		return "0U";
	}
}

#=============================================================================#
# Get the ORed list of macros as Permission mask for a reset line related to given slave node
#=============================================================================#
proc get_periph_perm_mask_txt_for_rst_line { reset_line } {
	set macro_list1 {}
	set macro_list2 {}
	set reset_management_master_list "[get_list_of_management_master "reset"]"

	set line_node [dict get [dict get $pmufw::reset_line_map $reset_line] node ]
	set periph_name [dict get [dict get $pmufw::node_map $line_node] periph]
	set periph_type [dict get [dict get $pmufw::node_map $line_node] type]

	if { ($periph_type == "slave")} {
		foreach master $pmufw::master_list {
			#Get the slave list for this master
			set slave_list [get_mem_ranges -of_objects [get_cells -hier $master]];
			#Search for the slave in list
			set slave_index [lsearch $slave_list $periph_name]
			#if found, add the macro to list
			if { $slave_index >=0 && [is_ipi_defined $master] == 1 } {
				set master_ipi_mask_txt [get_ipi_mask_txt $master]
				lappend macro_list1 $master_ipi_mask_txt
				if { (-1 != [string first "$master_ipi_mask_txt" "$reset_management_master_list"])} {
					lappend macro_list2 $master_ipi_mask_txt
				}
			}
		}
	} elseif { ($periph_type == "memory") } {
		if { ($periph_name == "psu_ddr")} {
			set mem_perms [get_mem_perm_mask $periph_name]
		} elseif { ($periph_name == "psu_ocm_0")} {
			set ocm_bank_0_perms [get_mem_perm_mask "psu_ocm_0"]
			set ocm_bank_1_perms [get_mem_perm_mask "psu_ocm_1"]
			set ocm_bank_2_perms [get_mem_perm_mask "psu_ocm_2"]
			set ocm_bank_3_perms [get_mem_perm_mask "psu_ocm_3"]
			set mem_perms [expr ($ocm_bank_0_perms | $ocm_bank_1_perms | $ocm_bank_2_perms | $ocm_bank_3_perms)]
		} else {
			set mem_perms "0U"
		}
		foreach master $pmufw::master_list {
			if { [expr (($mem_perms & [get_ipi_mask $master]) != 0)] &&
			     [is_ipi_defined $master] == 1 } {
				set master_ipi_mask_txt [get_ipi_mask_txt $master]
				lappend macro_list1 $master_ipi_mask_txt
				if { (-1 != [string first "$master_ipi_mask_txt" "$reset_management_master_list"])} {
					lappend macro_list2 $master_ipi_mask_txt
				}
			}
		}
	}

	#Return the ORed macro list
	if { [llength $macro_list2] >0 } {
		return [join $macro_list2 " | "];
	} elseif { [llength $macro_list1] >0 } {
		return [join $macro_list1 " | "];
	} else {
		return "0U";
	}
}

#puts "PMUFW Config Generator"
#puts [info script]

# Create a map of all the Nodes
#global node_map
set node_map [dict create]
dict set node_map NODE_APU { label NODE_APU periph psu_cortexa53_0 type processor }
dict set node_map NODE_APU_0 { label NODE_APU_0 periph psu_cortexa53_0 type processor }
dict set node_map NODE_APU_1 { label NODE_APU_1 periph psu_cortexa53_1 type processor }
dict set node_map NODE_APU_2 { label NODE_APU_2 periph psu_cortexa53_2 type processor }
dict set node_map NODE_APU_3 { label NODE_APU_3 periph psu_cortexa53_3 type processor }
dict set node_map NODE_RPU { label NODE_RPU periph psu_cortexr5_0 type processor }
dict set node_map NODE_RPU_0 { label NODE_RPU_0 periph psu_cortexr5_0 type processor }
dict set node_map NODE_RPU_1 { label NODE_RPU_1 periph psu_cortexr5_1 type processor }
dict set node_map NODE_PLD { label NODE_PLD periph NA type power }
dict set node_map NODE_FPD { label NODE_FPD periph NA type power }
dict set node_map NODE_OCM_BANK_0 { label NODE_OCM_BANK_0 periph psu_ocm_0 type memory }
dict set node_map NODE_OCM_BANK_1 { label NODE_OCM_BANK_1 periph psu_ocm_1 type memory }
dict set node_map NODE_OCM_BANK_2 { label NODE_OCM_BANK_2 periph psu_ocm_2 type memory }
dict set node_map NODE_OCM_BANK_3 { label NODE_OCM_BANK_3 periph psu_ocm_3 type memory }
dict set node_map NODE_TCM_0_A { label NODE_TCM_0_A periph psu_r5_0_atcm_global type memory }
dict set node_map NODE_TCM_0_B { label NODE_TCM_0_B periph psu_r5_0_btcm_global type memory }
dict set node_map NODE_TCM_1_A { label NODE_TCM_1_A periph psu_r5_1_atcm_global type memory }
dict set node_map NODE_TCM_1_B { label NODE_TCM_1_B periph psu_r5_1_btcm_global type memory }
dict set node_map NODE_L2 { label NODE_L2 periph NA type others }
dict set node_map NODE_GPU_PP_0 { label NODE_GPU_PP_0 periph psu_gpu type slave }
dict set node_map NODE_GPU_PP_1 { label NODE_GPU_PP_1 periph psu_gpu type slave }
dict set node_map NODE_USB_0 { label NODE_USB_0 periph psu_usb_0 type slave }
dict set node_map NODE_USB_1 { label NODE_USB_1 periph psu_usb_1 type slave }
dict set node_map NODE_TTC_0 { label NODE_TTC_0 periph psu_ttc_0 type slave }
dict set node_map NODE_TTC_1 { label NODE_TTC_1 periph psu_ttc_1 type slave }
dict set node_map NODE_TTC_2 { label NODE_TTC_2 periph psu_ttc_2 type slave }
dict set node_map NODE_TTC_3 { label NODE_TTC_3 periph psu_ttc_3 type slave }
dict set node_map NODE_SATA { label NODE_SATA periph psu_sata type slave }
dict set node_map NODE_ETH_0 { label NODE_ETH_0 periph psu_ethernet_0 type slave }
dict set node_map NODE_ETH_1 { label NODE_ETH_1 periph psu_ethernet_1 type slave }
dict set node_map NODE_ETH_2 { label NODE_ETH_2 periph psu_ethernet_2 type slave }
dict set node_map NODE_ETH_3 { label NODE_ETH_3 periph psu_ethernet_3 type slave }
dict set node_map NODE_UART_0 { label NODE_UART_0 periph psu_uart_0 type slave }
dict set node_map NODE_UART_1 { label NODE_UART_1 periph psu_uart_1 type slave }
dict set node_map NODE_SPI_0 { label NODE_SPI_0 periph psu_spi_0 type slave }
dict set node_map NODE_SPI_1 { label NODE_SPI_1 periph psu_spi_1 type slave }
dict set node_map NODE_I2C_0 { label NODE_I2C_0 periph psu_i2c_0 type slave }
dict set node_map NODE_I2C_1 { label NODE_I2C_1 periph psu_i2c_1 type slave }
dict set node_map NODE_SD_0 { label NODE_SD_0 periph psu_sd_0 type slave }
dict set node_map NODE_SD_1 { label NODE_SD_1 periph psu_sd_1 type slave }
dict set node_map NODE_DP { label NODE_DP periph psu_dp type slave }
dict set node_map NODE_GDMA { label NODE_GDMA periph psu_gdma_0 type slave }
dict set node_map NODE_ADMA { label NODE_ADMA periph psu_adma_0 type slave }
dict set node_map NODE_NAND { label NODE_NAND periph psu_nand_0 type slave }
dict set node_map NODE_QSPI { label NODE_QSPI periph psu_qspi_0 type slave }
dict set node_map NODE_GPIO { label NODE_GPIO periph psu_gpio_0 type slave }
dict set node_map NODE_CAN_0 { label NODE_CAN_0 periph psu_can_0 type slave }
dict set node_map NODE_CAN_1 { label NODE_CAN_1 periph psu_can_1 type slave }
dict set node_map NODE_EXTERN { label NODE_EXTERN periph NA type others }
dict set node_map NODE_DDR { label NODE_DDR periph psu_ddr type memory }
dict set node_map NODE_IPI_APU { label NODE_IPI_APU periph NA type ipi }
dict set node_map NODE_IPI_RPU_0 { label NODE_IPI_RPU_0 periph NA type ipi }
dict set node_map NODE_IPI_RPU_1 { label NODE_IPI_RPU_1 periph NA type ipi }
dict set node_map NODE_GPU { label NODE_GPU periph psu_gpu type slave }
dict set node_map NODE_PCIE { label NODE_PCIE periph psu_pcie type slave }
dict set node_map NODE_PCAP { label NODE_PCAP periph NA type slave }
dict set node_map NODE_RTC { label NODE_RTC periph psu_rtc type slave }
dict set node_map NODE_VCU { label NODE_VCU periph vcu_0 type slave }
dict set node_map NODE_PL { label NODE_PL periph NA type others }

# Create a map of all reset lines
set reset_line_map [dict create]
dict set reset_line_map XILPM_RESET_PCIE_CFG { label XILPM_RESET_PCIE_CFG type rst_periph node NODE_PCIE }
dict set reset_line_map XILPM_RESET_PCIE_BRIDGE { label XILPM_RESET_PCIE_BRIDGE type rst_periph node NODE_PCIE }
dict set reset_line_map XILPM_RESET_PCIE_CTRL { label XILPM_RESET_PCIE_CTRL type rst_periph node NODE_PCIE }
dict set reset_line_map XILPM_RESET_DP { label XILPM_RESET_DP type rst_periph node NODE_DP }
dict set reset_line_map XILPM_RESET_SWDT_CRF { label XILPM_RESET_SWDT_CRF type normal }
dict set reset_line_map XILPM_RESET_AFI_FM5 { label XILPM_RESET_AFI_FM5 type normal }
dict set reset_line_map XILPM_RESET_AFI_FM4 { label XILPM_RESET_AFI_FM4 type normal }
dict set reset_line_map XILPM_RESET_AFI_FM3 { label XILPM_RESET_AFI_FM3 type normal }
dict set reset_line_map XILPM_RESET_AFI_FM2 { label XILPM_RESET_AFI_FM2 type normal }
dict set reset_line_map XILPM_RESET_AFI_FM1 { label XILPM_RESET_AFI_FM1 type normal }
dict set reset_line_map XILPM_RESET_AFI_FM0 { label XILPM_RESET_AFI_FM0 type normal }
dict set reset_line_map XILPM_RESET_GDMA { label XILPM_RESET_GDMA type rst_periph node NODE_GDMA }
dict set reset_line_map XILPM_RESET_GPU_PP1 { label XILPM_RESET_GPU_PP1 type rst_periph node NODE_GPU_PP_1 }
dict set reset_line_map XILPM_RESET_GPU_PP0 { label XILPM_RESET_GPU_PP0 type rst_periph node NODE_GPU_PP_0 }
dict set reset_line_map XILPM_RESET_GPU { label XILPM_RESET_GPU type rst_periph node NODE_GPU }
dict set reset_line_map XILPM_RESET_GT { label XILPM_RESET_GT type normal }
dict set reset_line_map XILPM_RESET_SATA { label XILPM_RESET_SATA type rst_periph node NODE_SATA }
dict set reset_line_map XILPM_RESET_ACPU3_PWRON { label XILPM_RESET_ACPU3_PWRON type rst_proc proc APU }
dict set reset_line_map XILPM_RESET_ACPU2_PWRON { label XILPM_RESET_ACPU2_PWRON type rst_proc proc APU }
dict set reset_line_map XILPM_RESET_ACPU1_PWRON { label XILPM_RESET_ACPU1_PWRON type rst_proc proc APU }
dict set reset_line_map XILPM_RESET_ACPU0_PWRON { label XILPM_RESET_ACPU0_PWRON type rst_proc proc APU }
dict set reset_line_map XILPM_RESET_APU_L2 { label XILPM_RESET_APU_L2 type rst_proc proc APU }
dict set reset_line_map XILPM_RESET_ACPU3 { label XILPM_RESET_ACPU3 type rst_proc proc APU }
dict set reset_line_map XILPM_RESET_ACPU2 { label XILPM_RESET_ACPU2 type rst_proc proc APU }
dict set reset_line_map XILPM_RESET_ACPU1 { label XILPM_RESET_ACPU1 type rst_proc proc APU }
dict set reset_line_map XILPM_RESET_ACPU0 { label XILPM_RESET_ACPU0 type rst_proc proc APU }
dict set reset_line_map XILPM_RESET_DDR { label XILPM_RESET_DDR type rst_periph node NODE_DDR }
dict set reset_line_map XILPM_RESET_APM_FPD { label XILPM_RESET_APM_FPD type normal }
dict set reset_line_map XILPM_RESET_SOFT { label XILPM_RESET_SOFT type rst_shared }
dict set reset_line_map XILPM_RESET_GEM0 { label XILPM_RESET_GEM0 type rst_periph node NODE_ETH_0 }
dict set reset_line_map XILPM_RESET_GEM1 { label XILPM_RESET_GEM1 type rst_periph node NODE_ETH_1 }
dict set reset_line_map XILPM_RESET_GEM2 { label XILPM_RESET_GEM2 type rst_periph node NODE_ETH_2 }
dict set reset_line_map XILPM_RESET_GEM3 { label XILPM_RESET_GEM3 type rst_periph node NODE_ETH_3 }
dict set reset_line_map XILPM_RESET_QSPI { label XILPM_RESET_QSPI type rst_periph node NODE_QSPI }
dict set reset_line_map XILPM_RESET_UART0 { label XILPM_RESET_UART0 type rst_periph node NODE_UART_0 }
dict set reset_line_map XILPM_RESET_UART1 { label XILPM_RESET_UART1 type rst_periph node NODE_UART_1 }
dict set reset_line_map XILPM_RESET_SPI0 { label XILPM_RESET_SPI0 type rst_periph node NODE_SPI_0 }
dict set reset_line_map XILPM_RESET_SPI1 { label XILPM_RESET_SPI1 type rst_periph node NODE_SPI_1 }
dict set reset_line_map XILPM_RESET_SDIO0 { label XILPM_RESET_SDIO0 type normal }
dict set reset_line_map XILPM_RESET_SDIO1 { label XILPM_RESET_SDIO1 type normal }
dict set reset_line_map XILPM_RESET_CAN0 { label XILPM_RESET_CAN0 type rst_periph node NODE_CAN_0 }
dict set reset_line_map XILPM_RESET_CAN1 { label XILPM_RESET_CAN1 type rst_periph node NODE_CAN_1 }
dict set reset_line_map XILPM_RESET_I2C0 { label XILPM_RESET_I2C0 type rst_periph node NODE_I2C_0 }
dict set reset_line_map XILPM_RESET_I2C1 { label XILPM_RESET_I2C1 type rst_periph node NODE_I2C_1 }
dict set reset_line_map XILPM_RESET_TTC0 { label XILPM_RESET_TTC0 type rst_periph node NODE_TTC_0 }
dict set reset_line_map XILPM_RESET_TTC1 { label XILPM_RESET_TTC1 type rst_periph node NODE_TTC_1 }
dict set reset_line_map XILPM_RESET_TTC2 { label XILPM_RESET_TTC2 type rst_periph node NODE_TTC_2 }
dict set reset_line_map XILPM_RESET_TTC3 { label XILPM_RESET_TTC3 type rst_periph node NODE_TTC_3 }
dict set reset_line_map XILPM_RESET_SWDT_CRL { label XILPM_RESET_SWDT_CRL type normal }
dict set reset_line_map XILPM_RESET_NAND { label XILPM_RESET_NAND type rst_periph node NODE_NAND }
dict set reset_line_map XILPM_RESET_ADMA { label XILPM_RESET_ADMA type rst_periph node NODE_ADMA }
dict set reset_line_map XILPM_RESET_GPIO { label XILPM_RESET_GPIO type normal }
dict set reset_line_map XILPM_RESET_IOU_CC { label XILPM_RESET_IOU_CC type normal }
dict set reset_line_map XILPM_RESET_TIMESTAMP { label XILPM_RESET_TIMESTAMP type normal }
dict set reset_line_map XILPM_RESET_RPU_R50 { label XILPM_RESET_RPU_R50 type rst_proc proc RPU_0 }
dict set reset_line_map XILPM_RESET_RPU_R51 { label XILPM_RESET_RPU_R51 type rst_proc proc RPU_1 }
dict set reset_line_map XILPM_RESET_RPU_AMBA { label XILPM_RESET_RPU_AMBA type rst_proc proc RPU }
dict set reset_line_map XILPM_RESET_OCM { label XILPM_RESET_OCM type rst_periph node NODE_OCM_BANK_0 }
dict set reset_line_map XILPM_RESET_RPU_PGE { label XILPM_RESET_RPU_PGE type rst_proc proc RPU }
dict set reset_line_map XILPM_RESET_USB0_CORERESET { label XILPM_RESET_USB0_CORERESET type rst_periph node NODE_USB_0 }
dict set reset_line_map XILPM_RESET_USB1_CORERESET { label XILPM_RESET_USB1_CORERESET type rst_periph node NODE_USB_1 }
dict set reset_line_map XILPM_RESET_USB0_HIBERRESET { label XILPM_RESET_USB0_HIBERRESET type rst_periph node NODE_USB_0 }
dict set reset_line_map XILPM_RESET_USB1_HIBERRESET { label XILPM_RESET_USB1_HIBERRESET type rst_periph node NODE_USB_1 }
dict set reset_line_map XILPM_RESET_USB0_APB { label XILPM_RESET_USB0_APB type rst_periph node NODE_USB_0 }
dict set reset_line_map XILPM_RESET_USB1_APB { label XILPM_RESET_USB1_APB type rst_periph node NODE_USB_1 }
dict set reset_line_map XILPM_RESET_IPI { label XILPM_RESET_IPI type rst_shared }
dict set reset_line_map XILPM_RESET_APM_LPD { label XILPM_RESET_APM_LPD type normal }
dict set reset_line_map XILPM_RESET_RTC { label XILPM_RESET_RTC type rst_periph node NODE_RTC }
dict set reset_line_map XILPM_RESET_SYSMON { label XILPM_RESET_SYSMON type NA }
dict set reset_line_map XILPM_RESET_AFI_FM6 { label XILPM_RESET_AFI_FM6 type normal }
dict set reset_line_map XILPM_RESET_LPD_SWDT { label XILPM_RESET_LPD_SWDT type normal }
dict set reset_line_map XILPM_RESET_FPD { label XILPM_RESET_FPD type rpu_only }
dict set reset_line_map XILPM_RESET_RPU_DBG1 { label XILPM_RESET_RPU_DBG1 type rst_proc proc RPU }
dict set reset_line_map XILPM_RESET_RPU_DBG0 { label XILPM_RESET_RPU_DBG0 type rst_proc proc RPU }
dict set reset_line_map XILPM_RESET_DBG_LPD { label XILPM_RESET_DBG_LPD type normal }
dict set reset_line_map XILPM_RESET_DBG_FPD { label XILPM_RESET_DBG_FPD type normal }
dict set reset_line_map XILPM_RESET_APLL { label XILPM_RESET_APLL type rst_proc proc APU }
dict set reset_line_map XILPM_RESET_DPLL { label XILPM_RESET_DPLL type rst_shared }
dict set reset_line_map XILPM_RESET_VPLL { label XILPM_RESET_VPLL type rst_shared }
dict set reset_line_map XILPM_RESET_IOPLL { label XILPM_RESET_IOPLL type rst_shared }
dict set reset_line_map XILPM_RESET_RPLL { label XILPM_RESET_RPLL type rst_proc proc RPU }
dict set reset_line_map XILPM_RESET_GPO3_PL_0 { label XILPM_RESET_GPO3_PL_0 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_1 { label XILPM_RESET_GPO3_PL_1 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_2 { label XILPM_RESET_GPO3_PL_2 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_3 { label XILPM_RESET_GPO3_PL_3 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_4 { label XILPM_RESET_GPO3_PL_4 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_5 { label XILPM_RESET_GPO3_PL_5 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_6 { label XILPM_RESET_GPO3_PL_6 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_7 { label XILPM_RESET_GPO3_PL_7 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_8 { label XILPM_RESET_GPO3_PL_8 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_9 { label XILPM_RESET_GPO3_PL_9 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_10 { label XILPM_RESET_GPO3_PL_10 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_11 { label XILPM_RESET_GPO3_PL_11 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_12 { label XILPM_RESET_GPO3_PL_12 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_13 { label XILPM_RESET_GPO3_PL_13 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_14 { label XILPM_RESET_GPO3_PL_14 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_15 { label XILPM_RESET_GPO3_PL_15 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_16 { label XILPM_RESET_GPO3_PL_16 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_17 { label XILPM_RESET_GPO3_PL_17 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_18 { label XILPM_RESET_GPO3_PL_18 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_19 { label XILPM_RESET_GPO3_PL_19 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_20 { label XILPM_RESET_GPO3_PL_20 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_21 { label XILPM_RESET_GPO3_PL_21 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_22 { label XILPM_RESET_GPO3_PL_22 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_23 { label XILPM_RESET_GPO3_PL_23 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_24 { label XILPM_RESET_GPO3_PL_24 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_25 { label XILPM_RESET_GPO3_PL_25 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_26 { label XILPM_RESET_GPO3_PL_26 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_27 { label XILPM_RESET_GPO3_PL_27 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_28 { label XILPM_RESET_GPO3_PL_28 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_29 { label XILPM_RESET_GPO3_PL_29 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_30 { label XILPM_RESET_GPO3_PL_30 type normal }
dict set reset_line_map XILPM_RESET_GPO3_PL_31 { label XILPM_RESET_GPO3_PL_31 type normal }
dict set reset_line_map XILPM_RESET_RPU_LS { label XILPM_RESET_RPU_LS type rst_proc proc RPU }
dict set reset_line_map XILPM_RESET_PS_ONLY { label XILPM_RESET_PS_ONLY type normal }
dict set reset_line_map XILPM_RESET_PL { label XILPM_RESET_PL type normal }
dict set reset_line_map XILPM_RESET_GPIO5_EMIO_92 { label XILPM_RESET_GPIO5_EMIO_92 type normal }
dict set reset_line_map XILPM_RESET_GPIO5_EMIO_93 { label XILPM_RESET_GPIO5_EMIO_93 type normal }
dict set reset_line_map XILPM_RESET_GPIO5_EMIO_94 { label XILPM_RESET_GPIO5_EMIO_94 type normal }
dict set reset_line_map XILPM_RESET_GPIO5_EMIO_95 { label XILPM_RESET_GPIO5_EMIO_95 type normal }

proc get_slave_section { } {
	#global node_map
	set slave_count 0
	set slave_text ""

	# Loop through each node
	foreach node [dict keys $pmufw::node_map] {
		set periph_name [dict get [dict get $pmufw::node_map $node] periph]
		set periph_type [dict get [dict get $pmufw::node_map $node] type]
		set periph_label [dict get [dict get $pmufw::node_map $node] label]

		# Process nodes of type "SLAVE". if periph_type is NA, sets perm to 0U
		if { ($periph_type == "slave")} {

			#set the perms(ipi mask) value for this node
			dict set pmufw::node_map $node perms [get_slave_perm_mask_txt $periph_name]
			#print out for debug purpose
			#puts "$periph_name \t: [dict get [dict get $pmufw::node_map $node] perms] "

			if { [dict get [dict get $pmufw::node_map $node] perms] == "0U" } {
				continue
			}

			#Increment the slave count
			incr slave_count

			#concat to the slave data text
			#append  slave_text "\t/**********************************************************************/\n"
			append  slave_text "\t[dict get [dict get $pmufw::node_map $node] label],\n"
			append  slave_text "\tPM_SLAVE_FLAG_IS_SHAREABLE,\n"
			append  slave_text "\t[dict get [dict get $pmufw::node_map $node] perms], /* IPI Mask */\n\n"
		}
		# Process nodes of type "MEMORY"
		if { ($periph_type == "memory") && ($periph_name != "NA") } {

			#Increment the slave count
			incr slave_count

			#set the perms(ipi mask) value for this node
			dict set pmufw::node_map $node perms [convert_ipi_mask_to_txt [get_mem_perm_mask $periph_name]]
			#print out for debug purpose
			#puts "$periph_name \t: [dict get [dict get $pmufw::node_map $node] perms] "

			#concat to the slave data text
			#append  slave_text "\t/**********************************************************************/\n"
			append  slave_text "\t[dict get [dict get $pmufw::node_map $node] label],\n"
			append  slave_text "\tPM_SLAVE_FLAG_IS_SHAREABLE,\n"
			append  slave_text "\t[dict get [dict get $pmufw::node_map $node] perms], /* IPI Mask */\n\n"
		}

		# Process nodes of type "others"
		if { ($periph_type == "others") } {

			#Increment the slave count
			incr slave_count

			#set the perms(ipi mask) value for this node
			dict set pmufw::node_map $node perms [get_all_masters_mask_txt]
			#print out for debug purpose
			#puts "$periph_name \t: [dict get [dict get $pmufw::node_map $node] perms] "

			#concat to the slave data text
			#append  slave_text "\t/**********************************************************************/\n"
			append  slave_text "\t[dict get [dict get $pmufw::node_map $node] label],\n"
			append  slave_text "\tPM_SLAVE_FLAG_IS_SHAREABLE,\n"
			append  slave_text "\t[dict get [dict get $pmufw::node_map $node] perms], /* IPI Mask */\n\n"
		}

		#Process nodes of type IPI
		set ipi_perm ""
		if { ($periph_type == "ipi")} {

			#puts $periph_label
			switch $periph_label {
				"NODE_IPI_APU" {
					if { [lsearch $pmufw::master_list psu_cortexa53_0] >= 0 &&
					     [is_ipi_defined psu_cortexa53_0] == 1 } {
						set ipi_perm [get_ipi_mask_txt psu_cortexa53_0]
					} else {
						set ipi_perm ""
					}
				}
				"NODE_IPI_RPU_0" {
					if { [lsearch $pmufw::master_list psu_cortexr5_0] >= 0 &&
					     [is_ipi_defined psu_cortexr5_0] == 1 } {
						set ipi_perm [get_ipi_mask_txt psu_cortexr5_0]
					} else {
						set ipi_perm ""
					}
				}
				"NODE_IPI_RPU_1" {
					if { [lsearch $pmufw::master_list psu_cortexr5_1] >= 0 &&
					     [is_ipi_defined psu_cortexr5_1] == 1 } {
						set ipi_perm [get_ipi_mask_txt psu_cortexr5_1]
					} else {
						set ipi_perm ""
					}
				}
				default {
					set ipi_perm ""
				}
			}
			if { $ipi_perm != "" } {
				#Increment the slave count
				incr slave_count

				#set the perms(ipi mask) value for this node
				dict set pmufw::node_map $node perms $ipi_perm
				#concat to the slave data text
				#append  slave_text "\t/**********************************************************************/\n"
				append  slave_text "\t[dict get [dict get $pmufw::node_map $node] label],\n"
				append  slave_text "\t0U,\n"
				append  slave_text "\t[dict get [dict get $pmufw::node_map $node] perms], /* IPI Mask */\n\n"
			}
		}

	}

	set slave_text "

\tPM_CONFIG_SLAVE_SECTION_ID,	/* Section ID */
\t$slave_count,				/* Number of slaves */

$slave_text"

	return $slave_text
}

proc get_master_ipidef { } {
	set master_ipidef "\n"
	foreach master $pmufw::master_list {
		if { [is_ipi_defined $master] == 1 } {
			append master_ipidef "#define "  [get_ipi_mask_txt $master] "    " [get_ipi_mask $master] "\n"
		}
	}
	append master_ipidef "\n"

	return $master_ipidef
}


proc get_master_section { } {
	set master_text ""

	append master_text "\tPM_CONFIG_MASTER_SECTION_ID, /* Master SectionID */" "\n"

	set master_count [llength $pmufw::master_list]
	append master_text "\t$master_count" "U, /* No. of Masters*/" "\n"
	append master_text "\n"

	foreach master $pmufw::master_list {
		if { [dict get [dict get $pmufw::master_map $master] name] == "RPU0" } {
			if { [is_rpu_lockstep] } {
				set master_node "NODE_RPU"
			} else {
				set master_node "NODE_RPU_0"
			}
		} elseif { [dict get [dict get $pmufw::master_map $master] name] == "RPU1" } {
			set master_node "NODE_RPU_1"
		} else {
			set master_node "NODE_APU"
		}
		append master_text "\t$master_node, /* Master Node ID */" "\n"
		if { [is_ipi_defined $master] == 1 } {
			append master_text "\t[get_ipi_mask_txt $master], /* IPI Mask of this master */" "\n"
		} else {
			append master_text "\t0U, /* IPI Mask of this master */" "\n"
		}
		append master_text "\tSUSPEND_TIMEOUT, /* Suspend timeout */" "\n"
		append master_text "\t[get_all_other_masters_mask_txt $master], /* Suspend permissions */" "\n"
		append master_text "\t[get_all_other_masters_mask_txt $master], /* Wake permissions */" "\n"
		append master_text "\n"
	}

	return $master_text
}

proc get_prealloc_for_master_txt { master_name prealloc_list } {
	set node_count 0
	set master_prealloc_txt ""

	if { [is_ipi_defined $master_name] == 1 } {
		set master_mask [get_ipi_mask_txt $master_name]
	}
	foreach node $prealloc_list {
		set periph_perms [dict get [dict get $pmufw::node_map $node] perms]
		set periph_name [dict get [dict get $pmufw::node_map $node] periph]
		set periph_type [dict get [dict get $pmufw::node_map $node] type]
		set periph_label [dict get [dict get $pmufw::node_map $node] label]

		if { [string first $master_mask $periph_perms ] >= 0 } {
			append master_prealloc_txt "\t$periph_label," "\n"
			append master_prealloc_txt "\tPM_MASTER_USING_SLAVE_MASK, /* Master is using Slave */" "\n"
			append master_prealloc_txt "\tPM_CAP_ACCESS | PM_CAP_CONTEXT, /* Current Requirements */" "\n"
			append master_prealloc_txt "\tPM_CAP_ACCESS | PM_CAP_CONTEXT, /* Default Requirements */" "\n"
			append master_prealloc_txt "\n"
			incr node_count
		}
	}

	set master_prealloc_txt "/* Prealloc for $master_name */
	$master_mask,
	$node_count,
$master_prealloc_txt
	"
	return $master_prealloc_txt
}

proc get_prealloc_section { proc_type } {
	set prealloc_text "\n"
	set apu_prealloc_list {NODE_DDR NODE_L2 NODE_OCM_BANK_0 NODE_OCM_BANK_1 NODE_OCM_BANK_2 NODE_OCM_BANK_3 NODE_I2C_0 NODE_I2C_1 NODE_SD_1 NODE_QSPI NODE_PL}
	if { [is_ipi_defined psu_cortexa53_0] == 1 } {
		append apu_prealloc_list { NODE_IPI_APU}
	}
	set r5_0_prealloc_list {NODE_TCM_0_A NODE_TCM_0_B NODE_TCM_1_A NODE_TCM_1_B}
	if { "psu_cortexr5" == $proc_type } {
		append r5_0_prealloc_list { NODE_DDR NODE_OCM_BANK_0 NODE_OCM_BANK_1 NODE_OCM_BANK_2 NODE_OCM_BANK_3 NODE_I2C_0 NODE_I2C_1 NODE_SD_1 NODE_QSPI NODE_PL NODE_ADMA}
	}
	if { [is_ipi_defined psu_cortexr5_0] == 1 } {
		append r5_0_prealloc_list { NODE_IPI_RPU_0}
	}
	set r5_1_prealloc_list {NODE_TCM_1_A NODE_TCM_1_B}
	if { [is_ipi_defined psu_cortexr5_1] == 1 } {
		append r5_1_prealloc_list { NODE_IPI_RPU_1}
	}

	append prealloc_text "\tPM_CONFIG_PREALLOC_SECTION_ID, /* Preallaoc SectionID */" "\n"

	set master_count 0
	foreach master $pmufw::master_list {
		if { [is_ipi_defined $master] == 1 } {
			incr master_count
		}
	}
	append prealloc_text "\t$master_count" "U, /* No. of Masters*/" "\n"
	append prealloc_text "\n"

	if { [lsearch $pmufw::master_list psu_cortexa53_0] >= 0 &&
	     [is_ipi_defined psu_cortexa53_0] == 1 } {
		append prealloc_text [get_prealloc_for_master_txt psu_cortexa53_0 $apu_prealloc_list]
	}
	if { [lsearch $pmufw::master_list psu_cortexr5_0] >= 0 &&
	     [is_ipi_defined psu_cortexr5_0] == 1 } {
		append prealloc_text [get_prealloc_for_master_txt psu_cortexr5_0 $r5_0_prealloc_list]
	}
	if { [lsearch $pmufw::master_list psu_cortexr5_1] >= 0 &&
	     [is_ipi_defined psu_cortexr5_1] == 1 } {
		append prealloc_text [get_prealloc_for_master_txt psu_cortexr5_1 $r5_1_prealloc_list]
	}

	return $prealloc_text
}

set power_perms [dict create]
dict set power_perms NODE_APU { psu_cortexr5_0 psu_cortexr5_1 }
dict set power_perms NODE_RPU { psu_cortexa53_0 psu_cortexr5_0 psu_cortexr5_1 }
dict set power_perms NODE_FPD { psu_cortexr5_0 psu_cortexr5_1 }
dict set power_perms NODE_PLD { psu_cortexa53_0 psu_cortexr5_0 psu_cortexr5_1 }

#=============================================================================#
# Return the Macro text for IPI masks of a all Masters having force power down
# permissions
#=============================================================================#
proc get_power_domain_perm_mask_txt { pwr_domain } {
	set macro_list {}

	set pwr_perm_masters [dict get $pmufw::power_perms $pwr_domain]
	global rpu0_as_power_management_master
	global rpu1_as_power_management_master
	global apu_as_power_management_master

	foreach master $pwr_perm_masters {
		if { [lsearch $pmufw::master_list $master] >= 0 &&
		     [is_ipi_defined $master] == 1 } {
			if { ($apu_as_power_management_master == false) &&
			     ("psu_cortexa53_0" == $master) } {
				continue
			} elseif { ($rpu0_as_power_management_master == false) &&
				   ("psu_cortexr5_0" == $master) } {
				continue
			} elseif { ($rpu1_as_power_management_master == false) &&
				   (0 == [is_rpu_lockstep]) &&
				   ("psu_cortexr5_1" == $master) } {
				continue
			}

			lappend macro_list [get_ipi_mask_txt $master]
		}
	}

	if { ((("NODE_APU" == $pwr_domain) || ("NODE_FPD" == $pwr_domain)) &&
	      ([llength $macro_list] == 0)) } {
		set macro [get_ipi_mask_txt "psu_cortexr5_0"]
		if { $macro != "" } {
			lappend macro_list $macro
		}
	}

	#Return the ORed macro list
	if { [llength $macro_list] > 0 } {
		return [join $macro_list " | "];
	} else {
		return "0U";
	}
}

proc get_power_section { } {
	set power_text "\n"
	set power_node_list {NODE_APU NODE_RPU NODE_FPD NODE_PLD}

	append power_text "\tPM_CONFIG_POWER_SECTION_ID, /* Power Section ID */" "\n"
	append power_text "\t[llength $power_node_list]" "U, /* Number of power nodes */" "\n"
	append power_text "\n"

	foreach node $power_node_list {
		append power_text "\t$node, /* Power node ID */" "\n"
		append power_text "\t[get_power_domain_perm_mask_txt $node], /* Force power down permissions */" "\n"
		append power_text "\n"
	}

	return $power_text
}

#=============================================================================#
# Return true if all masters are enabled as power/reset management masters else false.
#=============================================================================#
proc is_all_master_enabled { master_type } {
	global rpu0_as_power_management_master
	global rpu1_as_power_management_master
	global apu_as_power_management_master
	global rpu0_as_reset_management_master
	global rpu1_as_reset_management_master
	global apu_as_reset_management_master

	if { ("power" == $master_type) } {
		set rpu0_as_master $rpu0_as_power_management_master
		set rpu1_as_master $rpu1_as_power_management_master
		set apu_as_master  $apu_as_power_management_master
	} elseif { ("reset" == $master_type) } {
		set rpu0_as_master $rpu0_as_reset_management_master
		set rpu1_as_master $rpu1_as_reset_management_master
		set apu_as_master  $apu_as_reset_management_master
	} else {
		return -1
	}

	if { ($rpu0_as_master == true) &&
	     ($apu_as_master == true) } {
		if { (0 == [is_rpu_lockstep]) } {
			if { ($rpu1_as_master == true) } {
				return 1
			} else {
				return 0
			}
		} else {
			return 1
		}
	} else {
		return 0
	}
}
#=============================================================================#
# Return the list of power/reset management masters based on master type and
# which are enabled in ORed form.
#=============================================================================#
proc get_list_of_management_master { master_type } {
	set macro_list {}
	global rpu0_as_power_management_master
	global rpu1_as_power_management_master
	global apu_as_power_management_master
	global rpu0_as_reset_management_master
	global rpu1_as_reset_management_master
	global apu_as_reset_management_master
	global rpu0_as_overlay_config_master
	global rpu1_as_overlay_config_master
	global apu_as_overlay_config_master

	if { ("power" == $master_type) } {
		set rpu0_as_master $rpu0_as_power_management_master
		set rpu1_as_master $rpu1_as_power_management_master
		set apu_as_master  $apu_as_power_management_master
	} elseif { ("reset" == $master_type) } {
		set rpu0_as_master $rpu0_as_reset_management_master
		set rpu1_as_master $rpu1_as_reset_management_master
		set apu_as_master  $apu_as_reset_management_master
	} elseif { ("overlay_config" == $master_type) } {
		set rpu0_as_master $rpu0_as_overlay_config_master
		set rpu1_as_master $rpu1_as_overlay_config_master
		set apu_as_master  $apu_as_overlay_config_master
	} else {
		return "0U";
	}

	foreach master $pmufw::master_list {
		if { [is_ipi_defined $master] == 1 } {
			if { ($rpu0_as_master == false) &&
			     ("psu_cortexr5_0" == $master) } {
				continue
			}
			if { ($rpu1_as_master == false) &&
			     (0 == [is_rpu_lockstep]) &&
			     ("psu_cortexr5_1" == $master) } {
				continue
			}
			if { ($apu_as_master == false) &&
			     ("psu_cortexa53_0" == $master) } {
				continue
			}
			lappend macro_list [get_ipi_mask_txt $master]
		}
	}

	#Return the ORed macro list
	if { [llength $macro_list] > 0 } {
		return [join $macro_list " | "];
	} else {
		return "0U";
	}
}

proc get_reset_section { } {
	set reset_text "\n"
	set reset_management_master_list "[get_list_of_management_master "reset"]"

	append reset_text "\tPM_CONFIG_RESET_SECTION_ID, /* Reset Section ID */" "\n"
	append reset_text "\t[dict size $pmufw::reset_line_map]" "U, /* Number of resets */" "\n"
	append reset_text "\n"

	foreach reset_line [dict keys $pmufw::reset_line_map] {
		set line_name [dict get [dict get $pmufw::reset_line_map $reset_line] label]
		set line_type [dict get [dict get $pmufw::reset_line_map $reset_line] type]

		if { $line_type == "normal" } {
			append reset_text "\t$line_name, [get_all_masters_mask_txt]," "\n"
		} elseif { $line_type == "rpu_only" } {
			if { [lsearch $pmufw::master_list psu_cortexr5_0] >= 0 &&
			     [is_ipi_defined psu_cortexr5_0] == 1 } {
				append reset_text "\t$line_name, [get_ipi_mask_txt "psu_cortexr5_0"]," "\n"
			} else {
				append reset_text "\t$line_name, 0," "\n"
			}
		} elseif { (1 == [is_all_master_enabled "reset"]) && (($line_type == "rst_periph") ||
			   ($line_type == "rst_shared" ) || ($line_type == "rst_proc")) } {
			append reset_text "\t$line_name, [get_all_masters_mask_txt]," "\n"
		} elseif { (0 == [is_all_master_enabled "reset"]) && (($line_type == "rst_periph") ||
			   ($line_type == "rst_shared" ) || ($line_type == "rst_proc")) } {
			if { $line_type == "rst_periph" } {
				set perms [get_periph_perm_mask_txt_for_rst_line $reset_line]
				append reset_text "\t$line_name, $perms," "\n"
			} elseif { $line_type == "rst_shared" } {
				append reset_text "\t$line_name, $reset_management_master_list," "\n"
			} elseif { $line_type == "rst_proc" } {
				set line_proc [dict get [dict get $pmufw::reset_line_map $reset_line] proc ]
				set macro_list ""
				set master_txt ""
				if { $line_proc == "APU" } {
					append master_txt [get_ipi_mask_txt "psu_cortexa53_0"]
				} elseif {((($line_proc == "RPU_1") || ($line_proc == "RPU")) && ([is_rpu_lockstep])) ||
					  ($line_proc == "RPU_0") } {
					append master_txt [get_ipi_mask_txt "psu_cortexr5_0"]
				} elseif { $line_proc == "RPU_1" } {
					append master_txt [get_ipi_mask_txt "psu_cortexr5_1"]
				} elseif { $line_proc == "RPU" } {
					set master_rpu_0 [get_ipi_mask_txt "psu_cortexr5_0"]
					set master_rpu_1 [get_ipi_mask_txt "psu_cortexr5_1"]
					if { ((-1 == [string first "$master_rpu_0" "$reset_management_master_list"]) &&
					      ([string length "$master_rpu_0"]) &&
					      (-1 == [string first "$master_rpu_1" "$reset_management_master_list"]) &&
					      ([string length "$master_rpu_1"])) } {
						append master_txt "$master_rpu_0"
						append master_txt " | "
						append master_txt "$master_rpu_1"
					}
				}
				if { (-1 == [string first "$master_txt" "$reset_management_master_list"]) &&
				     ([string length "$master_txt"]) } {
					append macro_list "$master_txt"
				}
				if { (0 == [string match "0U" "$reset_management_master_list"])} {
					if { ([string length "$macro_list"]) } {
						append macro_list " | "
					}
					append macro_list "$reset_management_master_list"
				}
				if { (0 == [string length "$macro_list"])} {
					append macro_list "0U"
				}
				append reset_text "\t$line_name, $macro_list," "\n"
			}
		} else {
                        append reset_text "\t$line_name, 0," "\n"
		}
	}

	return $reset_text
}

proc get_gpo_section {} {
	set gpo_text ""

	append gpo_text "\tPM_CONFIG_GPO_SECTION_ID,\t\t/* GPO Section ID */\n"

	if { "1" == [get_property CONFIG.C_GPO2_POLARITY [get_cells -hier psu_pmu_iomodule]] } {
		append gpo_text "\tPM_CONFIG_GPO1_BIT_2_MASK |\n"
	}
	if { "1" == [get_property CONFIG.C_GPO3_POLARITY [get_cells -hier psu_pmu_iomodule]] } {
		append gpo_text "\tPM_CONFIG_GPO1_BIT_3_MASK |\n"
	}
	if { "1" == [get_property CONFIG.C_GPO4_POLARITY [get_cells -hier psu_pmu_iomodule]] } {
		append gpo_text "\tPM_CONFIG_GPO1_BIT_4_MASK |\n"
	}
	if { "1" == [get_property CONFIG.C_GPO5_POLARITY [get_cells -hier psu_pmu_iomodule]] } {
		append gpo_text "\tPM_CONFIG_GPO1_BIT_5_MASK |\n"
	}

	if { "1" == [get_property CONFIG.C_GPO2_ENABLE [get_cells -hier psu_pmu_iomodule]] } {
		append gpo_text "\tPM_CONFIG_GPO1_MIO_PIN_34_MAP |\n"
	}
	if { "1" == [get_property CONFIG.C_GPO3_ENABLE [get_cells -hier psu_pmu_iomodule]] } {
		append gpo_text "\tPM_CONFIG_GPO1_MIO_PIN_35_MAP |\n"
	}
	if { "1" == [get_property CONFIG.C_GPO4_ENABLE [get_cells -hier psu_pmu_iomodule]] } {
		append gpo_text "\tPM_CONFIG_GPO1_MIO_PIN_36_MAP |\n"
	}
	if { "1" == [get_property CONFIG.C_GPO5_ENABLE [get_cells -hier psu_pmu_iomodule]] } {
		append gpo_text "\tPM_CONFIG_GPO1_MIO_PIN_37_MAP |\n"
	}
	append gpo_text "\t0,\t\t\t\t\t/* State of GPO pins */"

	return $gpo_text
}

proc get_shutdown_section { } {
	set shutdown_text "\n"
	set power_management_master_list "[get_list_of_management_master "power"]"

	append shutdown_text "\tPM_CONFIG_SHUTDOWN_SECTION_ID, /* Shutdown Section ID */" "\n"
	append shutdown_text "\t$power_management_master_list, /* System Shutdown/Restart Permission */" "\n"

	return $shutdown_text
}

proc get_config_section {} {
	set config_text ""
	set overlay_config_master_list "[get_list_of_management_master "overlay_config"]"

	append config_text "\tPM_CONFIG_SET_CONFIG_SECTION_ID,\t\t/* Set Config Section ID */\n"
	append config_text "\t0U, /* Permissions to load base config object */" "\n"
	append config_text "\t$overlay_config_master_list, /* Permissions to load overlay config object */" "\n"

	return $config_text
}

proc gen_cfg_data { cfg_fname proc_type } {
# Open file and dump the data
set cfg_fid [open $cfg_fname w]

set pmufw::cfg_template [string map [list "<<MASTER_IPI_MASK_DEF>>" "[get_master_ipidef]"] $pmufw::cfg_template]
set pmufw::cfg_template [string map [list "<<MASTER_SECTION_DATA>>" "[get_master_section]"] $pmufw::cfg_template]
set pmufw::cfg_template [string map [list "<<SLAVE_SECTION_DATA>>" "[get_slave_section]"] $pmufw::cfg_template]
set pmufw::cfg_template [string map [list "<<PREALLOC_SECTION_DATA>>" "[get_prealloc_section $proc_type]"] $pmufw::cfg_template]
set pmufw::cfg_template [string map [list "<<POWER_SECTION_DATA>>" "[get_power_section]"] $pmufw::cfg_template]
set pmufw::cfg_template [string map [list "<<RESET_SECTION_DATA>>" "[get_reset_section]"] $pmufw::cfg_template]
set pmufw::cfg_template [string map [list "<<SET_CONFIG_SECTION_DATA>>" "[get_config_section]"] $pmufw::cfg_template]
set pmufw::cfg_template [string map [list "<<SHUTDOWN_SECTION_DATA>>" "[get_shutdown_section]"] $pmufw::cfg_template]
set pmufw::cfg_template [string map [list "<<GPO_SECTION_DATA>>" "[get_gpo_section]"] $pmufw::cfg_template]

puts $cfg_fid "$pmufw::cfg_template"
close $cfg_fid
#puts $node_map

}

}
