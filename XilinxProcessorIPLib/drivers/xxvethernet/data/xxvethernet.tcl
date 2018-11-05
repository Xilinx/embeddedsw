###############################################################################
#
# Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
# MODIFICATION HISTORY:
# 06/16/17 hk First Release
#
###############################################################################
#uses "xillib.tcl"

set periph_config_params 	0
set periph_ninstances    	0

proc init_periph_config_struct { deviceid } {
    global periph_config_params
    set periph_config_params($deviceid) [list]
}

proc get_periph_config_struct_fields { deviceid } {
    global periph_config_params
    return $periph_config_params($deviceid)
}
proc add_field_to_periph_config_struct { deviceid fieldval } {
    global periph_config_params
    lappend periph_config_params($deviceid) $fieldval
}

# ------------------------------------------------------------------
# Given an Xxv Ethernet peripheral, generate all the stuff required in
# the system include file.
#
#    Given an Xxv Ethernet which is an initiator on a
#    Axi4 Stream interface, traverse the Axi4 Stream to the target side,
#    figure out the peripheral type that is connected and
#    put in appropriate defines. The peripheral on the other side
#    can be AXI DMA or  AXI Streaming FIFO.
#
# ------------------------------------------------------------------
proc xdefine_xxvethernet_include_file {drv_handle file_name drv_string} {
    global periph_ninstances

    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # ----------------------------------------------
    # PART 1 - XXV Ethernet related parameters
    # ----------------------------------------------

    # Handle NUM_INSTANCES
    set periph_ninstances 0
    puts $file_handle "/* Definitions for driver [string toupper [get_property NAME $drv_handle]] */"
    foreach periph $periphs {
	init_periph_config_struct $periph_ninstances
	incr periph_ninstances 1
    }
    puts $file_handle "\#define [::hsi::utils::get_driver_param_name $drv_string NUM_INSTANCES] $periph_ninstances"

    close $file_handle
    # Now print all useful parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
	set file_handle [::hsi::utils::open_include_file $file_name]

	xdefine_temac_params_include_file $file_handle $periph $device_id

	# Create canonical definitions
	xdefine_temac_params_canonical $file_handle $periph $device_id
	# Interrupt ID (canonical)
	xdefine_temac_interrupt $file_handle $periph $device_id

	incr device_id
	puts $file_handle "\n"
	close $file_handle
   }

     # -------------------------------------------------------
    # PART 2 -- MCDMA Connection related parameters
    # -------------------------------------------------------
     set file_handle [::hsi::utils::open_include_file $file_name]
    xdefine_axi_target_params $periphs $file_handle

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}
# ------------------------------------------------------------------
# Main generate function - called by the tools
# ------------------------------------------------------------------
proc generate {drv_handle} {
    xdefine_xxvethernet_include_file $drv_handle "xparameters.h" "XXxvEthernet"
    xdefine_xxvethernet_config_file  "xxxvethernet_g.c" "XXxvEthernet"
}

# ---------------------------------------------------------------------------
# Given each AXI4 Stream peripheral which is an initiator on a AXI4
# Stream interface, traverse to the target side, figure out the peripheral
# type that is connected and put in appropriate defines.
# The peripheral on the other side can be AXI DMA or  AXi Streaming FIFO.
#
# NOTE: This procedure assumes that each AXI4 Stream on each peripheral
#       corresponds to a unique device id in the system and populates
#       the global device config params structure accordingly.
# ---------------------------------------------------------------------------
proc xdefine_axi_target_params {periphs file_handle} {

    global periph_ninstances

     #
    # First dump some enumerations on AXI_TYPE
    #
    puts $file_handle "/* XxvEthernet TYPE Enumerations */"
    puts $file_handle "#define XPAR_MCDMA     1"
    puts $file_handle ""

    set device_id 0
    set validentry 0

    # Get unique list of p2p peripherals
    foreach periph $periphs {
        set p2p_periphs [list]
        set periph_name [string toupper [get_property NAME $periph]]
	# Get all point2point buses for periph
	set p2p_busifs_i [get_intf_pins -of_objects $periph -filter "TYPE==INITIATOR"]

        puts $file_handle ""
        puts $file_handle "/* Canonical Axi parameters for $periph_name */"

        # Add p2p periphs
        foreach p2p_busif $p2p_busifs_i {

            set busif_name [string toupper [get_property NAME  $p2p_busif]]
            set conn_busif_handle [::hsi::utils::get_connected_intf $periph $busif_name]
	    if { [string compare -nocase $conn_busif_handle ""] == 0} {
                continue
            } else {
		# if there is a single match, we know if it is FIFO or DMA
		# no need for further iterations
		set conn_busif_name [get_property NAME  $conn_busif_handle]
		set target_periph [get_cells -of_objects $conn_busif_handle]
		set target_periph_type [get_property IP_NAME $target_periph]
                if { [string compare -nocase $target_periph_type "tri_mode_ethernet_mac"] == 0 } {
			continue
		}
		set tartget_per_name [get_property NAME $target_periph]
		set target_periph_name [string toupper [get_property NAME $target_periph]]
		set canonical_tag [string toupper [format "AXIETHERNET_%d" $device_id ]]
		set validentry 1
		break
            }
      }

	if {$validentry == 1} {
		if {$target_periph_type == "axi_mcdma"} {
		    set mcdma_baseaddr [get_property  CONFIG.C_BASEADDR $target_periph]
                    # Handle base address and connection type
                    set canonical_name [format "XPAR_%s_CONNECTED_TYPE" $canonical_tag]
                    puts $file_handle "#define $canonical_name XPAR_MCDMA"
                    add_field_to_periph_config_struct $device_id $canonical_name
                    set canonical_name [format "XPAR_%s_CONNECTED_BASEADDR" $canonical_tag]
                    puts $file_handle [format "#define $canonical_name %s" $mcdma_baseaddr]
                    add_field_to_periph_config_struct $device_id $canonical_name

		    puts $file_handle [format "#define XPAR_%s_CONNECTED_FIFO_INTR 0xFF" $canonical_tag]
                    add_field_to_periph_config_struct $device_id 0xFF
		    set dmarx_signal [format "s2mm_introut"]
                    set dmatx_signal [format "mm2s_introut"]
		}
		incr device_id

	}

       if {$validentry !=1} {
		 puts "*******************************************************************************\r\n"
		 puts "The target Peripheral(MCDMA) is not connected properly to the XXV Ethernet core."
		 puts "*******************************************************************************\r\n"
      }
   }
}

proc xdefine_temac_params_include_file {file_handle periph device_id} {
	puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"

	puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "DEVICE_ID"] $device_id"
	puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "BASEADDR"] [common::get_property CONFIG.C_BASEADDR $periph]"
	puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "HIGHADDR"] [common::get_property CONFIG.C_HIGHADDR $periph]"

	set value [common::get_property CONFIG.Statistics_Counters $periph]
	set value [is_property_set $value]
	puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "STATS"] $value"

	set phyaddr [common::get_property CONFIG.PHYADDR $periph]
	set value [::hsi::utils::convert_binary_to_decimal $phyaddr]
	if {[llength $value] == 0} {
		set value 0
	}
	puts $file_handle "\#define [::hsi::utils::get_driver_param_name $periph "PHYADDR"] $value"
}

# ------------------------------------------------------------------
# This procedure creates XPARs that are canonical/normalized for the
# hardware design parameters.  It also adds these to the Config table.
# ------------------------------------------------------------------
proc xdefine_temac_params_canonical {file_handle periph device_id} {

    puts $file_handle "\n/* Canonical definitions for peripheral [string toupper [get_property NAME $periph]] */"

    set canonical_tag [string toupper [format "XPAR_XXVETHERNET_%d" $device_id]]

    # Handle device ID
    set canonical_name  [format "%s_DEVICE_ID" $canonical_tag]
    puts $file_handle "\#define $canonical_name $device_id"
    add_field_to_periph_config_struct $device_id $canonical_name

    # Handle BASEADDR specially
    set canonical_name  [format "%s_BASEADDR" $canonical_tag]
    puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_BASEADDR]"
    add_field_to_periph_config_struct $device_id $canonical_name

    # Handle HIGHADDR specially
    set canonical_name  [format "%s_HIGHADDR" $canonical_tag]
    puts $file_handle "\#define $canonical_name [::hsi::utils::get_param_value $periph C_HIGHADDR]"

    set canonical_name  [format "%s_STATS" $canonical_tag]
    set value [::hsi::utils::get_param_value $periph Statistics_Counters]
    set value [is_property_set $value]
    puts $file_handle "\#define $canonical_name $value"
    add_field_to_periph_config_struct $device_id $canonical_name

    set canonical_name  [format "%s_PHYADDR" $canonical_tag]
    set phyaddr [::hsi::utils::get_param_value $periph PHYADDR]
    set value [::hsi::utils::convert_binary_to_decimal $phyaddr]
    if {[llength $value] == 0} {
        set value 0
    }
    puts $file_handle "\#define $canonical_name $value"

}

# ------------------------------------------------------------------
# Create configuration C file as required by Xilinx drivers
# Use the config field list technique
# ------------------------------------------------------------------
proc xdefine_xxvethernet_config_file {file_name drv_string} {
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
        foreach field [get_periph_config_struct_fields $i] {
            puts -nonewline $config_file [format "%s\t\t%s" $comma $field]
            set comma ",\n"
        }

        puts -nonewline $config_file "\n\t\}"
        set start_comma ",\n"
    }
    puts $config_file "\n\};\n"
    close $config_file
}

# ------------------------------------------------------------------------------------
# This procedure re-forms the XXV Ethernet interrupt ID XPAR constant and adds it to
# the driver config table. This Tcl needs to be careful of the order of the
# config table entries
# ------------------------------------------------------------------------------------
proc xdefine_temac_interrupt {file_handle periph device_id} {

    #set mhs_handle [xget_hw_parent_handle $periph]
    set periph_name [string toupper [get_property NAME $periph]]

    # set up the canonical constant name
    set canonical_name [format "XPAR_XXVETHERNET_%d_INTR" $device_id]

    #
    # In order to reform the XPAR for the interrupt ID, we need to hunt
    # for the interrupt ID based on the interrupt signal name of the TEMAC
    #
      # First get the interrupt ports on this peripheral
    set interrupt_port  [::hsi::get_pins -of_objects $periph -filter {TYPE==INTERRUPT&&DIRECTION==O}]
    if {$interrupt_port == ""} {
	puts "Info: There are no XXV Ethernet Interrupt ports"
	# No interrupts were connected, so add dummy entry to the config structure
	puts $file_handle [format "#define $canonical_name 0xFF"]
        add_field_to_periph_config_struct $device_id 0xFF
        return
    }

    set addentry 0
    # For each interrupt port, find out the ordinal of the interrupt line
    #  as connected to an interrupt controller
    set interrupt_signal_name [get_property NAME $interrupt_port]
    #set interrupt_signal [xget_hw_value $interrupt_port]
    set intc_prt [::hsi::utils::get_sink_pins [get_pins -of_objects [get_cells -hier $periph] INTERRUPT]]

    # Make sure the interrupt signal was connected in this design. We assume
    # at least one is. (could be a bug if user just wants polled mode)
    if { $intc_prt != "" } {
        set intc_periph [::hsi::utils::get_connected_intr_cntrl $periph [get_pins -of_objects [get_cells -hier $periph] INTERRUPT] ]
        if {$intc_periph == ""} {
                puts "Info: Xxv Ethernet interrupt not connected to intc\n"
                # No interrupts were connected, so add dummy entry to the config structure
                puts $file_handle [format "#define $canonical_name 0xFF"]
                add_field_to_periph_config_struct $device_id 0xFF
                return
        }

        set intc_periph_type [get_property IP_NAME $intc_periph]
        set intc_name [string toupper [get_property NAME $intc_periph]]
	#Handling for ZYNQMP
	if { [llength $intc_periph_type] > 1 } {
		set intc_periph_type [lindex $intc_periph_type 1]
	}
    } else {
         puts "Info: $periph_name interrupt signal $interrupt_signal_name not connected"
         # No interrupts were connected, so add dummy entry to the config structure
         puts $file_handle [format "#define $canonical_name 0xFF"]
         add_field_to_periph_config_struct $device_id 0xFF
         return
    }

    # A bit of ugliness here. The only way to figure the ordinal is to
    # iterate over the interrupt lines again and see if a particular signal
    # matches the original interrupt signal we were tracking.
    set proc  [hsi::get_sw_processor];
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc]]
    if { $intc_periph_type != [format "ps7_scugic"]  && $intc_periph_type != [format "psu_acpu_gic"] && $proc_type != "psu_pmu"} {
	 set ethernet_int_signal_name [get_pins -of_objects $periph INTERRUPT]
	 set int_id [::hsi::utils::get_port_intr_id $periph $ethernet_int_signal_name]
	 puts $file_handle "\#define $canonical_name $int_id"
         add_field_to_periph_config_struct $device_id $canonical_name
	 set addentry 1
    } elseif { $proc_type != "psu_pmu"} {
        puts $file_handle [format "#define $canonical_name XPAR_FABRIC_%s_INTERRUPT_INTR" $periph_name]
        add_field_to_periph_config_struct $device_id $canonical_name
	set addentry 1
    }

    if { $addentry == 0 } {
        # No interrupts were connected, so add dummy entry to the config structure
        puts $file_handle [format "#define $canonical_name 0xFF"]
        add_field_to_periph_config_struct $device_id 0xFF
    }
}

proc is_property_set {value} {
	if {[string compare -nocase $value "true"] == 0} {
		set value 1
	} else {
		set value 0
	}

	return $value
}
