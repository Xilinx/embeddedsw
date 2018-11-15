###############################################################################
#
# Copyright (C) 2005 - 2018 Xilinx, Inc.  All rights reserved.
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
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 2.00a sdm  07/26/10 Updated to use the string "Axi_Fifo" in canonical
#                     definitions for AxiFifo
# 3.00a adk  08/10/13 Added parameters C_AXI4_BASEADDR and C_AXI4_HIGHADDR
#		      and C_DATA_INTERFACE_TYPE inorder to support AXI4
#		      Datainterface.
# 4.0      adk    12/10/13 Updated as per the New Tcl API's
# 5.1  adk   01/02/15 CR#885653 Fix Incorrect AXI4 Base address being
#		      Exported to the xparameters.h file.
# 5.2   ms   04/18/17 Modified tcl file to add suffix U for all macros
#                     definitions of llfifo in xparameters.h
##############################################################################

set periph_config_params_fifo 0
set periph_ninstances_fifo    0

proc init_periph_config_struct_fifo { deviceid } {
    global periph_config_params_fifo
    set periph_config_params_fifo($deviceid) [list]
}

proc add_field_to_periph_config_struct_fifo { deviceid fieldval } {
    global periph_config_params_fifo
    lappend periph_config_params_fifo($deviceid) $fieldval
}

proc get_periph_config_struct_fields_fifo { deviceid } {
    global periph_config_params_fifo
    return $periph_config_params_fifo($deviceid)
}

proc xdefine_axififo_include_file {drv_handle file_name drv_string} {
	global periph_ninstances

	    # Open include file
	    set file_handle [::hsi::utils::open_include_file $file_name]

	    # Get all peripherals connected to this driver
	    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

	    set uSuffix "U"
	    # Handle NUM_INSTANCES
	    set periph_ninstances 0
	    puts $file_handle "/* Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
	    foreach periph $periphs {
		init_periph_config_struct_fifo $periph_ninstances
		incr periph_ninstances 1
	    }
	    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $drv_string NUM_INSTANCES] $periph_ninstances$uSuffix"


	    # Now print all useful parameters for all peripherals
	    set device_id 0
	    foreach periph $periphs {
		puts $file_handle ""

		xdefine_axififo_params_instance $file_handle $periph $device_id

		xdefine_axififo_params_canonical $file_handle $periph $device_id
		incr device_id
		puts $file_handle "\n"
           }
           puts $file_handle "\n/******************************************************************/\n"
	   close $file_handle
}

proc xdefine_axififo_params_instance {file_handle periph device_id} {
    set uSuffix "U"
    set ip [hsi::get_cells -hier $periph]

    set axi_baseaddr  [common::get_property CONFIG.C_BASEADDR $periph]
    set axi_highaddr  [common::get_property CONFIG.C_HIGHADDR $periph]
    set axi4_baseaddr [common::get_property CONFIG.C_AXI4_BASEADDR $periph]
    set axi4_highaddr [common::get_property CONFIG.C_AXI4_HIGHADDR $periph]
    set datainterface [common::get_property CONFIG.C_DATA_INTERFACE_TYPE $periph]
    if { $datainterface == 0} {
	set axi4_baseaddr 0
	set axi4_highaddr 0
    }

    puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DEVICE_ID"] $device_id$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] $axi_baseaddr$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "HIGHADDR"] $axi_highaddr$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "AXI4_BASEADDR"] $axi4_baseaddr$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "AXI4_HIGHADDR"] $axi4_highaddr$uSuffix"
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DATA_INTERFACE_TYPE"] $datainterface$uSuffix"
}

proc xdefine_axififo_params_canonical {file_handle periph device_id} {
    set uSuffix "U"
    set axi4_baseaddr [common::get_property CONFIG.C_AXI4_BASEADDR $periph]
    set axi4_highaddr [common::get_property CONFIG.C_AXI4_HIGHADDR $periph]
    set datainterface [common::get_property CONFIG.C_DATA_INTERFACE_TYPE $periph]
    if { $datainterface == 0} {
	set axi4_baseaddr 0
	set axi4_highaddr 0
    }
    puts $file_handle "\n/* Canonical definitions for peripheral [string toupper [common::get_property NAME $periph]] */"

    set canonical_tag [string toupper [format "XPAR_Axi_Fifo_%d" $device_id]]

    # Handle device ID
    set canonical_name  [format "%s_DEVICE_ID" $canonical_tag]
    puts $file_handle "\#define $canonical_name $device_id$uSuffix"
    add_field_to_periph_config_struct_fifo $device_id $canonical_name

    set canonical_name  [format "%s_BASEADDR" $canonical_tag]
    set value [common::get_property CONFIG.C_BASEADDR $periph]
    puts $file_handle "\#define $canonical_name $value$uSuffix"
    add_field_to_periph_config_struct_fifo $device_id $canonical_name

    set canonical_name  [format "%s_HIGHADDR" $canonical_tag]
    set value [common::get_property CONFIG.C_HIGHADDR $periph]
    puts $file_handle "\#define $canonical_name $value$uSuffix"

    set canonical_name  [format "%s_AXI4_BASEADDR" $canonical_tag]
    puts $file_handle "\#define $canonical_name $axi4_baseaddr$uSuffix"
    add_field_to_periph_config_struct_fifo $device_id $canonical_name

    set canonical_name  [format "%s_AXI4_HIGHADDR" $canonical_tag]
    puts $file_handle "\#define $canonical_name $axi4_highaddr$uSuffix"

    set canonical_name  [format "%s_DATA_INTERFACE_TYPE" $canonical_tag]
    puts $file_handle "\#define $canonical_name $datainterface$uSuffix"
    add_field_to_periph_config_struct_fifo $device_id $canonical_name
}

proc xdefine_axififo_config_file {file_name drv_string} {

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
        foreach field [get_periph_config_struct_fields_fifo $i] {
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
    xdefine_axififo_include_file $drv_handle "xparameters.h" "XLlFifo"
    xdefine_axififo_config_file  "xllfifo_g.c" "XLlFifo"
}
