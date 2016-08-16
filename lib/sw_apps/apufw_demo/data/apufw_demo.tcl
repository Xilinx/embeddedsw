proc swapp_get_name {} {
    return "APUFW Arithmetic Demo";
}

proc swapp_get_description {} {
    return "The APUFW_demo application is a Secure EL1 A53 application. It is an example of APUFW library demostrating usage of xilinx secure payload.
    It takes arithmetic operation arguments from non-secure EL1 application, process and send back the result through ATF";
}

proc swapp_get_supported_processors {} {
    return "psu_cortexa53";
}

proc swapp_get_supported_os {} {
    return "standalone";
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
    # make sure we are using standalone OS
    check_standalone_os;
}

proc swapp_is_supported_hw {} {
	return;
}


proc get_stdout {} {
	return;
}

proc check_stdout_hw {} {
    return;
}

proc swapp_generate {} {
    # check processor type
    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
    set procdrv [::hsi::get_sw_processor]
    set exec_mode [common::get_property CONFIG.exec_mode $procdrv];
    set os [hsi::get_os];
    set el [common::get_property CONFIG.exception_level $os];
    set secstate [common::get_property CONFIG.security_state $os];

    if {( $proc_type != "psu_cortexa53" ) || ($exec_mode != "aarch64") || ($el != "EL1") || ($secstate != "secure")} {
                error "This application is supported only for CortexA53 64bit EL1 secure.";
    }

    return 1;
}

proc swapp_get_linker_constraints {} {
   return;
}
