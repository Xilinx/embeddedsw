###############################################################################
#
# Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
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
#  @file xtrafgen_v2_1_0.tcl
#
#
# <pre>
# MODIFICATION HISTORY:
#
# Ver   Who  Date     Changes
# ----- ---- -------- -------------------------------------------------------
# 1.00a srt  01/20/13 First release 
# 1.01a adk  03/09/13 Updated tcl to differentiate Different Modes in AXI 
#		      Traffic Genrator.
# 3.0   adk  12/10/13 Updated as per the New Tcl API's
# 4.1   sk   11/09/15 Removed delete filename statement CR# 784758.
# 4.2   ms   04/18/17 Modified tcl file to add suffix U for all macros
#                     definitions of trafgen in xparameters.h
# </pre>
#
##############################################################################


#uses "xillib.tcl"

set periph_config_params_atg 0
set periph_ninstances_atg    0
set atg_mode_value	     0
set atg_mode_value_l2        0
set axi_mode_value           0
set address_width_value      0
set baseaddr_value           0
set highaddr_value           0

proc init_periph_config_struct_atg { deviceid } {
    global periph_config_params_atg
    set periph_config_params_atg($deviceid) [list]
}

proc add_field_to_periph_config_struct_atg { deviceid fieldval } {
    global periph_config_params_atg
    lappend periph_config_params_atg($deviceid) $fieldval
}

proc get_periph_config_struct_fields_atg { deviceid } {
    global periph_config_params_atg
    return $periph_config_params_atg($deviceid)
}

proc xdefine_trafgen_include_file {drv_handle file_name drv_string} {
	global periph_ninstances
	
	    # Open include file
	    set file_handle [::hsi::utils::open_include_file $file_name]
	
	    # Get all peripherals connected to this driver
	    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
	
	    # Handle NUM_INSTANCES
	    set periph_ninstances 0
	    set uSuffix "U"
	    puts $file_handle "/* Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
	    foreach periph $periphs {
	    	init_periph_config_struct_atg $periph_ninstances
	    	incr periph_ninstances 1
	    }
	    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $drv_string NUM_INSTANCES] $periph_ninstances$uSuffix"
	
	
	    # Now print all useful parameters for all peripherals
	    set device_id 0
	    foreach periph $periphs {
        	puts $file_handle ""
        	
        	xdefine_trafgen_params_instance $file_handle $periph $device_id
        	
        	xdefine_trafgen_params_canonical $file_handle $periph $device_id
        	incr device_id
            	puts $file_handle "\n"
           }
           puts $file_handle "\n/******************************************************************/\n"
    	   close $file_handle
}

proc xdfeine_trafgen_params_constants { periph } {
    global atg_mode_value	     
    global atg_mode_value_l2        
    global axi_mode_value           
    global baseaddr_value           
    global address_width_value
    global highaddr_value       
 
    set atg_mode_name [::hsi::utils::get_param_value $periph C_ATG_MODE]
    set axi4_name [string match -nocase $atg_mode_name "AXI4"]
    set axi4_lite_name [string match -nocase $atg_mode_name "AXI4-Lite"]
    set axi4_Stream_name [string match -nocase $atg_mode_name "AXI4-Stream"]
     if {$axi4_name == 1} {
	set atg_mode_value 1
    }
    if {$axi4_lite_name == 1} {
	set atg_mode_value 2
    }
    if {$axi4_Stream_name == 1} {
	set atg_mode_value 3
    }
    
    if {[llength $atg_mode_name] == 0} {
         set atg_mode_value 0
    }
    
    set atg_mode_l2_name [::hsi::utils::get_param_value $periph C_ATG_MODE_L2]
    set adv_mode_name [string match -nocase $atg_mode_l2_name "Advanced"]
    set basic_mode_name [string match -nocase $atg_mode_l2_name "Basic"]
    set static_mode_name [string match -nocase $atg_mode_l2_name "Static"]
  if {$adv_mode_name == 1} {
	set atg_mode_value_l2 1
    }
    if {$basic_mode_name == 1} {
	set atg_mode_value_l2 2
    }
    if {$static_mode_name == 1} {
	set atg_mode_value_l2 3
    }
    if {[llength $atg_mode_l2_name] == 0} {
         set atg_mode_value_l2 0
    }
    set axi_mode_name [::hsi::utils::get_param_value $periph C_AXIS_MODE]
    set master_name [string match -nocase $axi_mode_name "Master Only"]
    set slave_name [string match -nocase $axi_mode_name "Slave Only"]
    set master_loop_name [string match -nocase $axi_mode_name "Master Loop back"]
    set slave_loop_name [string match -nocase $axi_mode_name "Slave Loop back"]
   if {$master_name == 1} {
	set axi_mode_value 1
    }
    if {$slave_name == 1} {
	set axi_mode_value 2
    }
    if {$master_loop_name == 1} {
	set axi_mode_value 3
    }
    if {$slave_loop_name == 1} {
	set axi_mode_value 4
    }
    if {[llength $axi_mode_name] == 0} {
         set axi_mode_value 0
    }

    set baseaddr_value [::hsi::utils::get_param_value $periph C_BASEADDR]
    if {[llength $baseaddr_value] == 0} {
             set baseaddr_value 0
    }
    set address_width_value [::hsi::utils::get_param_value $periph C_EXTENDED_ADDRESS_WIDTH]
    if {[llength $address_width_value] == 0} {
             set address_width_value 0
    }
    set highaddr_value [::hsi::utils::get_param_value $periph C_HIGHADDR]
    if {[llength $highaddr_value] == 0} {
             set highaddr_value 0
    }
}

proc xdefine_trafgen_params_instance {file_handle periph device_id} { 
    set uSuffix "U"
    xdfeine_trafgen_params_constants   $periph
    global atg_mode_value
    global atg_mode_value_l2
    global axi_mode_value
    puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
    
    puts $file_handle "\#define [::hsi::utils::get_ip_param_name $periph "DEVICE_ID"] $device_id$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_ip_param_name $periph "BASEADDR"] [::hsi::utils::get_param_value $periph C_BASEADDR]$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_ip_param_name $periph "HIGHADDR"] [::hsi::utils::get_param_value $periph C_HIGHADDR]$uSuffix"
    
    puts $file_handle "\#define [::hsi::utils::get_ip_param_name $periph "C_ATG_MODE"] $atg_mode_value$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_ip_param_name $periph "C_ATG_MODE_L2"] $atg_mode_value_l2$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_ip_param_name $periph "C_AXIS_MODE"] $axi_mode_value$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_ip_param_name $periph "C_EXTENDED_ADDRESS_WIDTH"] [::hsi::utils::get_param_value $periph C_EXTENDED_ADDRESS_WIDTH]$uSuffix"
}

proc xdefine_trafgen_params_canonical {file_handle periph device_id} {
    set uSuffix "U"
    xdfeine_trafgen_params_constants  $periph 
    global atg_mode_value	     
    global atg_mode_value_l2        
    global axi_mode_value           
    global address_width_value
    global baseaddr_value           
    global highaddr_value
    puts $file_handle "\n/* Canonical definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
    
    set canonical_tag [string toupper [format "XPAR_XTRAFGEN_%d" $device_id]]
    
     # Handle device ID
    set canonical_name  [format "%s_DEVICE_ID" $canonical_tag]
    puts $file_handle "\#define $canonical_name $device_id$uSuffix"
    add_field_to_periph_config_struct_atg $device_id $canonical_name
    
    set canonical_name  [format "%s_BASEADDR" $canonical_tag]
    puts $file_handle "\#define $canonical_name $baseaddr_value$uSuffix"
    add_field_to_periph_config_struct_atg $device_id $canonical_name
     
    set canonical_name  [format "%s_HIGHADDR" $canonical_tag]
    puts $file_handle "\#define $canonical_name $highaddr_value$uSuffix"
    
    set canonical_name  [format "%s_ATG_MODE" $canonical_tag]
    puts $file_handle "\#define $canonical_name $atg_mode_value$uSuffix"
    add_field_to_periph_config_struct_atg $device_id $canonical_name
    
    set canonical_name  [format "%s_ATG_MODE_L2" $canonical_tag]
    puts $file_handle "\#define $canonical_name $atg_mode_value_l2$uSuffix"
    add_field_to_periph_config_struct_atg $device_id $canonical_name
    
    set canonical_name  [format "%s_AXIS_MODE" $canonical_tag]
    puts $file_handle "\#define $canonical_name $axi_mode_value$uSuffix"
    add_field_to_periph_config_struct_atg $device_id $canonical_name

    set canonical_name  [format "%s_EXTENDED_ADDRESS_WIDTH" $canonical_tag]
    puts $file_handle "\#define $canonical_name $address_width_value$uSuffix"
    add_field_to_periph_config_struct_atg $device_id $canonical_name
}

proc xdefine_trafgen_config_file {file_name drv_string} {

    global periph_ninstances

    set filename [file join "src" $file_name]
    set config_file [open $filename w]
    ::hsi::utils::write_c_header $config_file "Driver configuration"
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
        foreach field [get_periph_config_struct_fields_atg $i] {
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
    xdefine_trafgen_include_file $drv_handle "xparameters.h" "XTrafGen"
    xdefine_trafgen_config_file  "xtrafgen_g.c" "XTrafGen"
}
