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
    return "OpenAMP RPC Demo"
}

proc swapp_get_description {} {
    return " OpenAMP rpc-demo application "
}

proc check_oamp_supported_os {} {
    set oslist [hsi::get_os]

    if { [llength $oslist] != 1 } {
        return 0
    }
    set os [lindex $oslist 0]

    if { ( $os != "standalone" ) && ( [string match -nocase "freertos*" "$os"] == 0 ) } {
        error "This application is supported only on the Standalone and FreeRTOS Board Support Packages"
    }
}

proc swapp_is_supported_sw {} {
    # make sure we are using a supported OS
    check_oamp_supported_os

    # make sure openamp and metal libs are available
    set librarylist_1 [hsi::get_libs -filter "NAME==openamp"]
    set librarylist_2 [hsi::get_libs -filter "NAME==libmetal"]

    if { ([llength $librarylist_1] == 0) || ([llength $librarylist_2] == 0) } {
        error "This application requires OpenAMP and Libmetal libraries in the Board Support Package."
    } elseif { [llength $librarylist_1] > 1 } {
        error "Multiple OpenAMP  libraries present in the Board Support Package."
    } elseif { [llength $librarylist_2] > 1 } {
        error "Multiple Libmetal libraries present in the Board Support Package."
    }

}

proc swapp_is_supported_hw {} {
    # check processor type
    set proc_instance [hsi::get_sw_processor]
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]]

    if { ( $proc_type != "psu_cortexr5" ) && ( $proc_type != "psv_cortexr5" ) && ( $proc_type != "ps7_cortexa9" ) } {
        error "This application is supported only for Cortex-R5 and Cortex-A9 processors."
    }

    return 1
}

proc get_stdout {} {
    return
}

proc check_stdout_hw {} {
    return
}

proc swapp_generate {} {
    set oslist [get_os]
    if { [llength $oslist] != 1 } {
        return 0
    }
    set os [lindex $oslist 0]

    set proc_instance [hsi::get_sw_processor]
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]]

    if { $os == "standalone" } {
        set osdir "generic"
    } elseif { [string match -nocase "freertos*" "$os"] > 0 } {
        set osdir "freertos"
    } else {
        error "Invalid OS: $os"
    }

    if { $proc_type == "psu_cortexr5" ||  $proc_type == "psv_cortexr5"} {
        set procdir "zynqmp_r5"
    } elseif { $proc_type == "ps7_cortexa9" } {
        set procdir "zynq7"
    } else {
        error "Invalid processor type: $proc_type"
    }

    # development support option: set this to 1 in order to link files to your development local repo
    set linkfiles 0
    # if using linkfiles=1, set the path below to your local repo
    set local_repo_app_src "your_path_here/.../lib/sw_apps/openamp_rpc_demo/src"

    foreach entry [glob -nocomplain -type f [file join machine *] [file join machine $procdir *] [file join system *] [file join system $osdir *] [file join system $osdir machine *] [file join system $osdir machine $procdir *]] {
        if { $linkfiles } {
            file link -symbolic [file tail $entry] [file join $local_repo_app_src $entry]
        } else {
            file copy -force $entry "."
        }
    }

    file delete -force "machine"
    file delete -force "system"

    return
}

proc swapp_get_linker_constraints {} {
    # don't generate a linker script, we provide one
    return "lscript no"
}

proc swapp_get_supported_processors {} {
    return "psu_cortexr5 psv_cortexr5 ps7_cortexa9"
}

proc swapp_get_supported_os {} {
    return "freertos10_xilinx standalone"
}
