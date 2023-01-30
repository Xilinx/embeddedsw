#/******************************************************************************
#* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
#* Copyright (c) 2023, Advanced Micro Devices, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


proc swapp_get_name {} {
	return "versal PLM";
}

proc swapp_get_description {} {
	return "Platform Loader and Manager for versal and versal net.";
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

    # make sure all required libraries are available
    set lib_needed "xilffs xilpdi xilplmi xilloader xilpm xilocp xilcert xilsecure xilpuf"
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

	if {($proc_type != "psu_pmc") && ($proc_type != "psv_pmc") && ($proc_type != "psxl_pmc") && ($proc_type != "psx_pmc")} {
		error "This application is supported only for PMC Microblaze processor.";
	}

	# List of all IPIs on SoC
	set ipi_list [hsi::get_cells -hier -filter { IP_NAME == "psu_ipi" || IP_NAME == "psv_ipi" || IP_NAME == "psxl_ipi" || IP_NAME == "psx_ipi"}]
	set a7x_proc [string map {psv_pmc psv_cortexa72 psxl_pmc psxl_cortexa78 psx_pmc psx_cortexa78} $proc_instance]
	set ipi_a7x [ipi_find_cpu $ipi_list CONFIG.C_CPU_NAME $a7x_proc]
	if {($ipi_a7x == "")} {
		 puts "APU IPIs are not enabled. Linux boot would not work."
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

	foreach entry [glob -nocomplain -types f [file join . *]] {
		file delete -force $entry
	}

	foreach entry [glob -nocomplain -types f [file join $common *]] {
		file copy -force $entry "."
	}
	if {$proc_type == "psxl_pmc" || $proc_type == "psx_pmc"} {
		foreach entry [glob -nocomplain -types f [file join $versal_net *]] {
			file copy -force $entry "."
		}
	}
	if {$proc_type == "psv_pmc"} {
		foreach entry [glob -nocomplain -types f [file join $versal *]] {
			file copy -force $entry "."
		}
	}
	file delete -force $versal_net
	file delete -force $common
	file delete -force $versal

	# disable global optimizations through --no-relax flag
	set def_link_flags [common::get_property APP_LINKER_FLAGS [hsi::current_sw_design]]
	set new_link_flags "-Wl,--no-relax "
	append new_link_flags $def_link_flags
	common::set_property -name {APP_LINKER_FLAGS} -value $new_link_flags -objects [hsi::current_sw_design]

	set def_flags [common::get_property APP_COMPILER_FLAGS [hsi::current_sw_design]]
	set new_flags " -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare"
	append new_flags " -mno-xl-soft-div"
	if {$proc_type == "psxl_pmc" || $proc_type == "psx_pmc"} {
		append new_flags " -mcpu=v11.0"
	} elseif {$proc_type == "psv_pmc"} {
		append new_flags " -mcpu=v10.0"
	}
	append new_flags " -mno-xl-soft-mul -mxl-multiply-high -Os -flto -ffat-lto-objects"
	append new_flags $def_flags
	# Set PMC Microblaze HW related compiler flags
	set_property -name APP_COMPILER_FLAGS -value $new_flags -objects [current_sw_design]
}

proc swapp_get_linker_constraints {} {
	# don't generate a linker script. PLM has its own linker script
	return "lscript no";
}

proc swapp_get_supported_processors {} {
	return "psu_pmc psv_pmc psxl_pmc psx_pmc";
}

proc swapp_get_supported_os {} {
	return "standalone";
}

proc ipi_find_cpu {ipi_list param hw_proc} {
	set proc_ipi_slave ""
	foreach ipi $ipi_list {
		set param_name [string range $param [string length "CONFIG."] [string length $param]]
		set param_value [common::get_property $param [hsi::get_cells -hier $ipi]]
		set ip_name [common::get_property IP_NAME [hsi::get_cells -hier $hw_proc]]
		set trimmed_param [string trim [string trim $param_value "[0-15]"] "_"]
		if { [string match -nocase "*$trimmed_param*" $ip_name] } {
			lappend proc_ipi_slave $ipi
		}
	}
	return $proc_ipi_slave
}
