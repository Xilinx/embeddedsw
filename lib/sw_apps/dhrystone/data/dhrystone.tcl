#/******************************************************************************
#* Copyright (c) 2004 - 2021 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

proc swapp_get_name {} {
    return "Dhrystone";
}

proc swapp_get_description {} {
    return "Dhrystone synthetic benchmark program for baremetal environment. Not supported for Microblaze in 2021.1.";
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

proc check_stdout_hw {} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

	if {($proc_type == "psu_microblaze")} {
		error "This application is not supported for PMU Microblaze processor (psu_microblaze).";
	}
	
	if {($proc_type == "microblaze")} {
		error "This application is not supported for Microblaze processor as of now.";
	}
	
	set slaves [common::get_property SLAVES [hsi::get_cells -hier [hsi::get_sw_processor]]]
	foreach slave $slaves {
		set slave_type [common::get_property IP_NAME [hsi::get_cells -hier $slave]];
		if { $slave_type == "ps7_uart" || $slave_type == "psu_uart"  || $slave_type == "psv_sbsauart" } {
			return;
		}
	}
	
	error "This application requires a Uart IP in the hardware."	
}

proc check_stdout_sw {} {
    set stdout [get_stdout];
    if { $stdout == "none" } {
        error "The STDOUT parameter is not set on the OS. Dhrystone requires stdout to be set."
    }
}

proc require_memory {memsize} {
    # obtain a unique list if "I", "D" and "ID" memories
    set proc_instance [hsi::get_sw_processor];
    set imemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter { IS_INSTRUCTION == true && MEM_TYPE == "MEMORY"}];
    set idmemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter { IS_INSTRUCTION == true && IS_DATA == true && MEM_TYPE == "MEMORY" }];
    set dmemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter { IS_DATA == true && MEM_TYPE == "MEMORY" }];

    # concatenate "I", "D" and "ID" memories
    set memlist [concat $imemlist $idmemlist $dmemlist];
    # create a unique list
    set unique_memlist {}
    foreach mem $memlist {
        set match [lsearch $unique_memlist $mem ]
        if { $match == -1 } {
            lappend unique_memlist $mem
        }
    }

    # check for a memory with required size
    foreach mem $unique_memlist {
        set base [common::get_property BASE_VALUE $mem]
        set high [common::get_property HIGH_VALUE $mem]
        if { [expr $high - $base + 1] >= $memsize } {
            return 1;
        }
    }

    error "This application requires atleast $memsize bytes of memory.";
}

proc swapp_is_supported_hw {} {
    # check for uart peripheral
    check_stdout_hw;

    # we require atleast 30k memory
    require_memory "30000";
}

proc swapp_is_supported_sw {} {
    # check for standalone OS
    check_standalone_os;

    # check for stdout being set
    check_stdout_sw;
}

proc swapp_generate {} {
# To be filled up once Microblaze support is added
}

proc swapp_get_linker_constraints {} {
    # we need a 4k stack
    return "stack 16k heap 16k"
}

proc swapp_get_supported_processors {} {
    return "psu_cortexa53 ps7_cortexa9 psv_cortexa72 psu_cortexr5 psv_cortexr5";
}

proc swapp_get_supported_os {} {
    return "standalone";
}
