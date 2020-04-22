###############################################################################
# Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 1.0      mus    03/16/19 First Release.
#          mn     09/05/19 Correct the variable name for C0_DDR_CH2 block
#          mn     09/12/19 Check only for initial block name for DDR region
#          mn     04/22/20 Update the tcl to check C1, C2 and C3 regions
#
###############################################################################

proc generate {drv_handle} {
      define_addr_params $drv_handle "xparameters.h"
}

proc define_addr_params {drv_handle file_name} {

   # Open include file
   set file_handle [::hsi::utils::open_include_file $file_name]
   set sw_proc_handle [hsi::get_sw_processor]
   set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
   set proctype [common::get_property IP_NAME $hw_proc_handle]

   #Flags to check if specific DDR region is present in design
   set is_ddr_low_0 0
   set is_ddr_low_1 0
   set is_ddr_low_2 0
   set is_ddr_low_3 0
   set is_ddr_ch_1 0
   set is_ddr_ch_2 0
   set is_ddr_ch_3 0

   set sw_proc [hsi::get_sw_processor]
   set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

   foreach periph $periphs {
	set ipname [string toupper [common::get_property NAME $periph]]
	puts $file_handle ""
	puts $file_handle "/* Definitions for peripheral $ipname */"
	set interface_block_names [get_property ADDRESS_BLOCK [get_mem_ranges \
                -of_objects [get_cells -hier $sw_proc] $periph]]

	set i 0
	foreach block_name $interface_block_names {

		if {[string match "C*_DDR_LOW0*" $block_name]} {
			if {$is_ddr_low_0 == 0} {
				set base_value_0 [common::get_property BASE_VALUE [lindex [get_mem_ranges \
					-of_objects [get_cells -hier $sw_proc] $periph] $i]]
			}
			set high_value_0 [common::get_property HIGH_VALUE [lindex [get_mem_ranges \
                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
			set is_ddr_low_0 1

		} elseif {[string match "C*_DDR_LOW1*" $block_name]} {
                        if {$is_ddr_low_1 == 0} {
                                set base_value_1 [common::get_property BASE_VALUE [lindex [get_mem_ranges \
                                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        }
                        set high_value_1 [common::get_property HIGH_VALUE [lindex [get_mem_ranges \
                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        set is_ddr_low_1 1

		} elseif {[string match "C*_DDR_LOW2*" $block_name]} {
                        if {$is_ddr_low_2 == 0} {
                                set base_value_2 [common::get_property BASE_VALUE [lindex [get_mem_ranges \
                                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        }
                        set high_value_2 [common::get_property HIGH_VALUE [lindex [get_mem_ranges \
                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        set is_ddr_low_2 1

		} elseif {[string match "C*_DDR_LOW3*" $block_name]} {
                        if {$is_ddr_low_3 == "0"} {
                                set base_value_3 [common::get_property BASE_VALUE [lindex [get_mem_ranges \
                                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        }
                        set high_value_3 [common::get_property HIGH_VALUE [lindex [get_mem_ranges \
                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        set is_ddr_low_3 1
		} elseif {[string match "C*_DDR_CH1*" $block_name]} {
                        if {$is_ddr_ch_1 == "0"} {
                                set base_value_4 [common::get_property BASE_VALUE [lindex [get_mem_ranges \
                                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        }
                        set high_value_4 [common::get_property HIGH_VALUE [lindex [get_mem_ranges \
                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        set is_ddr_ch_1 1
		} elseif {[string match "C*_DDR_CH2*" $block_name]} {
                        if {$is_ddr_ch_2 == "0"} {
                                set base_value_5 [common::get_property BASE_VALUE [lindex [get_mem_ranges \
                                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        }
                        set high_value_5 [common::get_property HIGH_VALUE [lindex [get_mem_ranges \
                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        set is_ddr_ch_2 1
		} elseif {[string match "C*_DDR_CH3*" $block_name]} {
                        if {$is_ddr_ch_3 == "0"} {
                                set base_value_6 [common::get_property BASE_VALUE [lindex [get_mem_ranges \
                                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        }
                        set high_value_6 [common::get_property HIGH_VALUE [lindex [get_mem_ranges \
                        -of_objects [get_cells -hier $sw_proc] $periph] $i]]
                        set is_ddr_ch_3 1
		}

		incr i

	}

        if {$is_ddr_low_0 == 1} {
		if {$base_value_0 == 0 && ($proctype == "psu_cortexr5" || $proctype == "psv_cortexr5")} {
			 puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_0_BASEADDR 0x00100000"
		} else {
                         puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_0_BASEADDR $base_value_0"
		}
                puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_0_HIGHADDR $high_value_0"
                puts $file_handle ""
        }

        if {$is_ddr_low_1 == 1} {
                puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_1_BASEADDR $base_value_1"
                puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_1_HIGHADDR $high_value_1"
                puts $file_handle ""
        }


        if {$is_ddr_low_2 == 1} {
                puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_2_BASEADDR $base_value_2"
                puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_2_HIGHADDR $high_value_2"
                puts $file_handle ""
        }


	if {$is_ddr_low_3 == 1} {
		puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_3_BASEADDR $base_value_3"
		puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_3_HIGHADDR $high_value_3"
		puts $file_handle ""
	}


	if {$is_ddr_ch_1 == 1} {
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_1_BASEADDR $base_value_4"
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_1_HIGHADDR $high_value_4"
		puts $file_handle ""
	}
	
	
	if {$is_ddr_ch_2 == 1} {
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_2_BASEADDR $base_value_5"
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_2_HIGHADDR $high_value_5"
		puts $file_handle ""
	}


	if {$is_ddr_ch_3 == 1} {
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_3_BASEADDR $base_value_6"
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_3_HIGHADDR $high_value_6"
		puts $file_handle ""
	}

   }

   puts $file_handle "\n/******************************************************************/\n"
   close $file_handle
}
