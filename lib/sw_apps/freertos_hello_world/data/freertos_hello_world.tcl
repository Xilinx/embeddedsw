#/******************************************************************************
#*
#* Copyright (C) 2015 - 2018 Xilinx, Inc.  All rights reserved.
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
    return "FreeRTOS Hello World";
}

proc swapp_get_description {} {
    return " FreeRTOS Hello World application";
}

proc check_freertos_os {} {
    set oslist [hsi::get_os];

    if { [llength $oslist] != 1 } {
        return 0;
    }
    set os [lindex $oslist 0];

    if { $os != "freertos10_xilinx" } {
        error "This application is supported only on the freertos10_xilinx.";
    }
}

proc swapp_is_supported_sw {} {

    check_freertos_os

    return 1;
}

proc swapp_is_supported_hw {} {

    # check processor type
    set proc_instance [::hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
    set procdrv [::hsi::get_sw_processor]
    if {[string compare -nocase $proc_type "psu_cortexa53"] == 0} {
	set compiler [common::get_property CONFIG.compiler $procdrv]
	if {[string compare -nocase $compiler "arm-none-eabi-gcc"] == 0} {
		error "ERROR: FreeRTOS is not supported for 32bit A53"
	}
    }
    if { $proc_type != "psu_cortexr5" && $proc_type != "psv_cortexr5" && $proc_type != "ps7_cortexa9" && $proc_type != "psu_cortexa53" && $proc_type != "microblaze" && $proc_type != "psv_cortexa72" } {
                error "This application is supported only for CortexR5/CortexA9/CortexA53/MicroBlaze/CortexA72 processors.";
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
    return;
}

proc swapp_get_linker_constraints {} {
    return "";
}

proc swapp_get_supported_processors {} {
    return "psu_cortexr5 psv_cortexr5 ps7_cortexa9 psu_cortexa53 microblaze psv_cortexa72";
}

proc swapp_get_supported_os {} {
    return "freertos10_xilinx";
}
