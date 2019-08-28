###############################################################################
#
# Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  cjp  09/05/17 First commit
# 1.1   Nava   04/20/18 Fixed compilation warnings.
# 1.2   cjp  04/27/18 Updated for clockps interdependency
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    xdefine_zynq_include_file $drv_handle "xparameters.h" "XResetPs" "NUM_INSTANCES" "DEVICE_ID"

    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XResetPs" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

    xdefine_zynq_canonical_xpars $drv_handle "xparameters.h" "XResetPs" "DEVICE_ID"

    xdefine_pmu_addr $drv_handle "xparameters.h"
}

#
# Given a list of arguments, define them all in an include file.
# Since resetps and clockps has common hardware, generalized defines
# for both are handled here
#
proc xdefine_zynq_include_file {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set uSuffix "U"
    set num_instance 1
    set device_id 0
    set baseaddr 0xFFFFFFFF

    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
	puts $file_handle "/* Definitions for driver RESETPS and CLOCKPS */"
	puts $file_handle "#define XPAR_XCRPSU_NUM_INSTANCES $num_instance$uSuffix"
	set args [lreplace $args $posn $posn]
    }

    set arg "DEVICE_ID"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
	puts $file_handle "\n/* Definitions for peripheral PSU_CR_0 */"
	puts $file_handle "#define XPAR_PSU_CR_DEVICE_ID $device_id"
	set args [lreplace $args $posn $posn]
    }

    puts $file_handle "\n/******************************************************************/"
    close $file_handle
}

#
# Given a list of arguments, define them all in an include file.
# Since resetps and clockps has common hardware, generalized defines
# for both are handled here
#
proc xdefine_zynq_canonical_xpars {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set device_id 0

    set arg "DEVICE_ID"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
	puts $file_handle "/* Canonical definitions for peripheral PSU_CR_0 */"
	puts $file_handle "#define XPAR_XCRPSU_0_DEVICE_ID $device_id"
	set args [lreplace $args $posn $posn]
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

#
#  PMU specific register defines
#
proc xdefine_pmu_addr {drv_handle file_name} {
    set file_handle [::hsi::utils::open_include_file $file_name]
    set ips [hsi::get_cells -hier "*"]
    set sw_proc [hsi::get_sw_processor]

    foreach ip $ips {
        set periph [common::get_property IP_NAME  $ip]
        if { [string compare -nocase $periph "psu_pmu_iomodule"] == 0 ||
             [string compare -nocase $periph "psu_lpd_slcr"] == 0 } {
            set name [string toupper [common::get_property IP_NAME $ip]]
            set baddr [common::get_property CONFIG.C_S_AXI_BASEADDR $ip]
            set haddr [common::get_property CONFIG.C_S_AXI_HIGHADDR $ip]
            puts $file_handle "\n/* Definitions for peripheral $name */"
            puts $file_handle "#define XPAR_${name}_S_AXI_BASEADDR $baddr"
            puts $file_handle "#define XPAR_${name}_S_AXI_HIGHADDR $haddr\n"
        }
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}
