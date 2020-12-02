###############################################################################
# Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 3.03a sdm  08/11/10 Added C_SPI_MODE parameter to config structure
# 3.04a bss  03/21/12 Added C_TYPE_OF_AXI4_INTERFACE, C_AXI4_BASEADDR and
#	              C_XIP_MODE to config structure.
# 		      Modified such that based on C_XIP_MODE and 
#		      C_TYPE_OF_AXI4_INTERFACE parameters C_BASEADDR will 
#		      be updated with the value of C_AXI4_BASEADDR.
#		      Modified such that C_FIFO_EXIST will be updated based
#		      on C_FIFO_DEPTH for compatibility of driver
# 3.06a adk 07/08/13 Added C_USE_STARTUP parameter to the config structure
# 4.0   adk 12/10/13 Updated as per the New Tcl API's
# 4.2   sk  11/09/15 Removed delete filename statement CR# 784758.
# 4.3   ms  04/18/17 Modified tcl file to add suffix U for all macros
#                    definitions of spi in xparameters.h
# 4.7	akm 10/22/20 Modified tcl file to add XPAR_SPI_0_FIFO_DEPTH to the
#		     config structure.
##############################################################################

#uses "xillib.tcl"

set periph_config_params_spi 0
set periph_ninstances_spi    0

proc init_periph_config_struct_spi { deviceid } {
    global periph_config_params_spi
    set periph_config_params_spi($deviceid) [list]
}

proc add_field_to_periph_config_struct_spi { deviceid fieldval } {
    global periph_config_params_spi
    lappend periph_config_params_spi($deviceid) $fieldval
}

proc get_periph_config_struct_fields_spi { deviceid } {
    global periph_config_params_spi
    return $periph_config_params_spi($deviceid)
}

proc xdefine_axispi_include_file {drv_handle file_name drv_string} {
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
	    	init_periph_config_struct_spi $periph_ninstances
	    	incr periph_ninstances 1
	    }
	    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $drv_string NUM_INSTANCES] $periph_ninstances$uSuffix"
	
	
	    # Now print all useful parameters for all peripherals
	    set device_id 0
	    foreach periph $periphs {
        	puts $file_handle ""
        	
        	xdefine_axispi_params_instance $file_handle $periph $device_id
        	
        	xdefine_axispi_params_canonical $file_handle $periph $device_id
        	incr device_id
            	puts $file_handle "\n"
           }
           puts $file_handle "\n/******************************************************************/\n"
    	   close $file_handle
}

proc xdefine_axispi_params_instance {file_handle periph device_id} {
	set uSuffix "U"
	set ip [hsi::get_cells -hier $periph]
    set xip_mode_value [common::get_property  CONFIG.C_XIP_MODE $ip]
    if {[llength $xip_mode_value] == 0} {
         set xip_mode_value 0
    }
    set axi_type_value [common::get_property CONFIG.C_TYPE_OF_AXI4_INTERFACE $periph]
    if {[llength $axi_type_value] == 0} {
         set axi_type_value 0
    }
    set axi4_baseaddr_value [common::get_property CONFIG.C_S_AXI4_BASEADDR $periph]
    if {[llength $axi4_baseaddr_value] == 0} {
             set axi4_baseaddr_value 0
    }
    set axi4_highaddr_value [common::get_property CONFIG.C_S_AXI4_HIGHADDR $periph]
    if {[llength $axi4_highaddr_value] == 0} {
             set axi4_highaddr_value 0
    }
    puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
    
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DEVICE_ID"] $device_id$uSuffix"
    if {$xip_mode_value == 0} {
    	if {$axi_type_value == 0} { 
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] [common::get_property CONFIG.C_BASEADDR $periph]$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph HIGHADDR] [common::get_property CONFIG.C_HIGHADDR $periph]$uSuffix"
    	} else {
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] $axi4_baseaddr_value$uSuffix"
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph HIGHADDR] $axi4_highaddr_value$uSuffix"
    	}
    } else {
	puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] [common::get_property CONFIG.C_BASEADDR $periph]$uSuffix"
	puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph HIGHADDR] [common::get_property CONFIG.C_HIGHADDR $periph]$uSuffix"
    }
    
    set value [common::get_property CONFIG.C_FIFO_EXIST $periph]
    set fifo_depth 0
    if {[llength $value] == 0} {
         set value1 [common::get_property CONFIG.C_FIFO_DEPTH $periph]
         if {[llength $value1] == 0} {
    	    set value1 0
         } else {
           set fifo_depth $value1
           if {$value1 == 0} {
              set value1 0
           } else {
              set value1 1
           }
        }
    } else {
        set value1 $value
    }
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "FIFO_EXIST"] $value1$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "FIFO_DEPTH"] $fifo_depth$uSuffix"
    
    set value [common::get_property CONFIG.C_SPI_SLAVE_ONLY $periph]
    if {[llength $value] == 0} {
	set value 0
    }
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "SPI_SLAVE_ONLY"] $value$uSuffix"
    set value [common::get_property CONFIG.C_NUM_SS_BITS $periph]
    if {[llength $value] == 0} {
    	set value 0
    }
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "NUM_SS_BITS"] $value$uSuffix"
    set value [common::get_property CONFIG.C_NUM_TRANSFER_BITS $periph]
    if {[llength $value] == 0} {
        set value 0
    }
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "NUM_TRANSFER_BITS"] $value$uSuffix"
    set value [common::get_property CONFIG.C_SPI_MODE $periph]
    if {[llength $value] == 0} {
            set value 0
    }
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "SPI_MODE"] $value$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "TYPE_OF_AXI4_INTERFACE"] $axi_type_value$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "AXI4_BASEADDR"] $axi4_baseaddr_value$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "AXI4_HIGHADDR"] $axi4_highaddr_value$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "XIP_MODE"] $xip_mode_value$uSuffix"
}

proc xdefine_axispi_params_canonical {file_handle periph device_id} {
    set uSuffix "U"
    set xip_mode_value [common::get_property CONFIG.C_XIP_MODE $periph]
    if {[llength $xip_mode_value] == 0} {
      set xip_mode_value 0
    }
    set axi_type_value [common::get_property CONFIG.C_TYPE_OF_AXI4_INTERFACE $periph]
    if {[llength $axi_type_value] == 0} {
       set axi_type_value 0
    }
    set axi4_baseaddr_value [common::get_property CONFIG.C_S_AXI4_BASEADDR $periph]
    if {[llength $axi4_baseaddr_value] == 0} {
	 set axi4_baseaddr_value 0
    }
    set axi4_highaddr_value [common::get_property CONFIG.C_S_AXI4_HIGHADDR $periph]
    if {[llength $axi4_highaddr_value] == 0} {
	 set axi4_highaddr_value 0
    }
    
    set use_startup_value [common::get_property CONFIG.C_USE_STARTUP $periph]
    if {[llength $use_startup_value] == 0} {
	set use_startup_value 0
    }
    
    puts $file_handle "\n/* Canonical definitions for peripheral [string toupper [common::get_property NAME $periph]] */"

    set canonical_tag [string toupper [format "XPAR_SPI_%d" $device_id]]

    # Handle device ID
    set canonical_name  [format "%s_DEVICE_ID" $canonical_tag]
    puts $file_handle "\#define $canonical_name $device_id$uSuffix"
    add_field_to_periph_config_struct_spi $device_id $canonical_name

    if {$xip_mode_value == 0} {
	if {$axi_type_value == 0} { 
		set canonical_name  [format "%s_BASEADDR" $canonical_tag]
		puts $file_handle "\#define $canonical_name [common::get_property CONFIG.C_BASEADDR $periph]$uSuffix"
                add_field_to_periph_config_struct_spi $device_id $canonical_name
                set canonical_name  [format "%s_HIGHADDR" $canonical_tag]
                puts $file_handle "\#define $canonical_name [common::get_property CONFIG.C_HIGHADDR $periph]$uSuffix"
	} else {
		set canonical_name  [format "%s_BASEADDR" $canonical_tag]
		puts $file_handle "\#define $canonical_name $axi4_baseaddr_value$uSuffix"
                add_field_to_periph_config_struct_spi $device_id $canonical_name
		set canonical_name  [format "%s_HIGHADDR" $canonical_tag]
                puts $file_handle "\#define $canonical_name $axi4_highaddr_value$uSuffix"
	}
    } else {
        set canonical_name  [format "%s_BASEADDR" $canonical_tag]
	puts $file_handle "\#define $canonical_name [common::get_property CONFIG.C_BASEADDR $periph]$uSuffix"
	add_field_to_periph_config_struct_spi $device_id $canonical_name
	set canonical_name  [format "%s_HIGHADDR" $canonical_tag]
        puts $file_handle "\#define $canonical_name [common::get_property CONFIG.C_HIGHADDR $periph]$uSuffix"
    }
    set canonical_name  [format "%s_FIFO_EXIST" $canonical_tag]
    set canonical_name1  [format "%s_FIFO_DEPTH" $canonical_tag]
    set value [common::get_property CONFIG.C_FIFO_EXIST $periph]
    set fifo_depth 0
    if {[llength $value] == 0} {
        set value1 [common::get_property CONFIG.C_FIFO_DEPTH $periph]
        if {[llength $value1] == 0} {
	    set value1 0
    	} else {
           set fifo_depth $value1
    	   if {$value1 == 0} {
    	   	set value1 0
    	   } else {
    	   	set value1 1
    	   }
    	}
    } else {
    	set value1 $value
    }
    puts $file_handle "\#define $canonical_name $value1$uSuffix"
    add_field_to_periph_config_struct_spi $device_id $canonical_name
    
    puts $file_handle "\#define $canonical_name1 $fifo_depth$uSuffix"

    set canonical_name  [format "%s_SPI_SLAVE_ONLY" $canonical_tag]
    set value [common::get_property CONFIG.C_SPI_SLAVE_ONLY $periph]
    if {[llength $value] == 0} {
    	set value 0
    }
    puts $file_handle "\#define $canonical_name $value$uSuffix"
    add_field_to_periph_config_struct_spi $device_id $canonical_name

    set canonical_name  [format "%s_NUM_SS_BITS" $canonical_tag]
    set value [common::get_property CONFIG.C_NUM_SS_BITS $periph]
    if {[llength $value] == 0} {
        set value 0
    }
    puts $file_handle "\#define $canonical_name $value$uSuffix"
    add_field_to_periph_config_struct_spi $device_id $canonical_name

    set canonical_name  [format "%s_NUM_TRANSFER_BITS" $canonical_tag]
    set value [common::get_property CONFIG.C_NUM_TRANSFER_BITS $periph]
    if {[llength $value] == 0} {
    	set value 0
    }
    puts $file_handle "\#define $canonical_name $value$uSuffix"
    add_field_to_periph_config_struct_spi $device_id $canonical_name

    set canonical_name  [format "%s_SPI_MODE" $canonical_tag]
    set value [common::get_property CONFIG.C_SPI_MODE $periph]
    if {[llength $value] == 0} {
       	set value 0
    }
    puts $file_handle "\#define $canonical_name $value$uSuffix"
    add_field_to_periph_config_struct_spi $device_id $canonical_name

    set canonical_name  [format "%s_TYPE_OF_AXI4_INTERFACE" $canonical_tag]
    puts $file_handle "\#define $canonical_name $axi_type_value$uSuffix"
    add_field_to_periph_config_struct_spi $device_id $canonical_name

    set canonical_name  [format "%s_AXI4_BASEADDR" $canonical_tag]
    puts $file_handle "\#define $canonical_name $axi4_baseaddr_value$uSuffix"
    add_field_to_periph_config_struct_spi $device_id $canonical_name

    set canonical_name  [format "%s_AXI4_HIGHADDR" $canonical_tag]
    puts $file_handle "\#define $canonical_name $axi4_highaddr_value$uSuffix"
    
    set canonical_name  [format "%s_XIP_MODE" $canonical_tag]
    puts $file_handle "\#define $canonical_name $xip_mode_value$uSuffix"
    add_field_to_periph_config_struct_spi $device_id $canonical_name
    
    set canonical_name  [format "%s_USE_STARTUP" $canonical_tag]
    puts $file_handle "\#define $canonical_name $use_startup_value$uSuffix"
    add_field_to_periph_config_struct_spi $device_id $canonical_name

    add_field_to_periph_config_struct_spi $device_id $canonical_name1
}

proc xdefine_axispi_config_file {file_name drv_string} {

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
        foreach field [get_periph_config_struct_fields_spi $i] {
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
    xdefine_axispi_include_file $drv_handle "xparameters.h" "XSpi"
    xdefine_axispi_config_file  "xspi_g.c" "XSpi"
}

