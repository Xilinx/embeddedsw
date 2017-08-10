###############################################################################
#
# Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
# XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
###############################################################################
###############################################################################
#
# Modification History
#
# Ver  Who     Date         Changes
# --- ---- ------------  ---------------------------------------------------
# 1.0  ms   07/18/2016     First release
# 1.1  ms   04/18/17       Modified tcl file to add suffix U for all macros
#                          definitions of prc in xparameters.h
#      ms   08/01/17	    Added a new parameter "Cp_Compression" in
#                          Peripheral, canonical definitions of xparameters.h
#                          and also in xprc_g.c.
#                          Modified version from prc_v1_1 to prc_v1_2 as
#                          api.tcl which is source for prc.tcl got updated.
###############################################################################

#uses "xillib.tcl"

source api.tcl -notrace

proc init_periph_config_struct_prc { deviceid } {
			global periph_config_params_prc
			set periph_config_params_prc($deviceid) [list]
		}
proc prc_generate_params {drv_handle file_name} {
	#Driver Prefix String
	set drv_string "XPrc"

	# open the xparameters.h file
	set file_handle [::hsi::utils::open_include_file $file_name]

	set uSuffix "U"
	# Get the ALL_PARAMS property
	# I've only managed to do this by looping through all instances of the PRC in the design.
	# Does generate get called for all instances, or for a specific instance?  If it's a specific
	# instance, how do I know which one it is?

	set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
	foreach periph $periphs {
		set configuration	[common::get_property CONFIG.ALL_PARAMS $periph]

		# Use the PRC's API to get the number of VSMs that the user configured in this instance of the PRC
		set num_vs  [prc_v1_2::priv::get_num_vs configuration]

		set clearing_bitstream [prc_v1_2::priv::requires_clear_bitstream configuration]
		set cp_arbitration_protocol  [prc_v1_2::priv::get_cp_arbitration_protocol configuration]
		set has_axi_lite_if [prc_v1_2::priv::get_has_axi_lite_if configuration]
		set reset_active_level [prc_v1_2::priv::get_reset_active_level configuration]
		set cp_fifo_depth [prc_v1_2::priv::get_cp_fifo_depth configuration]
		set cp_fifo_type [prc_v1_2::priv::get_cp_fifo_type_as_int configuration]
		set cp_family [prc_v1_2::priv::get_cp_family_as_int configuration]
		set cdc_stages [prc_v1_2::priv::get_cdc_stages configuration]
		set cp_compression [prc_v1_2::priv::get_cp_compression configuration]

		set address_offsets [prc_v1_2::priv::calculate_address_offsets configuration]

		set C_REG_SELECT_MSB	[dict get $address_offsets C_REG_SELECT_MSB  ]
		set C_REG_SELECT_LSB	[dict get $address_offsets C_REG_SELECT_LSB  ]
		set C_TABLE_SELECT_MSB	[dict get $address_offsets C_TABLE_SELECT_MSB]
		set C_TABLE_SELECT_LSB	[dict get $address_offsets C_TABLE_SELECT_LSB]
		set C_VSM_SELECT_MSB	[dict get $address_offsets C_VSM_SELECT_MSB  ]
		set C_VSM_SELECT_LSB	[dict get $address_offsets C_VSM_SELECT_LSB  ]

		# Handle NUM_INSTANCES
	    set periph_ninstances 0
	    puts $file_handle "/* Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
		foreach periph $periphs {
		init_periph_config_struct_prc $periph_ninstances
		incr periph_ninstances 1
	    }
		 puts $file_handle "\#define [::hsi::utils::get_driver_param_name $drv_string NUM_INSTANCES] $periph_ninstances$uSuffix"
		puts $file_handle ""

	# Now print all useful parameters for all peripherals
	    set device_id 0

		set baseaddr  [common::get_property CONFIG.C_BASEADDR $periphs]

		foreach periph $periphs {
			puts $file_handle [format "/* Definitions for peripheral PRC */ "]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "DEVICE_ID"  $device_id]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "BASEADDR"  $baseaddr]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "NUM_OF_VSMS" $num_vs]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "CLEARING_BITSTREAM" $clearing_bitstream]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "CP_ARBITRATION_PROTOCOL" $cp_arbitration_protocol]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "HAS_AXI_LITE_IF" $has_axi_lite_if]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "RESET_ACTIVE_LEVEL" $reset_active_level]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "CP_FIFO_DEPTH" $cp_fifo_depth]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "CP_FIFO_TYPE" $cp_fifo_type]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "CP_FAMILY" $cp_family]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "CDC_STAGES" $cdc_stages]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "CP_COMPRESSION" $cp_compression]

			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				set num_rms		[prc_v1_2::priv::get_num_rms_in_vs	configuration $vs_name]
				set num_rms_alloc	[prc_v1_2::priv::get_vs_num_rms_allocated	configuration $vs_name]
				set num_trger_alloc	[prc_v1_2::priv::get_vs_num_triggers_allocated	configuration $vs_name]
				set strt_in_shtdwn	[prc_v1_2::priv::get_vs_start_in_shutdown	configuration $vs_name]
				set shtdwn_on_err	[prc_v1_2::priv::get_vs_shutdown_on_error	configuration $vs_name]
				set has_por_rm	[prc_v1_2::priv::get_vs_has_por_rm	configuration $vs_name]
				set por_rm		[prc_v1_2::priv::get_vs_por_rm	configuration $vs_name]
				set rm_id		[prc_v1_2::priv::get_rm_id	configuration $vs_name $por_rm]
				set has_axs_status	[prc_v1_2::priv::get_vs_has_axis_status	configuration $vs_name]
				set has_axs_control	[prc_v1_2::priv::get_vs_has_axis_control	configuration $vs_name]
				set skp_rm_strtup_aft_rst	[prc_v1_2::priv::get_vs_skip_rm_startup_after_reset	configuration $vs_name]
				set num_hw_trgers	[prc_v1_2::priv::get_vs_num_hw_triggers	configuration $vs_name]

				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] "NUM_RMS" [string toupper $vs_name] $num_rms]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] "NUM_RMS_ALLOC" [string toupper $vs_name] $num_rms_alloc]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] "STRT_IN_SHTDOWN" [string toupper $vs_name] $strt_in_shtdwn]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] "NUM_TRGRS_ALLOC" [string toupper $vs_name] $num_trger_alloc]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] "SHUTDOWN_ON_ERR" [string toupper $vs_name] $shtdwn_on_err]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] "HAS_POR_RM" [string toupper $vs_name] $has_por_rm]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] "POR_RM" [string toupper $vs_name] $rm_id]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] "HAS_AXIS_STATUS" [string toupper $vs_name] $has_axs_status]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] "HAS_AXIS_CONTROL" [string toupper $vs_name] $has_axs_control]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] "SKIP_RM_STARTUP_AFTER_RESET" [string toupper $vs_name] $skp_rm_strtup_aft_rst]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] "NUM_HW_TRIGGERS" [string toupper $vs_name] $vs_id $num_hw_trgers]
			}

			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "VSM_SELECT_MSB" $C_VSM_SELECT_MSB]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "VSM_SELECT_LSB" $C_VSM_SELECT_LSB]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "TABLE_SELECT_MSB" $C_TABLE_SELECT_MSB]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "TABLE_SELECT_LSB" $C_TABLE_SELECT_LSB]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "REG_SELECT_MSB" $C_REG_SELECT_MSB]
			puts $file_handle [format "#define  XPAR_%s_%s  %s$uSuffix" [string toupper $periphs] "REG_SELECT_LSB" $C_REG_SELECT_LSB]
		}
		for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
			set vs_name	[prc_v1_2::priv::get_vs_name	configuration $vs_id]
			puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] [string toupper $vs_name] "ID" $vs_id]

			for {set rm_id 0} {$rm_id < $num_rms} {incr rm_id} {
				set rm_name	[prc_v1_2::priv::get_rm_name	configuration $vs_name $rm_id]
				puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] [string toupper $vs_name] [string toupper $rm_name] "ID" $rm_id]
			}
		}

		puts $file_handle ""
		puts $file_handle "/******************************************************************/"
		puts $file_handle ""
	}
	close $file_handle
}

proc prc_generate_canonical {drv_handle file_name} {

	# open the xparameters.h file
	set file_handle [::hsi::utils::open_include_file $file_name]

	# Get the ALL_PARAMS property
	# I've only managed to do this by looping through all instances of the PRC in the design.
	# Does generate get called for all instances, or for a specific instance?  If it's a specific
	# instance, how do I know which one it is?

	set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
	foreach periph $periphs {
		set configuration	[common::get_property CONFIG.ALL_PARAMS $periph]

		set uSuffix "U"

		# Use the PRC's API to get the number of VSMs that the user configured in this instance of the PRC
		set num_vs  [prc_v1_2::priv::get_num_vs configuration]

		set clearing_bitstream [prc_v1_2::priv::requires_clear_bitstream configuration]
		set cp_arbitration_protocol  [prc_v1_2::priv::get_cp_arbitration_protocol configuration]
		set has_axi_lite_if [prc_v1_2::priv::get_has_axi_lite_if configuration]
		set reset_active_level [prc_v1_2::priv::get_reset_active_level configuration]
		set cp_fifo_depth [prc_v1_2::priv::get_cp_fifo_depth configuration]
		set cp_fifo_type [prc_v1_2::priv::get_cp_fifo_type_as_int configuration]
		set cp_family [prc_v1_2::priv::get_cp_family_as_int configuration]
		set cdc_stages [prc_v1_2::priv::get_cdc_stages configuration]
		set cp_compression [prc_v1_2::priv::get_cp_compression configuration]

		set address_offsets [prc_v1_2::priv::calculate_address_offsets configuration]

		set C_REG_SELECT_MSB   [dict get $address_offsets C_REG_SELECT_MSB  ]
		set C_REG_SELECT_LSB   [dict get $address_offsets C_REG_SELECT_LSB  ]
		set C_TABLE_SELECT_MSB [dict get $address_offsets C_TABLE_SELECT_MSB]
		set C_TABLE_SELECT_LSB [dict get $address_offsets C_TABLE_SELECT_LSB]
		set C_VSM_SELECT_MSB   [dict get $address_offsets C_VSM_SELECT_MSB  ]
		set C_VSM_SELECT_LSB   [dict get $address_offsets C_VSM_SELECT_LSB  ]

		# Now print all useful parameters for all peripherals
	    set device_id 0

		set baseaddr  [common::get_property CONFIG.C_BASEADDR $periphs]
		set idx 0
			foreach periph $periphs {
			puts $file_handle [format "/* Canonical definitions for peripheral PRC */ "]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  XPAR_%s_%s" [string toupper $periphs] $idx "DEVICE_ID"  [string toupper $periphs] "DEVICE_ID"]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "BASEADDR"  $baseaddr]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "NUM_OF_VSMS" $num_vs]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "CLEARING_BITSTREAM" $clearing_bitstream]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "CP_ARBITRATION_PROTOCOL" $cp_arbitration_protocol]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "HAS_AXI_LITE_IF" $has_axi_lite_if]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "RESET_ACTIVE_LEVEL" $reset_active_level]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "CP_FIFO_DEPTH" $cp_fifo_depth]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "CP_FIFO_TYPE" $cp_fifo_type]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "CP_FAMILY" $cp_family]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "CDC_STAGES" $cdc_stages]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "CP_COMPRESSION" $cp_compression]

				for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
					set vs_name	[prc_v1_2::priv::get_vs_name	configuration $vs_id]
					set num_rms	[prc_v1_2::priv::get_num_rms_in_vs	configuration $vs_name]
					set num_rms_alloc	[prc_v1_2::priv::get_vs_num_rms_allocated	configuration $vs_name]
					set num_trgers_alloc	[prc_v1_2::priv::get_vs_num_triggers_allocated	configuration $vs_name]
					set strt_in_shtdwn	[prc_v1_2::priv::get_vs_start_in_shutdown	configuration $vs_name]
					set shtdwn_on_err	[prc_v1_2::priv::get_vs_shutdown_on_error	configuration $vs_name]
					set has_por_rm	[prc_v1_2::priv::get_vs_has_por_rm	configuration $vs_name]
					set por_rm	[prc_v1_2::priv::get_vs_por_rm	configuration $vs_name]
					set rm_id	[prc_v1_2::priv::get_rm_id	configuration $vs_name $por_rm]
					set has_axs_status	[prc_v1_2::priv::get_vs_has_axis_status	configuration $vs_name]
					set has_axs_control	[prc_v1_2::priv::get_vs_has_axis_control	configuration $vs_name]
					set skp_rm_strtup_aft_rst	[prc_v1_2::priv::get_vs_skip_rm_startup_after_reset	configuration $vs_name]
					set num_hw_trgers	[prc_v1_2::priv::get_vs_num_hw_triggers	configuration $vs_name]

					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "NUM_RMS" [string toupper $vs_name] $num_rms]
					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "NUM_RMS_ALLOC" [string toupper $vs_name] $num_rms_alloc]
					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "STRT_IN_SHTDOWN" [string toupper $vs_name] $strt_in_shtdwn]
					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "NUM_TRGRS_ALLOC" [string toupper $vs_name] $num_trgers_alloc]
					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "SHUTDOWN_ON_ERR" [string toupper $vs_name] $shtdwn_on_err]
					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "HAS_POR_RM" [string toupper $vs_name] $has_por_rm]
					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "POR_RM" [string toupper $vs_name] $rm_id]
					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "HAS_AXIS_STATUS" [string toupper $vs_name] $has_axs_status]
					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "HAS_AXIS_CONTROL" [string toupper $vs_name] $has_axs_control]
					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "SKIP_RM_STARTUP_AFTER_RESET" [string toupper $vs_name] $skp_rm_strtup_aft_rst]
					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "NUM_HW_TRIGGERS" [string toupper $vs_name] $num_hw_trgers]
				}

				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "VSM_SELECT_MSB" $C_VSM_SELECT_MSB]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "VSM_SELECT_LSB" $C_VSM_SELECT_LSB]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "TABLE_SELECT_MSB" $C_TABLE_SELECT_MSB]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "TABLE_SELECT_LSB" $C_TABLE_SELECT_LSB]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "REG_SELECT_MSB" $C_REG_SELECT_MSB]
				puts $file_handle [format "#define  XPAR_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx "REG_SELECT_LSB" $C_REG_SELECT_LSB]

				for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
					set vs_name	[prc_v1_2::priv::get_vs_name	configuration $vs_id]
					puts $file_handle [format "#define  XPAR_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx [string toupper $vs_name] "ID" $vs_id]

					for {set rm_id 0} {$rm_id < $num_rms} {incr rm_id} {
						set rm_name	[prc_v1_2::priv::get_rm_name	configuration $vs_name $rm_id]
						puts $file_handle [format "#define  XPAR_%s_%s_%s_%s_%s  %s$uSuffix" [string toupper $periphs] $idx [string toupper $vs_name] [string toupper $rm_name] "ID" $rm_id]
					}
				}
				incr device_id
				incr idx


			}

		puts $file_handle ""
		puts $file_handle "/******************************************************************/"
		puts $file_handle ""
	}
	close $file_handle
}

proc prc_generate_config {drv_handle file_name} {

	#Driver Prefix String
	set drv_string "XPrc"

	#The current processor
	set sw_proc_handle [::hsi::get_sw_processor]
	set hw_proc_handle [::hsi::get_cells -hier [common::get_property hw_instance $sw_proc_handle]]

	# List of PRCs owned by this processor
	set proc_prc_list [lsearch -all -inline [get_property SLAVES $hw_proc_handle] prc_*]

	# List of all IPIs on SoC
	set prc_list [get_cells -hier -filter { IP_NAME == "prc" }]

	set cfgfilename [file join "src" $file_name]
	set config_file [open $cfgfilename w]

	# Get the ALL_PARAMS property

	set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
	foreach periph $periphs {
		set configuration    [common::get_property CONFIG.ALL_PARAMS $periph]

		# Use the PRC's API to get the number of VSMs that the user configured in this instance of the PRC
		set num_vs  [prc_v1_2::priv::get_num_vs configuration]

		# Common Header
		::hsi::utils::write_c_header $config_file "Driver configuration"
		puts $config_file "#include \"xparameters.h\""
		puts $config_file "#include \"[string tolower $drv_string].h\""

		# Start generating the  Config table
		puts $config_file "\n/*"
		puts $config_file "* The configuration table for devices"
		puts $config_file "*/\n"
		puts $config_file [format "%s_Config %s_ConfigTable\[\] = \{" $drv_string $drv_string]

		set comma ""
		foreach periph $drv_string {
			puts $config_file $comma
			puts $config_file "\t\{"
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "DEVICE_ID,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "BASEADDR,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "NUM_OF_VSMS,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "CLEARING_BITSTREAM,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "CP_ARBITRATION_PROTOCOL,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "HAS_AXI_LITE_IF,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "RESET_ACTIVE_LEVEL,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "CP_FIFO_DEPTH,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "CP_FIFO_TYPE,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "CP_FAMILY,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "CDC_STAGES,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "CP_COMPRESSION,"]

			puts -nonewline $config_file "\t\t{"
			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				puts  -nonewline $config_file [format "XPAR_%s_%s_%s" [string toupper $periphs] "NUM_RMS" [string toupper $vs_name]]
				for {set id $vs_id } { $id < ($num_vs-1)} { incr id } {
				puts $config_file ", "
				puts -nonewline $config_file "\t\t\t"
				}

			}
			puts $config_file "},"

			puts -nonewline $config_file "\t\t{"
			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				puts -nonewline $config_file [format "XPAR_%s_%s_%s" [string toupper $periphs] "NUM_RMS_ALLOC" [string toupper $vs_name]]
				for {set id $vs_id } { $id < ($num_vs-1)} { incr id } {
				puts $config_file ", "
				puts -nonewline $config_file "\t\t\t"
				}
			}
			puts $config_file "},"

			puts -nonewline $config_file "\t\t{"
			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				puts -nonewline $config_file [format "XPAR_%s_%s_%s" [string toupper $periphs] "STRT_IN_SHTDOWN" [string toupper $vs_name]]
				for {set id $vs_id } { $id < ($num_vs-1)} { incr id } {
				puts $config_file ", "
				puts -nonewline $config_file "\t\t\t"
				}
			}
			puts $config_file "},"

			puts -nonewline $config_file "\t\t{"
			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				puts -nonewline $config_file [format "XPAR_%s_%s_%s" [string toupper $periphs] "NUM_TRGRS_ALLOC" [string toupper $vs_name]]
				for {set id $vs_id } { $id < ($num_vs-1)} { incr id } {
				puts $config_file ", "
				puts -nonewline $config_file "\t\t\t"
				}
			}
			puts $config_file "},"

			puts -nonewline $config_file "\t\t{"
			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				puts -nonewline $config_file [format "XPAR_%s_%s_%s" [string toupper $periphs] "SHUTDOWN_ON_ERR" [string toupper $vs_name]]
				for {set id $vs_id } { $id < ($num_vs-1)} { incr id } {
				puts $config_file ", "
				puts -nonewline $config_file "\t\t\t"
				}
			}
			puts $config_file "},"

			puts -nonewline $config_file "\t\t{"
			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				puts -nonewline $config_file [format "XPAR_%s_%s_%s" [string toupper $periphs] "HAS_POR_RM" [string toupper $vs_name]]
				for {set id $vs_id } { $id < ($num_vs-1)} { incr id } {
				puts $config_file ", "
				puts -nonewline $config_file "\t\t\t"
				}
			}
			puts $config_file "},"

			puts -nonewline $config_file "\t\t{"
			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				puts -nonewline $config_file [format "XPAR_%s_%s_%s" [string toupper $periphs] "POR_RM" [string toupper $vs_name]]
				for {set id $vs_id } { $id < ($num_vs-1)} { incr id } {
				puts $config_file ", "
				puts -nonewline $config_file "\t\t\t"
				}
			}
			puts $config_file "},"

			puts -nonewline $config_file "\t\t{"
			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				puts -nonewline $config_file [format "XPAR_%s_%s_%s" [string toupper $periphs] "HAS_AXIS_STATUS" [string toupper $vs_name]]
				for {set id $vs_id } { $id < ($num_vs-1)} { incr id } {
				puts $config_file ", "
				puts -nonewline $config_file "\t\t\t"
				}
			}
			puts $config_file "},"

			puts -nonewline $config_file "\t\t{"
			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				puts -nonewline $config_file [format "XPAR_%s_%s_%s" [string toupper $periphs] "HAS_AXIS_CONTROL" [string toupper $vs_name]]
				for {set id $vs_id } { $id < ($num_vs-1)} { incr id } {
				puts $config_file ", "
				puts -nonewline $config_file "\t\t\t"
				}
			}
			puts $config_file "},"

			puts -nonewline $config_file "\t\t{"
			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				puts -nonewline $config_file [format "XPAR_%s_%s_%s" [string toupper $periphs] "SKIP_RM_STARTUP_AFTER_RESET" [string toupper $vs_name]]
				for {set id $vs_id } { $id < ($num_vs-1)} { incr id } {
				puts $config_file ", "
				puts -nonewline $config_file "\t\t\t"
				}
			}
			puts $config_file "},"

			puts -nonewline $config_file "\t\t{"
			for {set vs_id 0} {$vs_id < $num_vs} { incr vs_id} {
				set vs_name		[prc_v1_2::priv::get_vs_name	configuration $vs_id]
				puts -nonewline $config_file [format "XPAR_%s_%s_%s" [string toupper $periphs] "NUM_HW_TRIGGERS" [string toupper $vs_name]]
				for {set id $vs_id } { $id < ($num_vs-1)} { incr id } {
				puts $config_file ", "
				puts -nonewline $config_file "\t\t\t"
				}
			}
			puts $config_file "},"

			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "VSM_SELECT_MSB,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "VSM_SELECT_LSB,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "TABLE_SELECT_MSB,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "TABLE_SELECT_LSB,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "REG_SELECT_MSB,"]
			puts $config_file [format "\t\tXPAR_%s_%s" [string toupper $periphs] "REG_SELECT_LSB,"]
			puts $config_file "\t\}"
		}
		puts $config_file "\n\};"
		close $config_file
	}
}

proc generate {drv_handle} {
	prc_generate_params $drv_handle "xparameters.h"

	prc_generate_canonical $drv_handle "xparameters.h"

	prc_generate_config $drv_handle "xprc_g.c"
}