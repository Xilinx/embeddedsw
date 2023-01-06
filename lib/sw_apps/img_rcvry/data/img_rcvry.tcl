#/******************************************************************************
#* Copyright (c) 2020 - 2022 Xilinx, Inc. All rights reserved.
#* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

proc swapp_get_name {} {
    return "Image Recovery";
}

proc swapp_get_description {} {
    return "Image Recovery tool which writes \
	user selected images on the board.";
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

    set lib_list_missing ""
    set has_missing_libs 0

    # make sure xilffs and lwip213 are available
    set lib_needed "xilffs lwip213"
    set lib_list [hsi::get_libs];

    foreach libs ${lib_needed} {
        # create a list of required libs that are not in BSP
        if {[lsearch $lib_list $libs] < 0 } {
            lappend lib_list_missing $libs
            set has_missing_libs [expr $has_missing_libs + 1]
        }
    }

    if {$has_missing_libs > 0} {
        error "These libraries which Image recovery requires are missing in Board Support Package: $lib_list_missing"
    }
}

proc swapp_is_supported_hw {} {

    set str_nodsgn ""
    set has_nodsgn_ips 0
    # check processor type
    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    if { $proc_instance != "psu_cortexa53_0" } {
                error "This application is supported only for CortexA53_0.";
    }
    set ip_list "psu_ethernet_* psu_ttc_* psu_qspi_* psu_gpio_*"
    set ip_nodsgn [::hsi::get_cells -hier]
    # create a list of IPs that are not in design
    foreach ips ${ip_list} {
        if {[lsearch $ip_nodsgn $ips] < 0 } {
            lappend ip_list_nodsgn $ips
            set has_nodsgn_ips [expr $has_nodsgn_ips + 1]
        }
    }
    if {$has_nodsgn_ips > 0} {
        set str_nodsgn "Image Recovery app uses these IPs but are missing in the design: $ip_list_nodsgn"
    }
    if { [llength $str_nodsgn] != 0 } {
        error $str_nodsgn
    }

    return 1;
}


proc get_stdout {} {
    set os [hsi::get_os];
    set stdout [common::get_property CONFIG.STDOUT $os];
    return $stdout;
}

proc check_stdout_hw {} {
    set psu_uart [hsi::get_cells -hier -filter "IP_NAME=psu_uart"];
}

proc swapp_generate {} {
    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

    # get the compiler flags, if set already
    set def_flags [common::get_property APP_COMPILER_FLAGS [hsi::current_sw_design]]
    set def_link_flags [common::get_property APP_LINKER_FLAGS [hsi::current_sw_design]]

    set new_flags "-Wall -fmessage-length=0 -DARMA53_64 -O2 $def_flags"

    set new_link_flags "-n $def_link_flags"

    # Update compiler and linker flags
    common::set_property -name {APP_COMPILER_FLAGS} -value $new_flags -objects [hsi::current_sw_design]
    common::set_property -name {APP_LINKER_FLAGS} -value $new_link_flags -objects [hsi::current_sw_design]
}

proc swapp_get_linker_constraints {} {

    # don't generate a linker script. Image Recovery tool  has its own linker script
    return "lscript no";
}
