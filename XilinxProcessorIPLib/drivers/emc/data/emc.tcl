###############################################################################
#
# Copyright (C) 2004 - 2014 Xilinx, Inc.  All rights reserved.
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
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
###############################################################################
# MODIFICATION HISTORY:
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
#        sdm 10/29/08 Added code to generate canonicals for EMC.
#        sdm 05/12/10 Added code for generating defines in xparameters.h to 
#	              distinguish  between XPS_MCH_EMC and AXI_EMC.
#                      These defines are used by the Xil_Flash Library
# 4.0   adk  10/12/13 Updated as per the New Tcl API's
################################################################################
#uses "xillib.tcl"


proc generate {drv_handle} {
    #---------------------------
    # #defines in xparameters.h
    #---------------------------
    xdefine_include_file $drv_handle "xparameters.h" "XEmc" "C_NUM_BANKS_MEM"

    #-------------------------------
    # memory_banks in xparameters.h
    #-------------------------------
    xdefine_include_file_membank $drv_handle "xparameters.h"

    #-----------------------------
    # canonicals for memory banks
    #-----------------------------
    xdefine_canonical_xpars $drv_handle "xparameters.h" "Emc" "C_NUM_BANKS_MEM"
}

#
# define a list of memory bank arguments in an include file.
#
proc xdefine_include_file_membank {drv_handle file_name} {

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips  $drv_handle]

    foreach periph $periphs {
        set addr_params ""
        set addr_params [::hsi::utils::find_addr_params $periph]
        ::hsi::utils::define_membank $periph $file_name $addr_params
    }
}

#
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all the peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips  $drv_handle]

    # Get the names of all the peripherals connected to this driver
    foreach periph $periphs {
        set peripheral_name [string toupper [common::get_property NAME $periph]]
        lappend peripherals $peripheral_name
    }

    # Get possible canonical names for all the peripherals connected to this driver
    set device_id 0
    foreach periph $periphs {
        set canonical_name [string toupper [format "%s_%s" $drv_string $device_id]]
        lappend canonicals $canonical_name
        
        # Create a list of IDs of the peripherals whose hardware instance name
        # doesn't match the canonical name. These IDs can be used later to
        # generate canonical definitions
        if { [lsearch $peripherals $canonical_name] < 0 } {
            lappend indices $device_id
        }
        incr device_id
    }

    set i 0
    foreach periph $periphs {
        set periph_name [string toupper [common::get_property NAME $periph]]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {

            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            # Generate canonical definitions for memory banks
            set addr_params ""
            set addr_params [::hsi::utils::find_addr_params $periph]
            set arguments [concat $args $addr_params]
            foreach arg $arguments {
                set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]

                # The commented out rvalue is the name of the instance-specific constant
                # set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
                # The rvalue set below is the actual value of the parameter
                set rvalue [common::get_property CONFIG.$arg $periph]
                if {[llength $rvalue] == 0} {
                    set rvalue 0
                }
                set rvalue [::hsi::utils::format_addr_string $rvalue $arg]

                puts $file_handle "#define $lvalue $rvalue"
            }

            if {$i == 0} {
                puts $file_handle ""
                set bus_name [::hsi::utils::get_intfnet_name $periph "SPLB"]
                if { [string compare -nocase $bus_name ""] != 0 } {
                    puts $file_handle "#define XPAR_XPS_MCH_EMC"
                } else {
                    set bus_name [::hsi::utils::get_intfnet_name $periph "S_AXI_MEM"]
                    if { [string compare -nocase $bus_name ""] != 0 } {
                        puts $file_handle "#define XPAR_AXI_EMC"
                    }
                }
            }
            incr i
        }
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}
