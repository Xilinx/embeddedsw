#/******************************************************************************
#* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
#* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


proc swapp_get_name {} {
    return "DDR self refresh";
}

proc swapp_get_description {} {
    return "This DDR self refresh application provides a simple demonstration of how to enter to/exit from DDR self refresh mode. This application runs on R5 out of TCM.";
}

proc get_stdout {} {
    set os [hsi::get_os]
    if { $os == "" } {
        error "No Operating System specified in the Board Support Package.";
    }
    set stdout [common::get_property CONFIG.STDOUT $os];
    return $stdout;
}

proc check_stdout_hw {} {
        # check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

	if { ( $proc_type != "psu_cortexr5" ) } {
                error "This application is supported only for CortexR5 processor.";
	}

	set slaves [common::get_property SLAVES [hsi::get_cells -hier [hsi::get_sw_processor]]]
	foreach slave $slaves {
		set slave_type [common::get_property IP_NAME [hsi::get_cells -hier $slave]];
		# Check for MDM-Uart peripheral. The MDM would be listed as a peripheral
		# only if it has a UART interface. So no further check is required
		if { $slave_type == "ps7_uart" ||  $slave_type == "psu_uart" || $slave_type == "axi_uartlite" ||
			 $slave_type == "axi_uart16550" || $slave_type == "iomodule" ||
			 $slave_type == "mdm" } {
			return;
		}
	}

	error "This application requires a Uart IP in the hardware."
}

proc check_stdout_sw {} {
    set stdout [get_stdout];
    if { $stdout == "none" } {
        error "The STDOUT parameter is not set on the OS. Application requires stdout to be set."
    }
}

proc swapp_is_supported_hw {} {
    # check for uart peripheral
    check_stdout_hw;

    return 1;
}

proc check_standalone_os {} {
    set oslist [hsi::get_os];

    if { [llength $oslist] != 1 } {
        return 0;
    }
    set os [lindex $oslist 0];

    if { $os != "standalone" } {
        error "This application is supported only on the Standalone Board Support Package.";
    }
}

proc swapp_is_supported_sw {} {
    # check for stdout being set
    check_stdout_sw;
    # make sure we are using standalone OS
    check_standalone_os;

    # make sure xilpm is available
    set lib_needed "xilpm"
    set lib_list [hsi::get_libs];
    if {[lsearch $lib_list $lib_needed] < 0 } {
        error "Xilpm library required by ddr_self_refresh application is missing in Board Support Package"
    }
}

proc generate_stdout_config { fid } {
    set stdout [get_stdout];
    set stdout [hsi::get_cells -hier $stdout]

    # if stdout is uartlite, we don't have to generate anything
    set stdout_type [common::get_property IP_NAME $stdout];

    if { [regexp -nocase "uartlite" $stdout_type] || [string match -nocase "mdm" $stdout_type] } {
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
    } elseif { [regexp -nocase "ps7_uart" $stdout_type] } {
	# mention that we have a ps7_uart
        puts $fid "#define STDOUT_IS_PS7_UART";

        # and get it device id
        set p7_uarts [lsort [hsi::get_cells -hier -filter { ip_name == "ps7_uart"} ]];
        set id 0
        foreach uart $p7_uarts {
            if {[string compare -nocase $uart $stdout] == 0} {
		puts $fid "#define UART_DEVICE_ID $id"
		break;
	    }
	    incr id
	}
    } elseif { [regexp -nocase "psu_uart" $stdout_type] } {
	# mention that we have a psu_uart
        puts $fid "#define STDOUT_IS_PSU_UART";
        # and get it device id
        set p8_uarts [lsort [hsi::get_cells -filter { ip_name == "psu_uart"} ]];
        set id 0
        foreach uart $p8_uarts {
            if {[string compare -nocase $uart $stdout] == 0} {
		puts $fid "#define UART_DEVICE_ID $id"
		break;
	    }
	    incr id
	}
    }
}

proc swapp_generate {} {
    set os [hsi::get_os];
    if { $os == "" } {
        error "No Operating System specified in the Board Support Package.";
    }

    # cleanup this file for writing
    set fid [open "platform_config.h" "w+"];
    puts $fid "#ifndef __PLATFORM_CONFIG_H_";
    puts $fid "#define __PLATFORM_CONFIG_H_\n";

    # if we have a uart16550/ps7_uart as stdout, then generate some config for that
    generate_stdout_config $fid;

    puts $fid "#endif";
    close $fid;
}

proc swapp_get_linker_constraints {} {
    # don't generate a linker script, we provide one
    return "lscript no"
}

proc swapp_get_supported_processors {} {
    return "psu_cortexr5";
}

proc swapp_get_supported_os {} {
    return "standalone";
}
