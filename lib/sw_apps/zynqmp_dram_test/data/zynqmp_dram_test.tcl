#/******************************************************************************
#* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
#* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

proc swapp_get_name {} {
    return "Zynq MP DRAM tests";
}

proc swapp_get_description {} {
    return "This application runs out of OCM and performs memory tests on Zynq MP DRAM. \
    For more information about the test, refer to ZYNQMP_DRAM_DIAGNOSTICS_TEST.docx, in the src directory of the application";
}

proc get_stdout {} {
    set os [hsi::get_os];
    if { $os == "" } {
        error "No Operating System specified in the Board Support Package";
    }
    
    set stdout [common::get_property CONFIG.STDOUT $os];
    return $stdout;
}

proc check_standalone_os {} {
    set os [hsi::get_os];

    if { $os == "" } {
        error "No Operating System specified in the Board Support Package";
    }
    
    if { $os != "standalone" } {
        error "This application is supported only on the Standalone Board Support Package.";
    }
}

proc check_stdout_hw {} {
	set slaves [common::get_property SLAVES [hsi::get_cells -hier [hsi::get_sw_processor]]]
	foreach slave $slaves {
		set slave_type [common::get_property IP_NAME [hsi::get_cells -hier $slave]];
		# Check for MDM-Uart peripheral. The MDM would be listed as a peripheral
		# only if it has a UART interface. So no further check is required
		if { $slave_type == "psu_uart" || $slave_type == "axi_uartlite" ||
			 $slave_type == "axi_uart16550" || $slave_type == "iomodule" ||
			 $slave_type == "mdm" } {
			return;
		}
	}

	error "This application requires a Uart IP in the hardware."
}

proc get_ip_sub_type { ip_inst_object} {
    if { [string compare -nocase cell [common::get_property CLASS $ip_inst_object]] != 0 } {
        error "get_mem_type API expect only mem_range type object whereas $class type object is passed"
    }

    set ip_type [common::get_property CONFIG.EDK_SPECIAL $ip_inst_object]
    if { [llength $ip_type] != 0 } {
        return $ip_type
    }

    set ip_name [common::get_property IP_NAME $ip_inst_object]
    if { [string compare -nocase "$ip_name"  "lmb_bram_if_cntlr"] == 0
        || [string compare -nocase "$ip_name" "isbram_if_cntlr"] == 0
        || [string compare -nocase "$ip_name" "axi_bram_ctrl"] == 0
        || [string compare -nocase "$ip_name" "dsbram_if_cntlr"] == 0
        || [string compare -nocase "$ip_name" "ps7_ram"] == 0 } {
	set ip_type "BRAM_CTRL"
    } elseif { [string match -nocase *ddr* "$ip_name" ] == 1 } {
	set ip_type "DDR_CTRL"
    } elseif { [string compare -nocase "$ip_name" "mpmc"] == 0 } {
	set ip_type "DRAM_CTRL"
    } elseif { [string compare -nocase "$ip_name" "axi_emc"] == 0 } {
	set ip_type "SRAM_FLASH_CTRL"
    } elseif { [string compare -nocase "$ip_name" "psu_ocm_ram_0"] == 0
	    || [string compare -nocase "$ip_name" "psu_ocm_ram_1"] == 0
	    || [string compare -nocase "$ip_name" "psu_ocm_ram"] == 0 } {
     set ip_type "OCM_CTRL"
    } else {
	set ip_type [common::get_property IP_TYPE $ip_inst_object]
    }

    return $ip_type
}

proc get_mem_type { mem } {
    set mem_type [get_ip_sub_type [hsi::get_cells $mem]]
    if { $mem_type == "BRAM_CTRL" } {
        return "BRAM"
    }
    if { $mem_type == "OCM_CTRL" } {
	return "OCM"
    }
    return "OTHER"
}

proc check_program_memory {} {
    # Obtain a list of "ID" memories
    set proc_instance [hsi::get_sw_processor]
    set idmemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells $proc_instance] -filter { IS_INSTRUCTION == true && IS_DATA == true && MEM_TYPE == "MEMORY" }]
    set security [common::get_property CONFIG.security_state [::hsi::get_os]]

    set required_mem_size 0x30000
    set ocm_ranges {}
    # Get a list of OCM regions
    foreach mem $idmemlist {
	# Skip memory regions if,
	# a. ACCESS_TYPE != RW
	# b. BSP security settings != TZ settings of the region
	# c. MEM TYPE != OCM
	set access [common::get_property ACCESS_TYPE $mem]
	if { $access != "Read/Write" } continue
	set tz [common::get_property TRUSTZONE $mem]
	if { $security == "secure" && $tz == "Strict-NonSecure" } continue
	if { $security == "non-secure" && $tz == "Secure" } continue
	if { [get_mem_type $mem] != "OCM" } continue
	set base [common::get_property BASE_VALUE $mem]
	set high [common::get_property HIGH_VALUE $mem]
	set size [expr $high - $base +1]
	if { $base == 0xfffc0000 && $size >= $required_mem_size } return
	lappend ocm_ranges [list $base $size]
    }
    if { [llength $ocm_ranges] == 0 } {
	error "This application requires at least [expr $required_mem_size/1024] KB of OCM memory at 0xfffc0000 to run"
    }

    # Sort the regions and contatenate sequential regions
    set ocm_ranges [lsort -integer -index 0 $ocm_ranges]
    set base0 [lindex $ocm_ranges 0 0]
    set size0 [lindex $ocm_ranges 0 1]
    set ocm_ranges [lrange $ocm_ranges 1 end]
    set concatenated_ranges {}
    foreach range $ocm_ranges {
	set base [lindex $range 0]
	set size [lindex $range 1]
	if { $base == [expr $base0 + $size0] } {
	    set size0 [expr $size0 + $size]
	    if { $base0 == 0xfffc0000 && $size0 >= $required_mem_size } return
	} else {
	    lappend concatenated_ranges [list $base0 $size0]
	    set base0 $base
	    set size0 $size
	}
    }

    # Check if any region is of required size
    lappend concatenated_ranges [list $base0 $size0]
    set concatenated_ranges [lsort -integer -index 0 $concatenated_ranges]
    foreach range $concatenated_ranges {
	set base [lindex $range 0]
	set size [lindex $range 1]
	if { $base == 0xfffc0000 && $size >= $required_mem_size } return
    }

    error "This application requires at least [expr $required_mem_size/1024] KB of OCM memory at 0xfffc0000 to run"
}

proc check_stdout_sw {} {
    set stdout [get_stdout];
    if { $stdout == "none" } {
        error "STDOUT parameter is not set for standalone OS. This application requires stdout to be set."
    }
}

proc swapp_is_supported_hw {} {

    # check processor type
    set proc_instance [hsi::get_sw_processor];
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc_instance]];

    if { $proc_type != "psu_cortexa53" } {
        error "This application is supported only on CortexA53 processors.";
    }

    # Check if the design has minimum memory for the program to run
    check_program_memory
    # check for uart peripheral
    check_stdout_hw;
}

proc swapp_is_supported_sw {} {
    # check for standalone OS
    check_standalone_os;

	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

	if {$processor_type == "psu_cortexa53"} {
		set procdrv [hsi::get_sw_processor]
		set compiler [::common::get_property CONFIG.compiler $procdrv]
		if {[string compare -nocase $compiler "arm-none-eabi-gcc"] == 0} {
			error "ERROR: This application is not supported for 32-bit A53";
			return;
        }
	}
	
    # check for stdout being set
    check_stdout_sw;
}

proc swapp_generate {} {
}

proc swapp_get_linker_constraints {} {
    # don't generate a linker script. dram_test has its own linker script
    return "lscript no";
}

proc swapp_get_supported_processors {} {
    return "psu_cortexa53";
}

proc swapp_get_supported_os {} {
    return "standalone";
}
