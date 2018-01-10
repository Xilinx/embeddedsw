#

proc swapp_get_name {} {
    return "FreeRTOS BSP (2) GPIO !";
}

proc swapp_get_description {} {
    return "This demo can use both AXI GPIO or PS GPIO to control an LED (output) or a Button (input) ! It is based on (and slightly modified from) the 'hello, world' demo we had create before and also uses a software timer. The driver is 'freertos_gpio.c' and it supports both AXI- or PS- GPIO, but if you want to use AXI GPIO, you should re-create the block design and manually add the AXI GPIO IP. PS GPIO is the default. On microzed, the LED is at pin (47) while the button is at pin (51), modify the settings according to your needs.";
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

