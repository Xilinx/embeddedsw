#/******************************************************************************
#* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
#* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

proc swapp_get_name {} {
    return "Zynq DRAM tests";
}

proc swapp_get_description {} {
    return "This application runs out of OCM and performs memory tests and read/write eye measurements on Zynq DRAM. \
    For more information about the test, refer to ZYNQ_DRAM_DIAGNOSTICS_TEST.docx, in the src directory of the application";
}

proc generate_stdout_config { fid } {
    set stdout [get_stdout];

    # if stdout is not uartns550, we don't have to generate anything
    set stdout_type [common::get_property IP_NAME [hsi::get_cells -hier $stdout]];

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
		if { $slave_type == "ps7_uart" || $slave_type == "axi_uartlite" ||
			 $slave_type == "axi_uart16550" || $slave_type == "iomodule" ||
			 $slave_type == "mdm" } {
			return;
		}
	}

	error "This application requires a Uart IP in the hardware."

#    set p7_uarts [hsi::get_cells -filter { ip_name == "ps7_uart" }];
#    set uartlites [hsi::get_cells -filter { ip_name == "axi_uartlite" }];
#    set uart16550s [hsi::get_cells -filter { ip_name == "axi_uart16550" }];
#    set mcs_iomodule [hsi::get_cells -filter { ip_name == "iomodule" }];
#    if { ([llength $p7_uarts] == 0) && ([llength $uartlites] == 0) &&
#	 ([llength $uart16550s] == 0) && ([llength $mcs_iomodule] == 0) } {
#        # Check for MDM-Uart peripheral. The MDM would be listed as a peripheral
#        # only if it has a UART interface. So no further check is required
#        set mdmlist [hsi::get_cells -filter { ip_name == "mdm" }]
#        if { [llength $mdmlist] == 0 } {
#	    error "This application requires a Uart IP in the hardware."
#        }
#    }
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

    if { $proc_type != "ps7_cortexa9" } {
        error "This application is supported only on CortexA9 processors.";
    }

    # check for uart peripheral
    check_stdout_hw;
}

proc swapp_is_supported_sw {} {
    # check for standalone OS
    check_standalone_os;

    # check for stdout being set
    check_stdout_sw;
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
}

proc swapp_get_linker_constraints {} {
    # don't generate a linker script. dram_test has its own linker script
    return "lscript no";
}

proc swapp_get_supported_processors {} {
    return "ps7_cortexa9";
}

proc swapp_get_supported_os {} {
    return "standalone";
}
