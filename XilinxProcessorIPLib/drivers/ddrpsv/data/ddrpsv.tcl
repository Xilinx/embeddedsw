###############################################################################
# Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
# 1.3      mus    05/08/20 Updated tcl to include detailed #defines for all
#                          NOC IPs present in the design. Also, updated logic
#                          to create canonical #defines.
#
#                          In case, if more than 1 NOC IP in design is connected
#                          to same DDR segment through different address range,
#                          generated #defines could be wrong, as existing logic
#                          doesnt generate unique #define, so names would be
#                          repeated and values would be wrong in certain
#                          scenarios.
#
#                          Now, new logic is adding 2 major enhancements. First
#                          one is, it is creating detailed #defines for each
#                          connection to the DDR segment, which includes NOC IP
#                          instance name, DDR segment name and master
#                          interface name, it would avoid repeatation in names.
#                          So, xparameters.h would reflect each NOC connection
#                          in HW design.
#                          Second enhancement is, it would create canonicals
#                          for each DDR segment, which would be consumed by
#                          translation table/MPU in BSP. Base address canonical
#                          would point to lowest base address value for that
#                          specific DDR segemnt in HW design, and high address
#                          canonical would point to highest high address for
#                          that DDR segment in given HW design.
#
#                          Limitation which still exist: In case, if
#                          different NOC instances are connected to same DDR
#                          region through different address ranges, we are
#                          assuming that there is no hole in that address
#                          ranges. If HW design contains holes in those
#                          address ranges, canonical #defines would point to
#                          base address and high address with holes.
#                          For example, specific HW design is having 2 NOC
#                          instances NOC1 and NOC2. NoC1 is connected to
#                          DDR_LOW_0 with base address 0x0000,
#                          and high address as 0x3FFF. NOC2 is connected to
#                          DDR_LOW_0 with base address as 0x4000_0000 and high
#                          address as 0x4FFF_FFFF. Now, as per logic,
#                          generated base address canonical for DDR_LOW_0
#                          would point to  0x0 and high address canonical
#                          points to 0x4FFF_FFFF. It is incorrect as, there is
#                          hole in address range starting from 0x4000 to
#                          0x3FFF_FFFF.
# 1.4   mus   08/09/21     Updated generate proc to add checks for psv_pmc and
#                          psv_psm processor. As macros exported by this tcl are
#                          consumed by only ARM based BSP's, we can skip
#                          it for firmware processor BSP. It fixes CR#1105828
# 1.5   sg    08/04/23     Added VersalNet support
# 1.6   ml    10/22/24     Update to look for AXI NOC2 as well in Versal platform
#                          to generate related DDR macros
#
###############################################################################
set file_handle 0
set sw_proc 0
set is_versal_net 0

# HW designs can have multiple NOC IPs, and each of them can be connected to
# same DDR segment with different address ranges, and through different
# master interface channels.
# For each DDR segment, NOC IP interface whose base address is lowest in the design
# would be stored in base_addr_list and highest high address would be stored in
# high_address_list. Index of base_address_list/high_address_list where base/high
# address for specific DDR segment/DDR region is stored is as given below. These
# lowest and highest addresses would be used to create canonical definitions, which
# would be consumed by MMU/MPU tables in Cortex-A72/Cortex-R5 BSP.
# ---------------------------------------
# DDR segment        |       Index       |
#--------------------|-------------------|
#  DDR_LOW_0         |        0          |
#  DDR_LOW_1         |        1          |
#  DDR_LOW_2         |        2          |
#  DDR_LOW_3         |        3          |
#  DDR_CH_1          |        4          |
#  DDR_CH_2          |        5          |
#  DDR_CH_3          |        6          |
#----------------------------------------
#
#Versal-Net
#---------------------------------------
# DDR segment        |       Index       |
#--------------------|-------------------|
#  DDR_CH0_LEGACY    |        0          |
#  DDR_CH0_MED       |        1          |
#  DDR_CH0_HIGH0     |        2          |
#  DDR_CH0_HIGH1     |        3          |
#  DDR_CH_1          |        4          |
#  DDR_CH_1A         |        5          |
#  DDR_CH_2	     |	      6		 |
#  DDR_CH_2A         |        7		 |
#  DDR_CH_3          |        8		 |
#  DDR_CH_3A 	     |        9		 |
#  DDR_CH_4          |        10         |
#----------------------------------------

#
global base_addr_list
global high_addr_list

proc generate {drv_handle} {
      set sw_proc_handle [hsi::get_sw_processor]
      set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
      set proctype [common::get_property IP_NAME $hw_proc_handle ]
      set is_versal_net [hsi::get_cells -hier -filter {IP_NAME=="psxl_cortexr52" || \
	      IP_NAME=="psx_cortexr52" || IP_NAME=="psxl_cortexa78" || IP_NAME=="psx_cortexa78"}]

      # define_addr_params macro proc exports macros related to available DDR regions to
      # xparameters.h. Those macro's are comsumed by MPU/MMU tables in ARM based BSPs.
      if { $proctype != "psv_pmc" && $proctype != "psv_psm" && \
	      $proctype != "psx_pmc" && $proctype != "psx_psm" } {
		if {[llength $is_versal_net] > 0} {
	              define_addr_params_versal_net $drv_handle "xparameters.h"
		} else {
	              define_addr_params $drv_handle "xparameters.h"
		}
      }
}

proc define_addr_params {drv_handle file_name} {
   global file_handle
   global sw_proc
   global base_addr_list
   global high_addr_list

   set base_addr_list {0 0 0 0 0 0 0}
   set high_addr_list {0 0 0 0 0 0 0}
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
                -of_objects [hsi::get_cells -hier $sw_proc] $periph]]

	set i 0
	foreach block_name $interface_block_names {

		if {[string match "C*_DDR_LOW0*" $block_name] || [string match "C*_DDR_CH0_LEGACY*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 0
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_low_0 $ddr_region_id
			set is_ddr_low_0 1

		} elseif {[string match "C*_DDR_LOW1*" $block_name] || [string match "C*_DDR_CH0_MED*" $block_name]} {
					#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 1
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_low_1 $ddr_region_id
			set is_ddr_low_1 1

		} elseif {[string match "C*_DDR_LOW2*" $block_name] || [string match "C*_DDR_CH0_HIGH_0*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 2
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_low_2 $ddr_region_id


			set is_ddr_low_2 1

		} elseif {[string match "C*_DDR_LOW3*" $block_name] || [string match "C*_DDR_CH0_HIGH_1*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 3
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_low_3 $ddr_region_id


			set is_ddr_low_3 1

		} elseif {[string match "C*_DDR_CH1*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 4
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_ch_1 $ddr_region_id

			set is_ddr_ch_1 1
		} elseif {[string match "C*_DDR_CH2*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 5
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_ch_2 $ddr_region_id

			set is_ddr_ch_2 1
		} elseif {[string match "C*_DDR_CH3*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 6
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_ch_3  $ddr_region_id

			set is_ddr_ch_3 1
		}

		incr i
	}
   }

	puts $file_handle ""
	puts $file_handle "/* Canonicals definitions for NOC DDR to be consumed by MMU/MPU tables in BSP*/"
	if {$is_ddr_low_0 == 1} {
		set ddr_region_id 0
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_0_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_0_HIGHADDR $value"
		puts $file_handle ""

	}

	if {$is_ddr_low_1 == 1} {
		set ddr_region_id 1
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_1_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_1_HIGHADDR $value"
		puts $file_handle ""
	}

	if {$is_ddr_low_2 == 1} {
		set ddr_region_id 2
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_2_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_2_HIGHADDR $value"
		puts $file_handle ""
	}

	if {$is_ddr_low_3 == 1} {
		set ddr_region_id 3
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_3_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_LOW_3_HIGHADDR $value"
		puts $file_handle ""
	}

	if {$is_ddr_ch_1 == 1} {
		set ddr_region_id 4
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle ""
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_1_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_1_HIGHADDR $value"
		puts $file_handle ""
	}

	if {$is_ddr_ch_2 == 1} {
		set ddr_region_id 5
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_2_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_2_HIGHADDR $value"
		puts $file_handle ""
	}


	if {$is_ddr_ch_3 == 1} {
		set ddr_region_id 6
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_3_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC_DDR_CH_3_HIGHADDR $value"
		puts $file_handle ""
	}
   puts $file_handle "\n/******************************************************************/\n"
   close $file_handle
}

proc define_addr_params_versal_net {drv_handle file_name} {
   global file_handle
   global sw_proc
   global base_addr_list
   global high_addr_list

   set base_addr_list {0 0 0 0 0 0 0 0 0 0 0}
   set high_addr_list {0 0 0 0 0 0 0 0 0 0 0}
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
   set is_ddr_ch_1  0
   set is_ddr_ch_1a 0
   set is_ddr_ch_2  0
   set is_ddr_ch_2a 0
   set is_ddr_ch_3  0
   set is_ddr_ch_3a 0
   set is_ddr_ch_4  0

   set sw_proc [hsi::get_sw_processor]
   set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

   foreach periph $periphs {
	set ipname [string toupper [common::get_property NAME $periph]]
	puts $file_handle ""
	puts $file_handle "/* Definitions for peripheral $ipname */"
	set interface_block_names [get_property ADDRESS_BLOCK [get_mem_ranges \
                -of_objects [hsi::get_cells -hier $sw_proc] $periph]]

	set i 0
	foreach block_name $interface_block_names {

		if {[string match "C*_DDR_CH0_LEGACY*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 0
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_low_0 $ddr_region_id
			set is_ddr_low_0 1

		} elseif {[string match "C*_DDR_CH0_MED*" $block_name]} {
					#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 1
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_low_1 $ddr_region_id
			set is_ddr_low_1 1

		} elseif {[string match "C*_DDR_CH0_HIGH_0*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 2
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_low_2 $ddr_region_id

			set is_ddr_low_2 1

		} elseif {[string match "C*_DDR_CH0_HIGH_1*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 3
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_low_3 $ddr_region_id

			set is_ddr_low_3 1
		} elseif {[string match "C*_DDR_CH1*" $block_name]} {
                        #
                        # ddr_region_id specifies index of base_addr_list/high_addr_list
                        # for this DDR region
                        #
                        set ddr_region_id 4
                        xddrpsv_handle_address_details $i $periph $block_name $is_ddr_ch_1 $ddr_region_id

                        set is_ddr_ch_1 1
                } elseif {[string match "C*_DDR_CH1A*" $block_name]} {
                        #
                        # ddr_region_id specifies index of base_addr_list/high_addr_list
                        # for this DDR region
                        #
                        set ddr_region_id 5
                        xddrpsv_handle_address_details $i $periph $block_name $is_ddr_ch_1a $ddr_region_id

                        set is_ddr_ch_1a 1
                } elseif {[string match "C*_DDR_CH2*" $block_name]} {
                        #
                        # ddr_region_id specifies index of base_addr_list/high_addr_list
                        # for this DDR region
                        #
                        set ddr_region_id 6
                        xddrpsv_handle_address_details $i $periph $block_name $is_ddr_ch_2  $ddr_region_id

                        set is_ddr_ch_2 1

		} elseif {[string match "C*_DDR_CH2A*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 7
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_ch_2a $ddr_region_id

			set is_ddr_ch_2a 1
		} elseif {[string match "C*_DDR_CH3*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 8
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_ch_3 5

			set is_ddr_ch_3 1
		} elseif {[string match "C*_DDR_CH3A*" $block_name]} {
			#
			# ddr_region_id specifies index of base_addr_list/high_addr_list
			# for this DDR region
			#
			set ddr_region_id 9
			xddrpsv_handle_address_details $i $periph $block_name $is_ddr_ch_3a  $ddr_region_id

			set is_ddr_ch_3a 1
		} elseif {[string match "C*_DDR_CH4*" $block_name]} {
                        #
                        # ddr_region_id specifies index of base_addr_list/high_addr_list
                        # for this DDR region
                        #
                        set ddr_region_id 10
                        xddrpsv_handle_address_details $i $periph $block_name $is_ddr_ch_4  $ddr_region_id

                        set is_ddr_ch_4 1
                }
		incr i
	}

   }

	puts $file_handle ""
	puts $file_handle "/* Canonicals definitions for NOC2 DDR to be consumed by MMU/MPU tables in BSP*/"
	if {$is_ddr_low_0 == 1} {
		set ddr_region_id 0
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_LOW_0_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_LOW_0_HIGHADDR $value"
		puts $file_handle ""

	}

	if {$is_ddr_low_1 == 1} {
		set ddr_region_id 1
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_LOW_1_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_LOW_1_HIGHADDR $value"
		puts $file_handle ""
	}

	if {$is_ddr_low_2 == 1} {
		set ddr_region_id 2
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_LOW_2_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_LOW_2_HIGHADDR $value"
		puts $file_handle ""
	}

	if {$is_ddr_low_3 == 1} {
		set ddr_region_id 3
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_LOW_3_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_LOW_3_HIGHADDR $value"
		puts $file_handle ""
	}

	if {$is_ddr_ch_1 == 1} {
		set ddr_region_id 4
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle ""
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_1_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_1_HIGHADDR $value"
		puts $file_handle ""
	}

	if {$is_ddr_ch_1a == 1} {
		set ddr_region_id 5
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_1A_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_1A_HIGHADDR $value"
		puts $file_handle ""
	}

	if {$is_ddr_ch_2 == 1} {
                set ddr_region_id 6
                set value  [lindex $base_addr_list $ddr_region_id]
                puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_2_BASEADDR $value"

                set value  [lindex $high_addr_list $ddr_region_id]
                puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_2_HIGHADDR $value"
                puts $file_handle ""
        }

	if {$is_ddr_ch_2a == 1} {
		set ddr_region_id 7
		set value  [lindex $base_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_2A_BASEADDR $value"

		set value  [lindex $high_addr_list $ddr_region_id]
		puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_2A_HIGHADDR $value"
		puts $file_handle ""
	}

	if {$is_ddr_ch_3 == 1} {
                set ddr_region_id 8
                set value  [lindex $base_addr_list $ddr_region_id]
                puts $file_handle ""
                puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_3_BASEADDR $value"

                set value  [lindex $high_addr_list $ddr_region_id]
                puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_3_HIGHADDR $value"
                puts $file_handle ""
        }

        if {$is_ddr_ch_3a == 1} {
                set ddr_region_id 9
                set value  [lindex $base_addr_list $ddr_region_id]
                puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_3A_BASEADDR $value"

                set value  [lindex $high_addr_list $ddr_region_id]
                puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_3A_HIGHADDR $value"
                puts $file_handle ""
        }

        if {$is_ddr_ch_4 == 1} {
                set ddr_region_id 10
                set value  [lindex $base_addr_list $ddr_region_id]
                puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_4_BASEADDR $value"

                set value  [lindex $high_addr_list $ddr_region_id]
                puts $file_handle "#define XPAR_AXI_NOC2_DDR_CH_4_HIGHADDR $value"
                puts $file_handle ""
        }


   puts $file_handle "\n/******************************************************************/\n"
   close $file_handle
}

proc xddrpsv_get_base_addr { sw_proc periph index } {
	return [common::get_property BASE_VALUE [lindex [get_mem_ranges \
					-of_objects [hsi::get_cells -hier $sw_proc] $periph] $index]]
}

proc xddrpsv_get_high_addr { sw_proc periph index } {
	return [common::get_property HIGH_VALUE [lindex [get_mem_ranges \
					-of_objects [hsi::get_cells -hier $sw_proc] $periph] $index]]
}

proc xddrpsv_generate_defines { periph block_name master_interface value is_baseaddr } {
	global file_handle
	global sw_proc

	if { $is_baseaddr == 1 } {
		if {$value == 0 && ($sw_proc == "psu_cortexr5" || $sw_proc == "psv_cortexr5" \
			|| $sw_proc =="psxl_cortexr52" || $sw_proc =="psx_cortexr52")} {
			puts $file_handle [format "#define XPAR_%s_%s_$s_BASEADDR 0x00100000" \
				[string toupper $periph] [string toupper $block_name] [string toupper $master_interface]]
		} else {
			puts $file_handle [format "#define XPAR_%s_%s_%s_BASEADDR %s" \
			[string toupper $periph] [string toupper $block_name] [string toupper $master_interface] $value]
		}
	} else {
		puts $file_handle [format "#define XPAR_%s_%s_%s_HIGHADDR %s" \
		[string toupper $periph] [string toupper $block_name] [string toupper $master_interface] $value]
	}
}

proc xddrpsv_handle_address_details { index periph block_name is_ddr_region_accessed ddr_region_id } {
	#Variables to hold base address and high address of each DDR region
	global sw_proc
	global base_addr_list
	global high_addr_list

	set temp [xddrpsv_get_base_addr $sw_proc $periph $index]
	if { $is_ddr_region_accessed == 0 || [string compare $temp [lindex $base_addr_list $ddr_region_id]] < 0 } {
		lset base_addr_list $ddr_region_id $temp
	}
	set master_interface [get_property MASTER_INTERFACE [lindex [ get_mem_ranges -of_objects \
		[hsi::get_cells -hier $sw_proc] $periph] $index]]
	set is_baseaddr 1
	xddrpsv_generate_defines $periph $block_name $master_interface $temp $is_baseaddr

	set temp [xddrpsv_get_high_addr $sw_proc $periph $index]
	if { $is_ddr_region_accessed == 0 || [string compare $temp [lindex $high_addr_list $ddr_region_id]] > 0} {
		lset high_addr_list $ddr_region_id $temp
	}
	set is_baseaddr 0
	xddrpsv_generate_defines $periph $block_name $master_interface $temp $is_baseaddr

}
