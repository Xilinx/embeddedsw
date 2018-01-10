#

proc swapp_get_name {} {
    return "FreeRTOS BSP (4) LWIP Echo !";
}

proc swapp_get_description {} {
    return "This demo is also based on previous demos (PS GPIO and AXI Timer), while it enables the capability of LwIP TCP/IP. Here, we only use the socket mode in this simple example. It consist a listen thread binded to the 27015 port. It receive any size of PC input (<4096 Bytes) and tries to write a large chunk ( 4*(65536 + 1024) bytes ) every 100 ms. For this demo, I use 'math.h' and you need to link math, i.e., '-lm' in project settings."
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

