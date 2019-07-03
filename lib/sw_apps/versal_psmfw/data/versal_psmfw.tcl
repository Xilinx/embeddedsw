#/******************************************************************************
#*
#* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
#* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
#* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#* THE SOFTWARE.
#*
#*
#*
#******************************************************************************/

proc swapp_get_name {} {
	return "versal PSM Firmware";
}

proc swapp_get_description {} {
	return "Processing System Management Unit Firmware for versal.";
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
	return 1;
}

proc swapp_is_supported_hw {} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

	if {($proc_type != "psu_psm") && ($proc_type != "psv_psm")} {
		error "This application is supported only for PSM Microblaze processor.";
	}

	return 1;
}

proc get_stdout {} {
	set os [hsi::get_os];
	set stdout [common::get_property CONFIG.STDOUT $os];
	return $stdout;
}

proc swapp_generate {} {
	# Get the compiler flags, if set already
	set def_flags [common::get_property APP_COMPILER_FLAGS [hsi::current_sw_design]]
	set new_flags "-mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v10.0 -mxl-soft-mul $def_flags"
	# Set PSM Microblaze HW related compiler flags
	set_property -name APP_COMPILER_FLAGS -value $new_flags -objects [current_sw_design]
}

proc swapp_get_linker_constraints {} {
	# don't generate a linker script. PSM Firmware has its own linker script
	return "lscript no";
}

proc swapp_get_supported_processors {} {
	return "psu_psm psv_psm";
}

proc swapp_get_supported_os {} {
	return "standalone";
}
