#/******************************************************************************
#* Copyright (c) 2015 - 2022 Xilinx, Inc.  All rights reserved.
#** Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

package require fileutil

proc swapp_get_name {} {
    return "Libmetal AMP Demo"
}

proc swapp_get_description {} {
    return "Libmetal AMP Application"
}

proc check_oamp_supported_os {} {
    set oslist [hsi::get_os]

    if { [llength $oslist] != 1 } {
        return 0
    }
    set os [lindex $oslist 0]

    if { ( $os != "standalone" ) && ( [string match -nocase "freertos*" "$os"] == 0 ) } {
        error "This application is supported only on the Standalone and FreeRTOS Board Support Packages"
    }
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

	set slaves [common::get_property SLAVES [hsi::get_cells -hier [hsi::get_sw_processor]]]
	foreach slave $slaves {
		set slave_type [common::get_property IP_NAME [hsi::get_cells -hier $slave]];
		# Check for MDM-Uart peripheral. The MDM would be listed as a peripheral
		# only if it has a UART interface. So no further check is required
		if { $slave_type == "psu_uart" || $slave_type == "axi_uartlite" ||
			 $slave_type == "axi_uart16550" || $slave_type == "iomodule" ||
			 $slave_type == "mdm" || $slave_type == "psv_sbsauart" } {
			return;
		}
	}

	error "This application requires a Uart IP in the hardware."
}

proc check_stdout_sw {} {
    set stdout [get_stdout];
    if { $stdout == "none" } {
        error "The STDOUT parameter is not set on the OS. This application requires stdout to be set."
    }
}
proc swapp_is_supported_sw {} {
    # make sure we are using a supported OS
    check_oamp_supported_os

    # make sure libmetal is available
    set librarylist [hsi::get_libs -filter "NAME==libmetal"]

    if { [llength $librarylist] == 0 } {
        error "This application requires libmetal library in the Board Support Package."
    } elseif { [llength $librarylist] > 1 } {
        error "Multiple libmetal libraries present in the Board Support Package."
    }
    check_stdout_sw
}

proc swapp_is_supported_hw {} {
    # check processor type
    set proc_instance [hsi::get_sw_processor]
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]]

    if { ($proc_type != "psu_cortexr5") && ($proc_type != "psv_cortexr5") &&
    ( $proc_type != "psxl_cortexr52" ) && ( $proc_type != "psx_cortexr52" ) } {
        error "This application is supported only for Cortex-R5 and Cortex-R52 processors."
    }

    check_stdout_hw
    return 1
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
    } elseif { [regexp -nocase "psu_uart" $stdout_type] } {
	# mention that we have a psu_uart
        puts $fid "#define STDOUT_IS_PSU_UART";
        # and get it device id
        set p8_uarts [lsort [hsi::get_cells -hier -filter { ip_name == "psu_uart"} ]];
        set id 0
        foreach uart $p8_uarts {
            if {[string compare -nocase $uart $stdout] == 0} {
				puts $fid "#define UART_DEVICE_ID $id"
				break;
			}
			incr id
		}
    } elseif { [regexp -nocase "psv_sbsauart" $stdout_type] } {
	# mention that we have a psv__sbsauart
        puts $fid "#define STDOUT_IS_PSV_SBSAUART";
        # and get it device id
        set p8_uarts [lsort [hsi::get_cells -hier -filter { ip_name == "psv_sbsauart"} ]];
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
    set oslist [get_os]
    if { [llength $oslist] != 1 } {
        return 0
    }
    set os [lindex $oslist 0]

    set proc_instance [hsi::get_sw_processor]
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]]

    if { $os == "standalone" } {
        set osdir "generic"
    } elseif { [string match -nocase "freertos*" "$os"] > 0 } {
        set osdir "freertos"
    } else {
        error "Invalid OS: $os"
    }

    if { $proc_type == "psu_cortexr5" || $proc_type == "psv_cortexr5" || $proc_type == "psxl_cortexr52" || $proc_type == "psx_cortexr52" } {
        set procdir "zynqmp_r5"
    } else {
        error "Invalid processor type: $proc_type"
    }

       foreach entry [glob -nocomplain -type f [file join machine *] [file join machine $procdir *] [file join system *] [file join system $osdir *] [file join system $osdir $procdir zynqmp_amp_demo *] [file join system $osdir machine *] [file join system $osdir machine $procdir *]] {
	file copy -force $entry "."
    }

    # cleanup this file for writing
    set fid [open "platform_config.h" "w+"];
    puts $fid "#ifndef __PLATFORM_CONFIG_H_";
    puts $fid "#define __PLATFORM_CONFIG_H_\n";

    # if we have a uart16550/ps7_uart as stdout, then generate some config for that
    generate_stdout_config $fid;

    puts $fid "#endif";
    close $fid;

    file delete -force "machine"
    file delete -force "system"
    file delete -force "sdt"

    return
}

proc swapp_get_linker_constraints {} {
    # don't generate a linker script, we provide one
    return "lscript no"
}

proc swapp_get_supported_processors {} {
    return "psu_cortexr5 psv_cortexr5 psxl_cortexr52 psx_cortexr52"
}

proc swapp_get_supported_os {} {
    return "freertos10_xilinx standalone"
}
