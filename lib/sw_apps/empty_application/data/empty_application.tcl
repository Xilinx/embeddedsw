###############################################################################
# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
##############################################################################

proc swapp_get_name {} {
    return "Empty Application(C)";
}

proc swapp_get_description {} {
    return "A blank C project.";
}

proc swapp_is_supported_hw {} {
   # check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

	if {($proc_type == "psu_microblaze")} {
		error "This application is not supported for PMU Microblaze processor (psu_pmu).";
	}

    return 1;
}

proc swapp_is_supported_sw {} {

    set oslist [hsi::get_os];

    if { [llength $oslist] != 1 } {
        return 0;
    }
    set os [lindex $oslist 0];

    set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

	if { $os == "freertos10_xilinx" && $processor_type == "psu_cortexa53"} {
		set procdrv [hsi::get_sw_processor]
		set compiler [::common::get_property CONFIG.compiler $procdrv]
		if {[string compare -nocase $compiler "arm-none-eabi-gcc"] == 0} {
			error "ERROR: FreeRTOS is not supported for 32bit A53";
			return;
		}
	}
	return 1;
}

proc swapp_generate {} {
}

proc swapp_get_linker_constraints {} {
    # we were not generating a linker script earlier (returning "noscript")
    # but CodeSourcery tools fail to link anything without a linker script
    return "";
}

proc swapp_get_supported_processors {} {
    return "microblaze ps7_cortexa9 psu_cortexa53 psu_cortexr5 psu_cortexa72 psv_cortexr5 psv_cortexa72 psxl_cortexa78 psxl_cortexr52 psx_cortexa78 psx_cortexr52 microblaze_riscv";
}

proc swapp_get_supported_os {} {
    return "standalone xilkernel freertos10_xilinx";
}
