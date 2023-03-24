#/******************************************************************************
#* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
#* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

set [namespace current]::memcfg "";

proc swapp_get_name {} {
    return "Memory Tests";
}

proc swapp_get_description {} {
    return "This application tests Memory Regions present in the hardware.";
}

proc generate_stdout_config { fid } {
    set stdout [get_stdout];
    set stdout [hsi::get_cells -hier $stdout]

    # if stdout is not uartns550, we don't have to generate anything
    set stdout_type [common::get_property IP_NAME $stdout];

    if { [regexp -nocase "uartlite" $stdout_type] || 
	 [string match -nocase "mdm" $stdout_type] ||
	 [regexp -nocase "ps7_uart" $stdout_type] ||
	 [regexp -nocase "iomodule" $stdout_type] } {
	return;
    } elseif { [regexp -nocase "uart16550" $stdout_type] } {
	# mention that we have a 16550
	puts $fid "#define STDOUT_IS_16550";

	# and note down its base address
	set prefix "XPAR_";
	set postfix "_BASEADDR";
	set stdout_baseaddr_macro $prefix$stdout$postfix;
	set stdout_baseaddr_macro [string toupper $stdout_baseaddr_macro];
	puts $fid "#define STDOUT_BASEADDR $stdout_baseaddr_macro";
    }
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
		if { $slave_type == "ps7_uart" || $slave_type == "psu_uart" || $slave_type == "axi_uartlite" ||
			 $slave_type == "axi_uart16550" || $slave_type == "iomodule" ||
			 $slave_type == "mdm" || $slave_type == "psv_sbsauart" || $slave_type == "psx_sbsauart" ||
			 $slave_type == "psxl_sbsauart"} {
			return;
		}
	}

	error "This application requires a Uart IP in the hardware."
}

proc check_stdout_sw {} {
    set stdout [get_stdout];
    if { $stdout == "none" } {
	error "STDOUT parameter is not set for standalone OS. Memory tests requires stdout to be set."
    }
}

proc get_mem_type { mem } {
    set mem_type [::hsi::utils::get_ip_sub_type [hsi::get_cells -hier $mem]]
    set base_addr [common::get_property BASE_VALUE [hsi::get_mem_ranges $mem]]
    if { $mem_type == "BRAM_CTRL" } {
	return "BRAM"
    }
    if { $mem_type == "OCM_CTRL" } {
	return "OCM"
    }
    if { $mem_type == "MEMORY_CNTLR"} {
       if { 0xFFFC0000 == [lindex $base_addr 0] || 0xBBF80000 == [lindex $base_addr 0] || 0xBBF00000 == [lindex $base_addr 0] } {
	return "OCM"
       }
    }
    return "OTHER"
}

proc get_mem_props {data type memlist} {
    upvar memdata $data
    foreach mem $memlist {
	set base [common::get_property BASE_VALUE $mem]
	set high [common::get_property HIGH_VALUE $mem]
	set base_name [common::get_property BASE_NAME $mem]
	set addr_block [common::get_property ADDRESS_BLOCK $mem]
	set id $mem
	if { $base_name != "" && $base_name != "C_BASEADDR" && $base_name != "C_S_AXI_BASEADDR" } {
	    append id "_$base_name"
	}
        if { $addr_block != "" } {
	    append id "_$addr_block"
        }
	set ip_type [common::get_property NAME $mem]
	set access [common::get_property ACCESS_TYPE $mem]
	if { $access == "" } { set access "Read/Write" }
	set tz [common::get_property TRUSTZONE $mem]
	dict set memdata $type $id [dict create base $base high $high mem $mem iptype $ip_type access $access tz $tz mtype [get_mem_type $mem]]
    }
}

# Create a dict with memory info
#	- dict keys = unique memory instance names (Instance name + Base Name + Address Block)
#	- key data  = base, high, HW memory instance, IP type, access type (RD/WR/RW), memory type (BRAM, OCM, etc), section type (CODE, DATA, etc)
proc get_mem_info { proc_instance } {
    if { [set [namespace current]::memcfg] != "" } {
        return [set [namespace current]::memcfg]
    }
    set memdata [dict create iranges {} dranges {} idranges {}]

    set imemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter { IS_INSTRUCTION == true && IS_DATA != true && MEM_TYPE == "MEMORY"}];
    set dmemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter { IS_INSTRUCTION != true && IS_DATA == true && MEM_TYPE == "MEMORY"}];
    set idmemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter { IS_INSTRUCTION == true && IS_DATA == true && MEM_TYPE == "MEMORY" }];
    get_mem_props memdata "iranges" $imemlist
    get_mem_props memdata "dranges" $dmemlist
    get_mem_props memdata "idranges" $idmemlist
    set [namespace current]::memcfg $memdata
    return $memdata
}

proc get_program_code_memory { proctype mem_ranges } {
    # check for a memory with required size
    set security [common::get_property CONFIG.security_state [::hsi::get_os]]
    set required_mem_size [get_required_mem_size]
    set bram_mem ""
    set ocm_mem ""
    set sec_bram_mem ""
    set sec_ocm_mem ""
    set imem_ranges [dict merge [dict get $mem_ranges iranges] [dict get $mem_ranges idranges]]
    dict for {mem data} $imem_ranges {
	set access [dict get $data access]
	if { $access != "Read/Write" } continue
	set tz [dict get $data tz]
	if { $security == "secure" && $tz == "Strict-NonSecure" } continue
	if { $security == "non-secure" && $tz == "Secure" } continue
	set base [dict get $data base]
	set high [dict get $data high]
	if { [expr $high - $base + 1] >= $required_mem_size } {
	    set memtype [dict get $data mtype]
	    if { $bram_mem == "" && $memtype == "BRAM" } {
		set bram_mem $mem
		if { $sec_bram_mem == "" && $security == "secure" && $tz == "Secure" } {
		    set sec_bram_mem $mem
		}
	    }
	    if { $ocm_mem == "" && $memtype == "OCM" } {
		set ocm_mem $mem
		if { $sec_ocm_mem == "" && $security == "secure" && $tz == "Secure" } {
		    set sec_ocm_mem $mem
		}
	    }
	}
    }

    if { $proctype == "microblaze" || $proctype == "microblaze_riscv"} {
	if { $bram_mem != "" } {
	    return $bram_mem
	} elseif { $ocm_mem != "" } {
	    return $ocm_mem
	}
    } else {
	if { $proctype == "psu_cortexa53" || $proctype == "psv_cortexa72" || $proctype == "psx_cortexa78" || $proctype == "psxl_cortexa78"} {
	    if { $sec_ocm_mem != "" } {
		return $sec_ocm_mem
	    }
	    if { $sec_bram_mem != "" } {
		return $sec_bram_mem
	    }
	}
	if { $ocm_mem != "" } {
	    return $ocm_mem
	} elseif { $bram_mem != "" } {
	    return $bram_mem
	}
    }

    error "This application requires atleast [expr $required_mem_size/1024] KB of BRAM/OCM to place code sections";
}

proc get_program_data_memory { proctype mem_ranges } {
    # check for a memory with required size
    set security [common::get_property CONFIG.security_state [::hsi::get_os]]
    set required_mem_size [get_required_mem_size]
    set bram_mem ""
    set ocm_mem ""
    set sec_bram_mem ""
    set sec_ocm_mem ""
    set dmem_ranges [dict merge [dict get $mem_ranges dranges] [dict get $mem_ranges idranges]]
    dict for {mem data} $dmem_ranges {
	set access [dict get $data access]
	if { $access != "Read/Write" } continue
	set tz [dict get $data tz]
	if { $security == "secure" && $tz == "Strict-NonSecure" } continue
	if { $security == "non-secure" && $tz == "Secure" } continue
	set base [dict get $data base]
	set high [dict get $data high]
	if { [expr $high - $base + 1] >= $required_mem_size } {
	    set memtype [dict get $data mtype]
	    if { $bram_mem == "" && $memtype == "BRAM" } {
		set bram_mem $mem
		if { $sec_bram_mem == "" && $security == "secure" && $tz == "Secure" } {
		    set sec_bram_mem $mem
		}
	    }
	    if { $ocm_mem == "" && $memtype == "OCM" } {
		set ocm_mem $mem
		if { $sec_ocm_mem == "" && $security == "secure" && $tz == "Secure" } {
		    set sec_ocm_mem $mem
		}
	    }
	}
    }

    if { $proctype == "microblaze" || $proctype == "microblaze_riscv"} {
	if { $bram_mem != "" } {
	    return $bram_mem
	} elseif { $ocm_mem != "" } {
	    return $ocm_mem
	}
    } else {
	if { $proctype == "psu_cortexa53" || $proctype == "psv_cortexa72" || $proctype == "psx_cortexa78" || $proctype == "psxl_cortexa78" } {
	    if { $sec_ocm_mem != "" } {
		return $sec_ocm_mem
	    }
	    if { $sec_bram_mem != "" } {
		return $sec_bram_mem
	    }
	}
	if { $ocm_mem != "" } {
	    return $ocm_mem
	} elseif { $bram_mem != "" } {
	    return $bram_mem
	}
    }

    error "This application requires atleast [expr $required_mem_size/1024] KB of BRAM/OCM to place data sections";
}

# for microblaze, we need 8k of memory.
# for microblaze_riscv, we need 8k of memory.
# for cortexa9, we need 64k of memory
proc get_required_mem_size {} {
    set proc_instance [hsi::get_sw_processor];
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc_instance]];
    if { $proc_type == "microblaze" || $proc_type == "microblaze_riscv" } {
	return "8192"
    } else {
	return "65536"
    }
}

proc swapp_is_supported_hw {} {
    set [namespace current]::memcfg "";

    # check for uart peripheral
    check_stdout_hw;
    set proc_instance [hsi::get_sw_processor];
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc_instance]];
    set memdata [get_mem_info $proc_instance]
    set code_mem [get_program_code_memory $proc_type $memdata]
    set data_mem [get_program_data_memory $proc_type $memdata]
}

proc swapp_is_supported_sw {} {
    # check for standalone OS
    check_standalone_os;

    # check for stdout being set
    check_stdout_sw;
}

proc generate_memory_config { fname } {
    set fp [open $fname "w+"];
    puts $fp "/* This file is automatically generated based on your hardware design. */";
    puts $fp "#include \"memory_config.h\"\n";

    set proc_instance [hsi::get_sw_processor];
    set exec_mode [common::get_property CONFIG.exec_mode [hsi::get_sw_processor]]
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc_instance]];
    set security [common::get_property CONFIG.security_state [::hsi::get_os]]

    # where did we store our program data
    set memdata [get_mem_info $proc_instance]
    set code_mem [get_program_code_memory $proc_type $memdata]
    set data_mem [get_program_data_memory $proc_type $memdata]
    set dmem_ranges [dict merge [dict get $memdata dranges] [dict get $memdata idranges]]
    set imem_ranges [dict merge [dict get $memdata iranges] [dict get $memdata idranges]]
    set code_base [dict get $imem_ranges $code_mem base]
    set data_base [dict get $dmem_ranges $data_mem base]
    set n_mem_ranges 0;
    puts $fp "struct memory_range_s memory_ranges\[\] = {";
    dict for {mem data} $dmem_ranges {
	# if this is a read-only memory or security settings do not match with the sw, we cannot use it for memory tests
	if { [dict exists $data access] && [dict get $data access] == "Read-only" } continue
	set tz [dict get $data tz]
	if { $security == "secure" && $tz == "Strict-NonSecure" } continue
	if { $security == "non-secure" && $tz == "Secure" } continue
	set mem_name $mem
	set mem_type [dict get $data mtype]
	set mem_ip   [dict get $data iptype]
	set mem_base [dict get $data base]
	set mem_high [dict get $data high]
	set mem_size [expr $mem_high - $mem_base + 1]

	# if this is the same place where we stored program, then we cannot use this memory
	if { $mem_base == $code_base || $mem_base == $data_base } {
	    puts $fp "\t/* $mem_name memory will not be tested since application resides in the same memory */";
	} elseif { [regexp -nocase "emc" $mem_ip] } {
	    # For EMC, skip mem_test if the memory connected is Flash.
	    # Determine the port num by matching the base addr of this
	    # memory against the base addr of memories connected to all
	    # the ports and then use this port num with C_MEMx_TYPE to
	    # detemine the memory type.
	    set emc_port 0
	    for {set i 0} {$i < 4} {incr i} {
		set _base [common::get_property [format CONFIG.C_S_AXI_MEM%d_BASEADDR $i] [hsi::get_cells -hier [dict get $data mem]]]
		if { $_base == $mem_base } {
		    set emc_port $i
		    break;
		}
	    }
	    set emc_type [common::get_property [format CONFIG.C_MEM%d_TYPE $emc_port] [hsi::get_cells -hier [dict get $data mem]]]
	    if { $emc_type == 2 } {
		puts $fp "\t/* $mem_name memory will not be tested since it looks like a flash memory */";
	    }
	} elseif { [regexp -nocase "ps7_ddrc" $mem_ip] } {
	    # no memory tests for ps7_ddrc
	} elseif { [regexp -nocase "ps7_qspi_linear" $mem_ip] ||
		   [regexp -nocase "ps7_nand" $mem_ip] || [regexp -nocase "ps7_nor" $mem_ip]  ||
		   [string match "psu_ocm" $mem_ip] || [regexp -nocase "psu_r5_0_atcm" $mem_ip] ||
		   [regexp -nocase "psv_r5_0_atcm_global" $mem_ip] ||
		   [regexp -nocase "psu_r5_0_atcm_lockstep" $mem_ip] || [regexp -nocase "psu_r5_0_btcm" $mem_ip]  ||
		   [regexp -nocase "psv_r5_1_atcm_global" $mem_ip] || [regexp -nocase "psv_r5_0_btcm_global" $mem_ip]  ||
		   [regexp -nocase "psu_r5_0_btcm_lockstep" $mem_ip] || [regexp -nocase "psu_r5_1_atcm" $mem_ip]  ||
		   [regexp -nocase "psv_r5_1_btcm_global" $mem_ip] || [regexp -nocase "psv_r5_tcm_ram_global" $mem_ip]  ||
		   [regexp -nocase "psx_r52_1a_atcm_global" $mem_ip] || [regexp -nocase "psx_r52_1a_btcm_global" $mem_ip]  ||
		   [regexp -nocase "psx_r52_1a_ctcm_global" $mem_ip] ||
		   [regexp -nocase "psx_ocm_ram_0" $mem_ip] || [regexp -nocase "psx_ocm_ram_1" $mem_ip]  ||
		   [regexp -nocase "psu_r5_1_btcm" $mem_ip] || [regexp -nocase "psu_pmu_ram" $mem_ip] ||
		   [regexp -nocase "psv_r5_0_data_cache" $mem_ip] || [regexp -nocase "psv_r5_1_data_cache" $mem_ip]  ||
		   [regexp -nocase "psv_lpd_afi_mem_0" $mem_ip] || [regexp -nocase "psv_fpd_afi_mem_2" $mem_ip] ||
		   [regexp -nocase "psv_fpd_afi_mem_0" $mem_ip] || [regexp -nocase "psv_r5_tcm_ram_global" $mem_ip] ||
		   [regexp -nocase "psu_bbram_0" $mem_ip] || [regexp -nocase "psu_ocm_xmpu_cfg" $mem_ip] } {
	    puts $fp "\t/* $mem_name memory will not be tested since it is a flash memory/non-writable memory */";
	} elseif { $exec_mode == "aarch32" && $mem_base > 0xFFFFFFFF} {
		puts $fp "\t/* $mem_name memory will not be tested since it base address is > 32-bit address */";	    
	} else {
	    # do processor specific changes
	    # The address range 0x0 - 0x50 is reserved for vector table and should not be overwritten
	    if { $proc_type == "microblaze" && $mem_base == 0x0 } {
		set mem_base [format "0x%08x" [expr $mem_base + 0x50]]
		set mem_size [expr $mem_size - 0x50]
	    }
	    puts $fp "\t{";
	    puts $fp "\t\t\"$mem_name\",";
	    puts $fp "\t\t\"$mem_ip\",";
	    puts $fp "\t\t$mem_base,";
	    puts $fp "\t\t$mem_size,";
	    puts $fp "\t},";

	    incr n_mem_ranges;
	}
    }
    puts $fp "};\n";

    puts $fp "int n_memory_ranges = $n_mem_ranges;";
    close $fp;
}

proc swapp_generate {} {
    # cleanup this file for writing
    set fid [open "platform_config.h" "w+"];
    puts $fid "#ifndef __PLATFORM_CONFIG_H_";
    puts $fid "#define __PLATFORM_CONFIG_H_";

    # if we have a uart16550 as stdout, then generate some config for that
    puts $fid "";
    generate_stdout_config $fid;

    puts $fid "#endif";
    close $fid;

    # generate memory configuration table 
    generate_memory_config "memory_config_g.c"
}

proc swapp_get_linker_constraints {} {
    set proc_instance [hsi::get_sw_processor];
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc_instance]];

    set memdata [get_mem_info $proc_instance]
    set code_mem [get_program_code_memory $proc_type $memdata]
    set data_mem [get_program_data_memory $proc_type $memdata]
    # set code & data memory to point to bram
    # no need for heap
    return "code_memory $code_mem data_memory $data_mem heap 0";
}

proc swapp_get_supported_processors {} {
    return "microblaze ps7_cortexa9 psu_cortexa53 psu_cortexr5 psv_cortexa72 psv_cortexr5 psxl_cortexa78 psxl_cortexr52 psx_cortexa78 psx_cortexr52 microblaze_riscv";
}

proc swapp_get_supported_os {} {
    return "standalone";
}
