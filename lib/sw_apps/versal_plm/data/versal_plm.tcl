#/******************************************************************************
#*
#* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#* THE SOFTWARE.
#*
#*
#*
#******************************************************************************/

proc swapp_get_name {} {
	return "versal PLM";
}

proc swapp_get_description {} {
	return "Platform Loader and Manager for versal.";
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

    # make sure xilffs and xilpdi are available
    set lib_needed "xilffs xilpdi"
    set lib_list [hsi::get_libs];

    foreach libs ${lib_needed} {
        # create a list of required libs that are not in BSP
        if {[lsearch $lib_list $libs] < 0 } {
            lappend lib_list_missing $libs
            set has_missing_libs [expr $has_missing_libs + 1]
        }
    }

    if {$has_missing_libs > 0} {
        error "These libraries which PLM requires are missing in Board Support Package: $lib_list_missing"
    }
}

proc swapp_is_supported_hw {} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

	if {($proc_type != "psu_pmc") && ($proc_type != "psv_pmc")} {
		error "This application is supported only for PMC Microblaze processor.";
	}

	return 1;
}

proc get_stdout {} {
	set os [hsi::get_os];
	set stdout [common::get_property CONFIG.STDOUT $os];
	return $stdout;
}

proc swapp_generate {} {
	set def_flags [common::get_property APP_COMPILER_FLAGS [hsi::current_sw_design]]
	set new_flags "-mlittle-endian -mxl-barrel-shift -mxl-pattern-compare"
	append new_flags " -mno-xl-soft-div -mcpu=v10.0 -mno-xl-soft-mul -mxl-multiply-high "
	append new_flags $def_flags
	# Set PMC Microblaze HW related compiler flags
	set_property -name APP_COMPILER_FLAGS -value $new_flags -objects [current_sw_design]
}

proc swapp_get_linker_constraints {} {
	# don't generate a linker script. PLM has its own linker script
	return "lscript no";
}

proc swapp_get_supported_processors {} {
	return "psu_pmc psv_pmc";
}

proc swapp_get_supported_os {} {
	return "standalone";
}
