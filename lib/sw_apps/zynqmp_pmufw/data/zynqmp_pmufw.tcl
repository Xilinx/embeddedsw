#/******************************************************************************
#* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


proc swapp_get_name {} {
	return "ZynqMP PMU Firmware";
}

proc swapp_get_description {} {
	return "Platform Management Unit Firmware for ZynqMP.";
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

	if {($proc_type != "psu_pmu")} {
		error "This application is supported only for PMU Microblaze processor (psu_pmu).";
	}

	return 1;
}

proc get_stdout {} {
	set os [hsi::get_os];
	set stdout [common::get_property CONFIG.STDOUT $os];
	return $stdout;
}

proc swapp_generate {} {
	# PMU Firmware uses its own startup file. so set the -nostartfiles flag
	set_property  -name APP_LINKER_FLAGS -value {-nostartfiles -Wl,--no-relax} -objects [hsi::current_sw_design]
	# Get the compiler flags, if set already
	set def_flags [common::get_property APP_COMPILER_FLAGS [hsi::current_sw_design]]
	set new_flags "-mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v9.2 -mxl-soft-mul -Os -flto -ffat-lto-objects $def_flags"
	# Set PMU Microblaze HW related compiler flags
	set_property -name APP_COMPILER_FLAGS -value $new_flags -objects [current_sw_design]
}

proc swapp_get_linker_constraints {} {
	# don't generate a linker script. PMU Firmware has its own linker script
	return "lscript no";
}

proc swapp_get_supported_processors {} {
	return "psu_pmu";
}

proc swapp_get_supported_os {} {
	return "standalone";
}
