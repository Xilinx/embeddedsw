#/******************************************************************************
#* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
#* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

set use_softeth_on_zynq 0
proc swapp_get_name {} {
    return "lwIP TCP Perf Server";
}

proc swapp_get_description {} {
    return "The LwIP TCP Perf Server application is used for creating TCP server and measure downlink performance using light-weight IP stack (lwIP). This application sets up the board to use default IPv4 address 192.168.1.10 and IPv6 link local address when ipv6_enable is true, with MAC address 00:0a:35:00:01:02. The application creates TCP server on board and starts listening for TCP client connections. It will display client connection information with interim and average TCP statistics for data transfer."
}

proc check_stdout_hw {} {
    set slaves [common::get_property SLAVES [hsi::get_cells -hier [hsi::get_sw_processor]]]
    foreach slave $slaves {
        set slave_type [common::get_property IP_NAME [hsi::get_cells -hier $slave]];
        # Check for MDM-Uart peripheral. The MDM would be listed as a peripheral
        # # only if it has a UART interface. So no further check is required
        if { $slave_type in { "ps7_uart" "psu_uart" "axi_uartlite" "axi_uart16550"
			      "iomodule" "mdm" "psv_sbsauart" "psx_sbsauart"} } {
            return;
        }
    }

    error "This application requires a Uart IP in the hardware."

}

proc get_stdout {} {
    set os [hsi::get_os];
    set stdout [common::get_property CONFIG.STDOUT $os];
    return $stdout;
}

proc check_emac_hw {} {
    set temacs [hsi::get_cells -hier -filter { ip_name == "axi_ethernet" }];
    if { [llength $temacs] != 0 } {
        return;
    }

    set temacs [hsi::get_cells -hier -filter { ip_name == "ps7_ethernet" }];
    if { [llength $temacs] != 0 } {
            return;
    }

    set temacs [hsi::get_cells -hier -filter { ip_name == "psu_ethernet" }];
        if { [llength $temacs] != 0 } {
                return;
    }

    set temacs [hsi::get_cells -hier -filter { ip_name == "psv_ethernet" }];
        if { [llength $temacs] != 0 } {
                return;
    }

    set temacs [hsi::get_cells -hier -filter { ip_name == "psx_ethernet" }];
    if { [llength $temacs] != 0 } {
                return;
    }
    error "This application requires an Ethernet MAC IP instance in the hardware."
}

proc get_mem_size { memlist } {
    return [lindex $memlist 4];
}

proc require_memory {memsize} {
    set proc_instance [hsi::get_sw_processor]
    set imemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter "IS_INSTRUCTION==1"];
    set idmemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter "IS_INSTRUCTION==1 && IS_DATA==1"];
    set dmemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter "IS_DATA==1"];

    set memlist [concat $imemlist $idmemlist $dmemlist];

    while { [llength $memlist] > 3 } {
        set mem [lrange $memlist 0 4];
        set memlist [lreplace $memlist 0 4];

        if { [get_mem_size $mem] >= $memsize } {
            return 1;
        }
    }

    error "This application requires at least $memsize bytes of memory.";
}

proc check_stdout_sw {} {
    set stdout [get_stdout];
    if { $stdout == "none" } {
        error "The STDOUT parameter is not set on the OS. lwIP requires stdout to be set."
    }
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

proc swapp_is_supported_hw {} {
    # Check if Ethernet IP in the system
    check_emac_hw;

    # check for stdout being set
    check_stdout_hw;

    # do processor specific checks
    set proc  [hsi::get_sw_processor];
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc]]
    if { $proc_type == "microblaze"} {
        # make sure there is a timer (if this is a MB)
        set timerlist [hsi::get_cells -hier -filter { ip_name == "axi_timer" }];
        if { [llength $timerlist] <= 0 } {
              error "There seems to be no timer peripheral in the hardware. lwIP requires an axi_timer for TCP operations.";
        }
    }

    # psu_pmu is not supported
    if { $proc_type == "psu_pmu" || $proc_type == "psv_pmu"} {
        error "ERROR: lwip is not supported on PMU";
        return;
    }

    # require about 1M of memory
    require_memory "1000000";

    return 1;
}

proc swapp_is_supported_sw {} {
    # make sure we are using standalone OS
    check_standalone_os;

    set sw_processor [hsi::get_sw_processor]
    set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
    set processor_type [common::get_property IP_NAME $processor]

	if {$processor_type == {"psv_cortexa72" "psx_cortexa78"}} {
		set procdrv [hsi::get_sw_processor]
		set compiler [::common::get_property CONFIG.compiler $procdrv]
		if {[string compare -nocase $compiler "arm-none-eabi-gcc"] == 0} {
			error "ERROR: lwip library does not support 32 bit A72 compiler";
		return;
            }
	}

    # check for stdout being set
    check_stdout_sw;

    # make sure lwip213 is available
    set librarylist [hsi::get_libs -filter "NAME==lwip213"];

    if { [llength $librarylist] == 0 } {
        error "This application requires lwIP library in the Board Support Package.";
    } elseif { [llength $librarylist] > 1} {
        error "Multiple lwIP libraries present in the Board Support Package."
    }

    return 1;
}

proc generate_stdout_config { fid } {
    set stdout [get_stdout];
    set stdout [hsi::get_cells -hier $stdout]

    # if stdout is uartlite, we don't have to generate anything
    set stdout_type [common::get_property IP_TYPE $stdout];

    if { [regexp -nocase "uartlite" $stdout_type] ||
     [regexp -nocase "ps7_uart" $stdout_type] ||
     [string match -nocase "mdm" $stdout_type] } {
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

proc generate_emac_config {fp} {
    global use_softeth_on_zynq

    # FIXME we'll just use the first emac we find. This is not consistent with
    # how lwIP determines the EMAC's that can be used.

    set proc  [hsi::get_sw_processor];
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc]]

    set temacs [hsi::get_cells -hier -filter { ip_name == "axi_ethernet" }];
    if { [llength $temacs] > 0 } {
        if {$proc_type == "ps7_cortexa9" && $use_softeth_on_zynq == 0} {
        } else {
            if {$proc_type == "ps7_cortexa9" && $use_softeth_on_zynq == 1} {
                puts $fp "#define USE_SOFTETH_ON_ZYNQ 1";
            }
            set temac [lindex $temacs 0]
            set prefix "XPAR_";
            set postfix "_BASEADDR";
            set emac_baseaddr $prefix$temac$postfix;
            set emac_baseaddr [string toupper $emac_baseaddr];
            puts $fp "#define PLATFORM_EMAC_BASEADDR $emac_baseaddr";
            return;
        }
    }

    set temacs [hsi::get_cells -hier -filter { ip_name == "ps7_ethernet" }];
    if { [llength $temacs] > 0 } {
            puts $fp "#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR";
            return;
    }

    set temacs [hsi::get_cells -hier -filter { ip_name == "psu_ethernet" }];
        if { [llength $temacs] > 0 } {
                puts $fp "#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR";
                return;
    }

    set temacs [hsi::get_cells -hier -filter { ip_name == "psv_ethernet" }];
        if { [llength $temacs] > 0 } {
                puts $fp "#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR";
                return;
    }
    set temacs [hsi::get_cells -hier -filter { ip_name == "psx_ethernet" }];
    if { [llength $temacs] > 0 } {
                puts $fp "#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR";
                return;
    }
}

proc generate_timer_config { fp } {
    # generate something like: XPAR_XPS_INTC_0_XPS_TIMER_1_INTERRUPT_INTR
    set prefix "XPAR_";
    set postfix_intr "_INTERRUPT_INTR";
    set postfix_base "_BASEADDR";

    set intcs [hsi::get_cells -hier -filter {ip_name == "xps_intc"}];
    if { [llength $intcs] == 0 } {
        set intcs [hsi::get_cells -hier -filter { ip_name == "axi_intc" }];
    }
    set intc [lindex $intcs 0];

    set timers [hsi::get_cells -hier -filter { ip_name == "axi_timer" }];
    set timer [lindex $timers 0];

    # baseaddr
    set timer_baseaddr $prefix$timer$postfix_base;
    set timer_baseaddr [string toupper $timer_baseaddr];

    # intr
    set uscore "_"
    set timer_intr $prefix$intc$uscore$timer$postfix_intr;
    set timer_intr [string toupper $timer_intr];

    puts $fp "#define PLATFORM_TIMER_BASEADDR $timer_baseaddr";
    puts $fp "#define PLATFORM_TIMER_INTERRUPT_INTR $timer_intr";
    puts $fp "#define PLATFORM_TIMER_INTERRUPT_MASK (1 << $timer_intr)";
}


# depending on the type of os (standalone|xilkernel), choose
# the correct source files
proc swapp_generate {} {
    global use_softeth_on_zynq
    # cleanup this file for writing
    set fid [open "platform_config.h" "w+"];
    puts $fid "#ifndef __PLATFORM_CONFIG_H_";
    puts $fid "#define __PLATFORM_CONFIG_H_\n";

    # if we have a uart16550 as stdout, then generate some config for that
    generate_stdout_config $fid;
    puts $fid "";

    set use_softeth_on_zynq [common::get_property CONFIG.use_axieth_on_zynq [hsi::get_libs lwip213]];
    # figure out the emac baseaddr
    generate_emac_config $fid;
    puts $fid "";

    # if MB, figure out the timer to be used
    set proc  [hsi::get_sw_processor];
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc]]

    if { $proc_type == "microblaze"} {
        generate_timer_config $fid;
        puts $fid "";
    }

    set hw_processor [common::get_property HW_INSTANCE $proc]
    set proc_arm [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
    if { $proc_arm == "ps7_cortexa9"} {
        puts $fid "#define PLATFORM_ZYNQ \n";
    } elseif { $proc_arm == "psu_cortexr5" || $proc_arm == "psu_cortexa53"} {
        puts $fid "#define PLATFORM_ZYNQMP \n";
    } elseif { $proc_arm == "psv_cortexr5" || $proc_arm == "psv_cortexa72" } {
	puts $fid "#define PLATFORM_VERSAL \n";
    } elseif { $proc_arm == "psx_cortexr52" || $proc_arm == "psx_cortexa78" } {
	puts $fid "#define PLATFORM_VERSAL_NET \n";
    }
    puts $fid "";

    puts $fid "#endif";
    close $fid;
}

proc swapp_get_linker_constraints {} {
    return "stack 40k heap 40k"
}

proc swapp_get_supported_processors {} {

    return "psx_cortexa78 psx_cortexr52 psv_cortexa72 psv_cortexr5 psu_cortexa53 psu_cortexr5 ps7_cortexa9 microblaze";
}

proc swapp_get_supported_os {} {

    return "standalone";
}
