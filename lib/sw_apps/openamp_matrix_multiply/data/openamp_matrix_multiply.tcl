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
#* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
    return "OpenAMP matrix multiplication Demo";
}

proc swapp_get_description {} {
    return " OpenAMP R5 matrix multiplication application";
}

proc check_standalone_os {} {
    set oslist [get_os];

    if { [llength $oslist] != 1 } {
        return 0;
    }
    set os [lindex $oslist 0];

    if {{ $os != "standalone" } || { $os != "freertos821_xilinx" }} {
        error "This application is supported only on the Standalone Board Support Package and freertos821.";
    }
}

proc swapp_is_supported_sw {} {
    # make sure we are using standalone OS
    check_standalone_os;

	# make sure xilffs is available
    set librarylist [hsi::get_libs -filter "NAME==xilopenamp"];

	if { [llength $librarylist] == 0 } {
        error "This application requires xilopenamp library in the Board Support Package.";
    } elseif { [llength $librarylist] > 1} {
        error "Multiple xilopenamp libraries present in the Board Support Package."
    }
}

proc swapp_is_supported_hw {} {

    # check processor type
    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

    if { $proc_type != "psu_cortexr5" } {
                error "This application is supported only for CortexR5 processors.";
    }

    return 1;
}


proc get_stdout {} {
    return;
}

proc check_stdout_hw {} {
    return;
}

proc swapp_generate {} {
	set oslist [get_os];
	if { [llength $oslist] != 1 } {
		return 0;
	}
	set os [lindex $oslist 0];
	if { $os != "standalone" } {
		set ld_file "lscript.ld"
		set ld_file_new "lscript_freertos.ld"
		file rename -force $ld_file_new $ld_file
		file delete -force $ld_file_new
	} else {
		set ld_file "lscript_freertos.ld"
		file delete -force $ld_file
	}
	return;
}

proc swapp_get_linker_constraints {} {

    # don't generate a linker script. fsbl has its own linker script
    return "lscript no";
}
