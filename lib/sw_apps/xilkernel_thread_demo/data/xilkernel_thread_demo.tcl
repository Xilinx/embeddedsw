proc swapp_get_name {} {
    return "Xilkernel POSIX Threads Demo";
}

proc swapp_get_description {} {
    return "This Xilkernel based application provides a simple example of how to create multiple POSIX threads and synchronize with them when they are complete. This example creates an initial master thread. The master thread creates 4 worker threads that go off to compute parts of a sum and return the partial sum as the result. The master thread accumulates the partial sums and prints the result. This example can serve as your starting point for your end application thread structure."
}

proc get_stdout {} {
    set os [hsi::get_os];
    set stdout [common::get_property CONFIG.STDOUT $os];
    return $stdout;
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

#    set uartlites [hsi::get_cells -hier -filter { ip_name == "axi_uartlite" }];
#    if { [llength $uartlites] == 0 } {
#        # we do not have an uartlite
#	set uart16550s [hsi::get_cells -hier -filter {ip_name == "axi_uart16550"}];
#	if { [llength $uart16550s] == 0 } {      
#	    # Check for MDM-Uart peripheral. The MDM would be listed as a peripheral
#	    # only if it has a UART interface. So no further check is required
#	    set mdmlist [hsi::get_cells -hier -filter {ip_name == "mdm"}]
#	    if { [llength $mdmlist] == 0 } {
#		error "This application requires a Uart IP in the hardware."
#	    }
#	}
#    }
}


proc check_stdout_sw {} {
    set stdout [get_stdout];
    if { $stdout == "none" } {
        error "The STDOUT parameter is not set on the OS. This demo requires STDOUT to be set."
    }
}

proc check_xilkernel_os {} {
    set oslist [hsi::get_os];

    if { [llength $oslist] != 1 } {
        return 0;
    }
    set os [lindex $oslist 0];

    if { $os != "xilkernel" } {
        error "This application is supported only on Xilkernel based Board Support Packages.";
    }
}

proc swapp_is_supported_hw {} {
    # xilkernel is supported only for microblaze target
    set proc [hsi::get_sw_processor];
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc]]

    if { [string match -nocase $proc_type "microblaze"] != 1} {
	error "This application is supported only for MicroBlaze processor";
    }

    # check for uart peripheral
    check_stdout_hw;

    if { $proc_type == 1} {
        # make sure there is a timer (if this is a MB)
        set timerlist [hsi::get_cells -hier -filter {ip_name == "xps_timer"}];
        if { [llength $timerlist] <= 0 } {
            set timerlist [hsi::get_cells -hier -filter {ip_name == "axi_timer"}];
            if { [llength $timerlist] <= 0 } {
                error "There seems to be no timer peripheral in the hardware. Xilkernel requires a timer for operation.";
            }
        }
    }

    return 1;
}

proc swapp_is_supported_sw {} {

    # make sure we are using the Xilkernel OS
    check_xilkernel_os;

    # check for stdout being set
    check_stdout_sw;

    set n_threads [common::get_property CONFIG.max_pthreads [hsi::get_os]];
    if { $n_threads < 6 } {
        error "This application requires that your Xilkernel OS be configured to support at-least 6 POSIX threads. Currently, it is configured to support only $n_threads threads."
    }

    return 1;
}

proc generate_stdout_config { fid } {
    set stdout [get_stdout];

    # if stdout is uartlite, we don't have to generate anything
    set stdout_type [common::get_property IP_NAME [hsi::get_cells -hier $stdout]];

    if { [regexp -nocase "uartlite" $stdout_type] || [string match -nocase "mdm" $stdout_type] } {
        puts $fid "#define STDOUT_IS_UARTLITE";
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

# depending on the type of os (standalone|xilkernel), choose
# the correct source files
proc swapp_generate {} {
    # cleanup this file for writing
    set fid [open "platform_config.h" "w+"];
    puts $fid "#ifndef __PLATFORM_CONFIG_H_";
    puts $fid "#define __PLATFORM_CONFIG_H_\n";

    # if we have a uart16550 as stdout, then generate some config for that
    generate_stdout_config $fid;

    puts $fid "";
    puts $fid "#endif";
    close $fid;
}

proc swapp_get_linker_constraints {} {
    return "stack 2k heap 2k"
}

proc swapp_get_supported_processors {} {
    return "microblaze";
}

proc swapp_get_supported_os {} {
    return "xilkernel";
}
