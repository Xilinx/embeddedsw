#/******************************************************************************
#*
#* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
#*
#* Permission is hereby granted, free of charge, to any person obtaining a copy
#* of this software and associated documentation files (the "Software"), to deal
#* in the Software without restriction, including without limitation the rights
#* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#* copies of the Software, and to permit persons to whom the Software is
#* furnished to do so, subject to the following conditions:
#*
#* The above copyright notice and this permission notice shall be included in
#* all copies or substantial portions of the Software.
#*
#* Use of the Software is limited solely to applications:
#* (a) running on a Xilinx device, or
#* (b) that interact with a Xilinx device through a bus or interconnect.
#*
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
#* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#* SOFTWARE.
#*
#* Except as contained in this notice, the name of the Xilinx shall not be used
#* in advertising or otherwise to promote the sale, use or other dealings in
#* this Software without prior written authorization from Xilinx.
#*
#******************************************************************************/

proc swapp_get_name {} {
    return "Zynq MP FSBL";
}

proc swapp_get_description {} {
    return "First Stage Bootloader (FSBL) for Zynq MP. The FSBL configures the FPGA with HW bit stream (if it exists) \
	and loads the Operating System (OS) Image or Standalone (SA) Image or 2nd Stage Boot Loader image from the \
	non-volatile memory (NAND/SD/QSPI) to RAM (DDR) and takes A5x/R5 out of reset.  It supports multiple partitions, \
	and each partition can be a code image or a bit stream.";
}

proc check_standalone_os {} {
    set oslist [get_os];

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

	# make sure xilffs is available
    set librarylist [get_libs -filter "NAME==xilffs"];

	if { [llength $librarylist] == 0 } {
        error "This application requires xilffs library in the Board Support Package.";
    } elseif { [llength $librarylist] > 1} {
        error "Multiple xilffs libraries present in the Board Support Package."
    }
}

proc swapp_is_supported_hw {} {

    # check processor type
    set proc_instance [get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    set proc_type [common::get_property IP_NAME [get_cells $hw_processor]];

    if { $proc_type != "psu_cortexr5" && $proc_type != "psu_cortexa53" } {
                error "This application is supported only for CortexA53/CortexR5 processors.";
    }

    return 1;
}


proc get_stdout {} {
    set os [get_os];
    set stdout [common::get_property CONFIG.STDOUT $os];
    return $stdout;
}

proc check_stdout_hw {} {
    set pu_uarts [get_cells -filter "IP_NAME=psu_uart"];
}

proc swapp_generate {} {
    # generate/copy ps init files
    ::hsi::utils::generate_psinit

    #delete unnecessary files (only psu_init.c & psu_init.h are needed for FSBL)

    set files(0) "psu_init.html"
    set files(1) "psu_init.tcl"
    set files(2) "psu_init_gpl.c"
    set files(3) "psu_init_gpl.h"
    set files(4) "psu_pmucfg.c"

    foreach init_file [array get files] {
        file delete -force $init_file
    }
}

proc swapp_get_linker_constraints {} {

    # don't generate a linker script. fsbl has its own linker script
    return "lscript no";
}
