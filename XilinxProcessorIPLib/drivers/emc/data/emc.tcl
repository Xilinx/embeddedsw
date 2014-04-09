##############################################################################
#
# (c) Copyright 2004-2014 Xilinx, Inc. All rights reserved.
#
# This file contains confidential and proprietary information of Xilinx, Inc.
# and is protected under U.S. and international copyright and other
# intellectual property laws.
#
# DISCLAIMER
# This disclaimer is not a license and does not grant any rights to the
# materials distributed herewith. Except as otherwise provided in a valid
# license issued to you by Xilinx, and to the maximum extent permitted by
# applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
# FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
# MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
# and (2) Xilinx shall not be liable (whether in contract or tort, including
# negligence, or under any other theory of liability) for any loss or damage
# of any kind or nature related to, arising under or in connection with these
# materials, including for any direct, or any indirect, special, incidental,
# or consequential loss or damage (including loss of data, profits, goodwill,
# or any type of loss or damage suffered as a result of any action brought by
# a third party) even if such damage or loss was reasonably foreseeable or
# Xilinx had been advised of the possibility of the same.
#
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-safe, or for use in
# any application requiring fail-safe performance, such as life-support or
# safety devices or systems, Class III medical devices, nuclear facilities,
# applications related to the deployment of airbags, or any other applications
# that could lead to death, personal injury, or severe property or
# environmental damage (individually and collectively, "Critical
# Applications"). Customer assumes the sole risk and liability of any use of
# Xilinx products in Critical Applications, subject only to applicable laws
# and regulations governing limitations on product liability.
#
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
# AT ALL TIMES.
#
#
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
    set periphs [xget_sw_iplist_for_driver  $drv_handle] 

    foreach periph $periphs {
        set addr_params ""
        set addr_params [xfind_addr_params $periph]
        xdefine_membank $periph $file_name $addr_params
    }
}

#
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [xopen_include_file $file_name]

    # Get all the peripherals connected to this driver
    set periphs [xget_sw_iplist_for_driver  $drv_handle]

    # Get the names of all the peripherals connected to this driver
    foreach periph $periphs {
        set peripheral_name [string toupper [get_property NAME $periph]]
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
        set periph_name [string toupper [get_property NAME $periph]]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {

            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            # Generate canonical definitions for memory banks
            set addr_params ""
            set addr_params [xfind_addr_params $periph]
            set arguments [concat $args $addr_params]
            foreach arg $arguments {
                set lvalue [xget_dname $canonical_name $arg]

                # The commented out rvalue is the name of the instance-specific constant
                # set rvalue [xget_name $periph $arg]
                # The rvalue set below is the actual value of the parameter
                set rvalue [get_property CONFIG.$arg $periph]
                if {[llength $rvalue] == 0} {
                    set rvalue 0
                }
                set rvalue [xformat_addr_string $rvalue $arg]

                puts $file_handle "#define $lvalue $rvalue"
            }

            if {$i == 0} {
                puts $file_handle ""
                set bus_name [xget_hw_busif_value $periph "SPLB"]
                if { [string compare -nocase $bus_name ""] != 0 } {
                    puts $file_handle "#define XPAR_XPS_MCH_EMC"
                } else {
                    set bus_name [xget_hw_busif_value $periph "S_AXI_MEM"]
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
