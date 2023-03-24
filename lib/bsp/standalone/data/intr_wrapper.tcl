###############################################################################
# Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
##############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 7.7   adk  24/11/21 First release
# 8.0   adk  09/08/22 Added support for versal net
# 8.1	adk  09/11/22 Fix interrupt parent address for versal net
# 9.0   sa   01/05/23 Added support for Microblaze RISC-V.
##############################################################################

#generate interrupt id and interrupt parent info
proc gen_intr {drv_handle file_name args} {
    set file_handle [::hsi::utils::open_include_file $file_name]
    set ips [::hsi::utils::get_common_driver_ips $drv_handle]
    set sw_processor [hsi::get_sw_processor]
    set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
    set processor_type [common::get_property IP_NAME $processor]

    foreach ip $ips {
        set isintr [::hsm::utils::is_ip_interrupting_current_proc $ip]
        set intc_parent_addr 0xffff
	set intr 0xffff
        set intr_prop "C_INTERRUPT"
        set dontgen_intrparent 0
	if {$isintr == 1} {
            if {$args eq ""} {
                set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ip]  -filter {TYPE==INTERRUPT&&DIRECTION==O}]
            } else {
               set intr_pin_name [lindex $args 0]
               set intr_prop [lindex $args 1]
               if {[llength $args] == 2 } {
                    set dontgen_intrparent 1
               }
            }
	    foreach pin $intr_pin_name {
		set intcname [::hsi::utils::get_connected_intr_cntrl $ip $pin]
                if {[llength $intcname] == 0 || [string match $intcname "{}"] || $intcname eq ""} {
                    continue
                }
		foreach intc $intcname {
		    set ipname [common::get_property IP_NAME $intc]
		    if {$processor_type == "ps7_cortexa9" && $ipname == "ps7_scugic"} {
			set intc_parent_addr [common::get_property CONFIG.C_PPI_S_AXI_BASEADDR $intc]
		    }
		    if {$processor_type == "psu_cortexa53" && $ipname == "psu_acpu_gic"} {
		        set intc_parent_addr [common::get_property CONFIG.C_S_AXI_BASEADDR $intc]
		    }
		    if {$processor_type == "psu_cortexr5" && $ipname == "psu_rcpu_gic"} {
			set intc_parent_addr [common::get_property CONFIG.C_S_AXI_BASEADDR $intc]
		    }
		    if {$processor_type == "psv_cortexa72" && $ipname == "psv_acpu_gic"} {
		        set intc_parent_addr [common::get_property CONFIG.C_S_AXI_BASEADDR $intc]
		    }
		    if {$processor_type == "psv_cortexr5" && $ipname == "psv_rcpu_gic"} {
			set intc_parent_addr [common::get_property CONFIG.C_S_AXI_BASEADDR $intc]
		    }
                    if {($processor_type == "psxl_cortexa78" && $ipname == "psxl_acpu_gic") || ($processor_type == "psx_cortexa78" && $ipname == "psx_acpu_gic") || ($processor_type == "psxl_cortexr52" && $ipname == "psxl_rcpu_gic") || ($processor_type == "psx_cortexr52" && $ipname == "psx_rcpu_gic")} {
                        set intc_parent_addr 0xE2000000U
                    }
		    if {$ipname == "axi_intc"} {
			set intc_parent_addr [common::get_property CONFIG.C_BASEADDR $intc]
			incr intc_parent_addr
			set intc_parent_addr [format 0x%xU $intc_parent_addr]
		    }
		}
            }

            if {${processor_type} == "microblaze" | ${processor_type} == "microblaze_riscv"} {
		set intcname [string toupper $intcname]
		set ip_name [string toupper $ip]
		set intr_pin_name [string toupper $intr_pin_name]
	        set sensitivity [get_intr_type $intc $ip $pin]
		if {$sensitivity != -1} {
			set sensitivity [format 0x%xU $sensitivity]
		}
	        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip $intr_prop] (XPAR_${intcname}_${ip_name}_${intr_pin_name}_INTR + $sensitivity)"
	   } else {
		set is_pl [get_property IS_PL [hsi::get_cells -hier $ip]]
                set ip_name [string toupper $ip]
		if {$is_pl} {
			set sensitivity [get_intr_type $intc $ip $pin]
			if {$sensitivity != -1} {
				set sensitivity [format 0x%xU $sensitivity]
			}
		        set intr_pin_name [string toupper $intr_pin_name]
                        if {[llength $intcname] == 0 || [string match $intcname "{}"] || $intcname eq ""} {
				puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip $intr_prop] 0xFFFFU"
			} else {
				puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip $intr_prop] (XPAR_FABRIC_${ip_name}_${intr_pin_name}_INTR + $sensitivity)"
			}
		} else {
		        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip $intr_prop] XPAR_${ip_name}_INTERRUPT_ID"
		}
	   }
	} else {
	        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip $intr_prop] $intr"
	}
        if {$dontgen_intrparent == 0} {
	        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "C_INTR_PARENT"] $intc_parent_addr"
	}
    }

    close $file_handle
}

proc get_intr_type {intc_name ip_name port_name} {
        set intc [hsi::get_cells -hier $intc_name]
        set ip [hsi::get_cells -hier $ip_name]
        if {[llength $intc] == 0 && [llength $ip] == 0} {
                return -1
        }
        if {[llength $intc] == 0} {
                return -1
        }
        set intr_pin [hsi::get_pins -of_objects $ip $port_name]
        set sensitivity ""
        if {[llength $intr_pin] >= 1} {
                set sensitivity [get_property SENSITIVITY $intr_pin]
        }
        set intc_type [get_property IP_NAME $intc ]
        set valid_intc_list "ps7_scugic psu_acpu_gic psv_acpu_gic psxl_acpu_gic psx_acpu_gic psxl_rcpu_gic psx_rcpu_gic"
        if {[lsearch  -nocase $valid_intc_list $intc_type] >= 0} {
                if {[string match -nocase $sensitivity "EDGE_FALLING"]} {
				set sens [expr {2 << 12}]
                                return $sens
                } elseif {[string match -nocase $sensitivity "EDGE_RISING"]} {
				set sens [expr {1 << 12}]
                                return $sens;
                } elseif {[string match -nocase $sensitivity "LEVEL_HIGH"]} {
				set sens [expr {4 << 12}]
                                return $sens
                } elseif {[string match -nocase $sensitivity "LEVEL_LOW"]} {
				set sens [expr {8 << 12}]
                                return $sens
                }
        } else {
                # Follow the openpic specification
                if {[string match -nocase $sensitivity "EDGE_FALLING"]} {
				set sens [expr {3 << 12}]
                                return $sens
                } elseif {[string match -nocase $sensitivity "EDGE_RISING"]} {
                                return 0;
                } elseif {[string match -nocase $sensitivity "LEVEL_HIGH"]} {
				set sens [expr {2 << 12}]
                                return $sens
                } elseif {[string match -nocase $sensitivity "LEVEL_LOW"]} {
				set sens [expr {1 << 12}]
                                return $sens
                }
        }
        return -1
}
