#/******************************************************************************
#* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

proc swapp_get_name {} {
    return "Image Selector";
}

proc swapp_get_description {} {
    return "ImgSel for Zynq Ultrascale+ MPSoC. The Image Selector \
	selects the image based on configuration parameters";
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

    # check processor type
    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    if { $proc_instance != "psu_cortexa53_0" } {
                error "This application is supported only for CortexA53_0.";
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

    # don't generate a linker script. ImgSel has its own linker script
    return "lscript no";
}