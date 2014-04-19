##############################################################################
#
# (c) Copyright 2014 Xilinx, Inc. All rights reserved.
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
##############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   adk  16/04/14 Initial release 
#
##############################################################################

#uses "xillib.tcl"

set periph_config_params_srio 0
set periph_ninstances_srio    0

proc init_periph_config_struct_srio { deviceid } {
    global periph_config_params_srio
    set periph_config_params_srio($deviceid) [list]
}

proc add_field_to_periph_config_struct_srio { deviceid fieldval } {
    global periph_config_params_srio
    lappend periph_config_params_srio($deviceid) $fieldval
}

proc get_periph_config_struct_fields_srio { deviceid } {
    global periph_config_params_srio
    return $periph_config_params_srio($deviceid)
}

proc xdefine_srio_include_file {drv_handle file_name drv_string} {
	global periph_ninstances
	
	    # Open include file
	    set file_handle [xopen_include_file $file_name]
	
	    # Get all peripherals connected to this driver
	    set periphs [xget_sw_iplist_for_driver $drv_handle]
	
	    # Handle NUM_INSTANCES
	    set periph_ninstances 0
	    puts $file_handle "/* Definitions for driver [string toupper [get_property NAME $drv_handle]] */"
	    foreach periph $periphs {
	    	init_periph_config_struct_srio $periph_ninstances
	    	incr periph_ninstances 1
	    }
	    puts $file_handle "\#define [xget_dname $drv_string NUM_INSTANCES] $periph_ninstances"
	
	
	    # Now print all useful parameters for all peripherals
	    set device_id 0
	    foreach periph $periphs {
        	puts $file_handle ""
        	
        	xdefine_srio_params_instance $file_handle $periph $device_id
        	
        	xdefine_srio_params_canonical $file_handle $periph $device_id
        	incr device_id
            	puts $file_handle "\n"
           }
           puts $file_handle "\n/******************************************************************/\n"
    	   close $file_handle
}

proc xdefine_srio_params_instance {file_handle periph device_id} {
    set sriois_memory [get_property CONFIG.C_PE_MEMORY $periph]
    if {$sriois_memory == 0} {
             set sriois_memory 0
    } 
    
    set sriois_processor [get_property CONFIG.C_PE_PROC $periph]
    if {$sriois_processor == 0} {
             set sriois_processor 0
    } else {
	     set sriois_processor 2
    }
    
    set sriois_bridge [get_property CONFIG.C_PE_BRIDGE $periph]
    if {$sriois_bridge == 0} {
             set sriois_bridge 0
    } else {
	     set sriois_bridge 3
    }
    
    puts $file_handle "/* Definitions for peripheral [string toupper [get_property NAME $periph]] */"
    
    puts $file_handle "\#define [xget_dname $periph "DEVICE_ID"] $device_id"
    set value [get_property CONFIG.C_BASEADDR $periph]
    if {[llength $value] == 0} {
            set value 0
    }
    puts $file_handle "\#define [xget_dname $periph "C_BASEADDR"] $value"
    
    set value [get_property CONFIG.C_HIGHADDR $periph]
    if {[llength $value] == 0} {
            set value 0
    }
    puts $file_handle "\#define [xget_dname $periph "C_HIGHADDR"] $value"
    
    set value [get_property CONFIG.C_DEVICEID_WIDTH $periph]
    if {[llength $value] == 0} {
            set value 0
    }
    puts $file_handle "\#define [xget_dname $periph "C_DEVICEID_WIDTH"] $value"
    
    set value [get_property CONFIG.C_IS_HOST $periph]
    if {[llength $value] == 0} {
            set value 0
    }
    puts $file_handle "\#define [xget_dname $periph "C_IS_HOST"] $value"
    
    set value [get_property CONFIG.C_TX_DEPTH $periph]
    if {[llength $value] == 0} {
            set value 0
    }
    puts $file_handle "\#define [xget_dname $periph "C_TX_DEPTH"] $value"
    
    set value [get_property CONFIG.C_RX_DEPTH $periph]
    if {[llength $value] == 0} {
            set value 0
    }
    puts $file_handle "\#define [xget_dname $periph "C_RX_DEPTH"] $value"
    
    set value [get_property CONFIG.C_DISCOVERED $periph]
    if {[llength $value] == 0} {
            set value 0
    }
    puts $file_handle "\#define [xget_dname $periph "C_DISCOVERED"] $value"
    
    puts $file_handle "\#define [xget_dname $periph "PE_MEMORY"] $sriois_memory"
    puts $file_handle "\#define [xget_dname $periph "PE_PROC"] $sriois_processor"
    puts $file_handle "\#define [xget_dname $periph "PE_BRIDGE"] $sriois_bridge"
    
}

proc xdefine_srio_params_canonical {file_handle periph device_id} {

    set sriois_memory [get_property CONFIG.C_PE_MEMORY $periph]
    if {$sriois_memory == 0} {
             set sriois_memory 0
    } 
    
    set sriois_processor [get_property CONFIG.C_PE_PROC $periph]
    if {$sriois_processor == 0} {
             set sriois_processor 0
    } else {
	     set sriois_processor 2
    }
    
    set sriois_bridge [get_property CONFIG.C_PE_BRIDGE $periph]
    if {$sriois_bridge == 0} {
             set sriois_bridge 0
    } else {
	     set sriois_bridge 3
    }
    
    puts $file_handle "\n/* Canonical definitions for peripheral [string toupper [get_property NAME $periph]] */"

    set canonical_tag [string toupper [format "XPAR_SRIO_%d" $device_id]]

    # Handle device ID
    set canonical_name  [format "%s_DEVICE_ID" $canonical_tag]
    puts $file_handle "\#define $canonical_name $device_id"
    add_field_to_periph_config_struct_srio $device_id $canonical_name

    set canonical_name  [format "%s_BASEADDR" $canonical_tag]
    set value [get_property CONFIG.C_BASEADDR $periph]
    if {[llength $value] == 0} {
       	set value 0
    }
    puts $file_handle "\#define $canonical_name $value"
    add_field_to_periph_config_struct_srio $device_id $canonical_name
    
    set canonical_name  [format "%s_PE_MEMORY" $canonical_tag]
    puts $file_handle "\#define $canonical_name $sriois_memory"
    add_field_to_periph_config_struct_srio $device_id $canonical_name
    
    set canonical_name  [format "%s_PE_PROC" $canonical_tag]
    puts $file_handle "\#define $canonical_name $sriois_processor"
    add_field_to_periph_config_struct_srio $device_id $canonical_name
    
     set canonical_name  [format "%s_PE_BRIDGE" $canonical_tag]
     puts $file_handle "\#define $canonical_name $sriois_bridge"
     add_field_to_periph_config_struct_srio $device_id $canonical_name
  
}

proc xdefine_srio_config_file {file_name drv_string} {

    global periph_ninstances

    set filename [file join "src" $file_name]
    file delete $filename
    set config_file [open $filename w]
    xprint_generated_header $config_file "Driver configuration"
    puts $config_file "\#include \"xparameters.h\""
    puts $config_file "\#include \"[string tolower $drv_string].h\""
    puts $config_file "\n/*"
    puts $config_file "* The configuration table for devices"
    puts $config_file "*/\n"
    puts $config_file [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
    puts $config_file "\{"

    set start_comma ""
    for {set i 0} {$i < $periph_ninstances} {incr i} {

        puts $config_file [format "%s\t\{" $start_comma]
        set comma ""
        foreach field [get_periph_config_struct_fields_srio $i] {
            puts -nonewline $config_file [format "%s\t\t%s" $comma $field]
            set comma ",\n"
        }

        puts -nonewline $config_file "\n\t\}"
        set start_comma ",\n"
    }
    puts $config_file "\n\};\n"
    close $config_file
}

proc generate {drv_handle} {
    xdefine_srio_include_file $drv_handle "xparameters.h" "XSrio"
    xdefine_srio_config_file  "xsrio_g.c" "XSrio"
}




