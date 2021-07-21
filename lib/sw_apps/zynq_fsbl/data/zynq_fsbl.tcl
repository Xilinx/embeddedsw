#/******************************************************************************
#* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/

proc swapp_get_name {} {
    return "Zynq FSBL";
}

proc swapp_get_description {} {
    return "First Stage Bootloader (FSBL) for Zynq. The FSBL configures the FPGA with HW bit stream (if it exists) \
	and loads the Operating System (OS) Image or Standalone (SA) Image or 2nd Stage Boot Loader image from the \
	non-volatile memory (NAND/NOR/QSPI) to RAM (DDR) and starts executing it.  It supports multiple partitions, \
	and each partition can be a code image or a bit stream.";
}

proc swapp_get_supported_processors {} {
    return "ps7_cortexa9";
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
    #check_standalone_os;

	# make sure xilffs and xilrsa libraries are available

    set librarylist_1 [hsi::get_libs -filter "NAME==xilffs"];

    if { [llength $librarylist_1] == 0 } {
        error "This application requires xilffs library in the Board Support Package.";
	}
}

proc swapp_is_supported_hw {} {

    # check processor type
    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
    
    if { $proc_type != "ps7_cortexa9" } {
                error "This application is supported only for CortexA9 processors.";
    }

    return 1;
}


proc get_stdout {} {
    set os [hsi::get_os];
    set stdout [common::get_property CONFIG.STDOUT $os];
    return $stdout;
}

proc check_stdout_hw {} {
    set p7_uarts [hsi::get_cells -hier -filter "IP_NAME=ps7_uart"];
}

proc swapp_generate {} {
    # generate/copy ps init files
    ::hsi::utils::generate_psinit

    #delete unnecessary files (only ps7_init.c & ps7_init.h are needed for FSBL)

    set files(0) "ps7_init.html"
    set files(1) "ps7_init.tcl"
    set files(2) "ps7_init_gpl.c"
    set files(3) "ps7_init_gpl.h"

    foreach init_file [array get files] {
        file delete -force $init_file
    }

}

proc swapp_get_linker_constraints {} {

    # don't generate a linker script. fsbl has its own linker script
    return "lscript no";
}
