##############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
# Ver  Who Date     Changes
# ---- --- -------- -----------------------------------------------------------
# 2.00 sha 10/05/15 Added Timer Counter support.
# 3.0  sha 02/05/16 Added support to generate XPAR_* parameters for multiple
#                   subsystems in a design.
# 4.0  tu  27/06/17 Updated parameter AUDIO_CHANNELS name
###############################################################################

proc generate {drv_handle} {
	::hsi::utils::define_include_file $drv_handle "xparameters.h" "XDpRxSs" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "AUDIO_ENABLE" "AUDIO_CHANNELS" "BITS_PER_COLOR" "HDCP_ENABLE" "LANE_COUNT" "MODE" "NUM_STREAMS" "COLOR_FORMAT" "SIM_MODE"
	hier_ip_define_config_file $drv_handle "xdprxss_g.c" "XDpRxSs" "DEVICE_ID" "C_BASEADDR" "AUDIO_ENABLE" "AUDIO_CHANNELS" "BITS_PER_COLOR" "HDCP_ENABLE" "LANE_COUNT" "MODE" "NUM_STREAMS" "COLOR_FORMAT"
	::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "DpRxSs" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "AUDIO_ENABLE" "AUDIO_CHANNELS" "BITS_PER_COLOR" "HDCP_ENABLE" "LANE_COUNT" "MODE" "NUM_STREAMS" "COLOR_FORMAT" "SIM_MODE"
}

##
# This procedure creates parameters XPAR_* of each sub-cores in xdprxss_g.c file.
# to align with subsystem config structure. Basically, subsystem is hierarchical IP (HIP),
# includes configuration parameters of each sub-cores along with its configuration
# parameters.
##
proc hier_ip_define_config_file {drv_handle file_name drv_string args} {
	set args [::hsi::utils::get_exact_arg_list $args]

	set brace 0

	array set sub_core_params {}
	set sub_core_params(displayport) "BASEADDR S_AXI_ACLK LANE_COUNT LINK_RATE MAX_BITS_PER_COLOR QUAD_PIXEL_ENABLE DUAL_PIXEL_ENABLE YCRCB_ENABLE YONLY_ENABLE GT_DATAWIDTH SECONDARY_SUPPORT AUDIO_CHANNELS MST_ENABLE NUMBER_OF_MST_STREAMS PROTOCOL_SELECTION FLOW_DIRECTION"
	set sub_core_params(axi_iic) "BASEADDR TEN_BIT_ADR GPO_WIDTH"
	set sub_core_params(hdcp) "BASEADDR S_AXI_FREQUENCY IS_RX IS_HDMI"
	set sub_core_params(axi_timer) "BASEADDR CLOCK_FREQ_HZ"
	set total_subcores [array size sub_core_params]

	set filename [file join "src" $file_name]
	set config_file [open $filename w]
	::hsi::utils::write_c_header $config_file "Driver configuration"
	puts $config_file "#include \"xparameters.h\""
	puts $config_file "#include \"[string tolower $drv_string].h\""
	puts $config_file "\n/*"
	puts $config_file "* The configuration table for devices"
	puts $config_file "*/\n"

	set periphs_g [::hsi::utils::get_common_driver_ips $drv_handle]

	foreach periph_g $periphs_g {
		::hsi::current_hw_instance $periph_g;

		set child_cells_g [::hsi::get_cells]

		foreach child_cell_g $child_cells_g {
			set child_cell_vlnv [::common::get_property VLNV $child_cell_g]
			set vlnv_arr [split $child_cell_vlnv :]
			lassign $vlnv_arr ip_vendor ip_library ip_name ip_version

			set child_cell_name_g [common::get_property NAME $child_cell_g]
			set ip_type_g [common::get_property IP_TYPE $child_cell_g]
			set final_child_cell_instance_name_present_g XPAR_${child_cell_name_g}_PRESENT

			if { [string compare -nocase "BUS" $ip_type_g] != 0 } {
				set interfaces [hsi::get_intf_pins -of_objects $child_cell_g]
				set is_slave 0

				foreach interface $interfaces {
					set intf_type [common::get_property TYPE $interface]

					if { [string compare -nocase "SLAVE" $intf_type] == 0 } {
						set is_slave 1
					}
				}
				if { $is_slave != 0 } {
					puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_present_g]\t1\n"
				}
			}
		}
		puts $config_file "\n"

		::hsi::current_hw_instance
	}

	puts $config_file [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
	puts $config_file "\{"
	set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
	set start_comma ""
	foreach periph $periphs {
		puts $config_file [format "%s\t\{" $start_comma]
		set comma ""
		foreach arg $args {
			if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
				puts -nonewline $config_file [format "%s\t\t%s,\n" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
				continue
			}
			puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
			set comma ",\n"
		}

		::hsi::current_hw_instance $periph
		set child_cells [::hsi::get_cells]
		puts $config_file ",\n"

		foreach child_cell $child_cells {
			set child_cell_vlnv [::common::get_property VLNV $child_cell]
			set vlnv_arr [split $child_cell_vlnv :]

			lassign $vlnv_arr ip_vendor ip_library ip_name ip_version

			set ip_type [common::get_property IP_TYPE $child_cell]
			set child_cell_name [common::get_property NAME $child_cell]
			set final_child_cell_instance_name XPAR_${child_cell_name}_DEVICE_ID
			set final_child_cell_instance_name_present XPAR_${child_cell_name}_PRESENT

			if { [string compare -nocase "BUS" $ip_type] != 0 } {
				set interfaces [hsi::get_intf_pins -of_objects $child_cell]
				set is_slave 0
				foreach interface $interfaces {
					set intf_type [common::get_property TYPE $interface]
					if { [string compare -nocase "SLAVE" $intf_type] == 0 } {
						set is_slave 1
					}
				}

				if { $is_slave != 0 } {
					set comma ",\n"

					puts $config_file "\t\t\{"
					puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_name_present]]
					puts $config_file ","
					puts $config_file "\t\t\t\{"
					puts -nonewline $config_file [format "\t\t\t\t%s" [string toupper $final_child_cell_instance_name]]

					set params_str $sub_core_params($ip_name)
					set params_arr [split $params_str " " ]

					foreach param $params_arr {
						set final_child_cell_param_name XPAR_${child_cell_name}_$param
						puts $config_file ","
						puts -nonewline $config_file [format "\t\t\t\t%s" [string toupper $final_child_cell_param_name]]
					}
					puts $config_file "\n\t\t\t\}"
					if { $brace < $total_subcores - 1 } {
						puts $config_file "\t\t\},"
						incr brace
					} else {
						puts $config_file "\t\t\}"
					}
				}
			}
		}

		::hsi::current_hw_instance

		set brace 0

		puts -nonewline $config_file "\t\}"
		set start_comma ",\n"
	}

	puts $config_file "\n\};"
	puts $config_file "\n";

	close $config_file
}
