#/******************************************************************************
#* Copyright (c) 2018-2022 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
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

	if {($proc_type != "psu_psm") && ($proc_type != "psv_psm") && ($proc_type != "psxl_psm") && ($proc_type != "psx_psm")} {
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
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set versal_net "versal_net/"
	set versal "versal/"
	set common "common/"
	if {$proc_type == "psxl_psm" || $proc_type == "psx_psm"} {
		foreach entry [glob -nocomplain -types f [file join . *]] {
			file delete -force $entry
		}
		foreach entry [glob -nocomplain -types f [file join $versal_net *]] {
			file copy -force $entry "."
		}
		foreach entry [glob -nocomplain -types f [file join $common *]] {
			file copy -force $entry "."
		}
	}
	if {$proc_type == "psv_psm"} {
		foreach entry [glob -nocomplain -types f [file join . *]] {
			file delete -force $entry
		}
		foreach entry [glob -nocomplain -types f [file join $versal *]] {
			file copy -force $entry "."
		}
		foreach entry [glob -nocomplain -types f [file join $common *]] {
			file copy -force $entry "."
		}
	}
	file delete -force $versal_net
	file delete -force $versal
	file delete -force $common
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
	return "psu_psm psv_psm psxl_psm psx_psm";
}

proc swapp_get_supported_os {} {
	return "standalone";
}
