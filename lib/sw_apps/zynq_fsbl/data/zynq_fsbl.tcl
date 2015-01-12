proc swapp_get_name {} {
    return "Zynq FSBL";
}

proc swapp_get_description {} {
    return "First Stage Bootloader (FSBL) for Zynq. The FSBL configures the FPGA with HW bit stream (if it exists) \
	and loads the Operating System (OS) Image or Standalone (SA) Image or 2nd Stage Boot Loader image from the \
	non-volatile memory (NAND/NOR/QSPI) to RAM (DDR) and starts executing it.  It supports multiple partitions, \
	and each partition can be a code image or a bit stream.";
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
    set librarylist_2 [hsi::get_libs -filter "NAME==xilrsa"];

	if { [llength $librarylist_1] == 0 && [llength $librarylist_2] == 0 } {
        error "This application requires xilffs and xilrsa libraries in the Board Support Package.";
    } elseif { [llength $librarylist_1] == 0 } {
        error "This application requires xilffs library in the Board Support Package.";
    } elseif { [llength $librarylist_2] == 0 } {
        error "This application requires xilrsa library in the Board Support Package.";
    }

}

proc swapp_is_supported_hw {} {

    # check processor type
    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    set proc_type [common::get_property IP_NAME [hsi::get_cells $hw_processor]];
    
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
    set p7_uarts [hsi::get_cells -filter "IP_NAME=ps7_uart"];
}

proc swapp_generate {} {
    # generate/copy ps init files
    ::hsi::utils::generate_psinit

    #delete unnecessary files (only ps7_init.c & ps7_init.h are needed for FSBL)
    set file "ps7_init.html"
    if { [file exists $file] } {
      file delete -force $file
    }

    set file "ps7_init.tcl"
    if { [file exists $file] } {
      file delete -force $file
    }

    set file "ps7_init_gpl.c"
    if { [file exists $file] } {
      file delete -force $file
    }

    set file "ps7_init_gpl.h"
    if { [file exists $file] } {
      file delete -force $file
    }

}

proc swapp_get_linker_constraints {} {

    # don't generate a linker script. fsbl has its own linker script
    return "lscript no";
}
