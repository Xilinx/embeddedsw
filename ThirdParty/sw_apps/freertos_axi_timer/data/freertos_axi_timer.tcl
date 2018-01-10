#

proc swapp_get_name {} {
    return "FreeRTOS BSP (3) AXI Timer !";
}

proc swapp_get_description {} {
    return "This demo is based on previous demos (PS GPIO and Hello), while instead of using soft timers in 'hello, world !', it uses an AXI timer to generate an interrupt, then acknowledge this interrupt to toggle an LED indicator. So, from this simple demo, you may learn howto use AXI Timer, handle interrupt, and write/read the PS GPIO."
}

proc check_os {} {
    set oslist [get_os];

    if { [llength $oslist] != 1 } {
        return 0;
    }
    set os [lindex $oslist 0];

    if { $os != "freertos_zynq" } {
        error "This application is supported only on FreeRTOS based Board Support Packages.";
    }
}

proc get_stdout {} {
    set os [get_os]
    if { $os == "" } {
        error "No Operating System specified in the Board Support Package.";
    }
    set stdout [get_property CONFIG.STDOUT $os];
    return $stdout;
}

proc check_stdout_hw {} {
}

proc check_stdout_sw {} {
}

proc swapp_is_supported_hw {} {
    return 1;
}

proc swapp_is_supported_sw {} {
    return 1;
}

proc swapp_generate {} {

}

