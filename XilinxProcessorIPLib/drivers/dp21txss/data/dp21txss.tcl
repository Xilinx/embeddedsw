##############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
# Ver  Who Date     Changes
# ---- --- -------- -----------------------------------------------------------
# 2.00 sha 08/07/15 Added HDCP support to work with DP pass-through.
# 2.00 sha 09/28/15 Added Timer Counter support to work with DP pass-through.
# 3.0  sha 02/05/16 Added support to generate XPAR_* parameters for multiple
#                   subsystems in a design.
###############################################################################

proc generate {drv_handle} {
	::hsi::utils::define_include_file $drv_handle "xparameters.h" "XDpTxSs" "NUM_INSTANCES" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID" "AUDIO_ENABLE" "BITS_PER_COLOR" "HDCP_ENABLE" "HDCP22_ENABLE" "LANE_COUNT" "MODE" "NUM_STREAMS" "SIM_MODE" "LINK_RATE"
	hier_ip_define_config_file $drv_handle "xdptxss_g.c" "XDpTxSs" "DEVICE_ID" "C_BASEADDR" "AUDIO_ENABLE" "BITS_PER_COLOR" "HDCP_ENABLE" "HDCP22_ENABLE" "LANE_COUNT" "MODE" "NUM_STREAMS"
	::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "DpTxSs" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID" "AUDIO_ENABLE" "BITS_PER_COLOR" "HDCP_ENABLE" "HDCP22_ENABLE" "LANE_COUNT" "MODE" "NUM_STREAMS" "SIM_MODE" "LINK_RATE"
}

# This procedure creates parameters XPAR_* of each sub-cores in xdptxss_g.c file.
# to align with subsystem config structure. Basically, subsystem is hierarchical IP (HIP),
# includes configuration parameters of each sub-cores along with its configuration
# parameters.
proc hier_ip_define_config_file {drv_handle file_name drv_string args} {
	set args [::hsi::utils::get_exact_arg_list $args]

	set brace 0

	array set sub_core_params {}
	set sub_core_params(dptx) "BASEADDR S_AXI_ACLK LANE_COUNT LINK_RATE MAX_BITS_PER_COLOR QUAD_PIXEL_ENABLE DUAL_PIXEL_ENABLE YCRCB_ENABLE YONLY_ENABLE GT_DATAWIDTH SECONDARY_SUPPORT AUDIO_CHANNELS MST_ENABLE NUMBER_OF_MST_STREAMS PROTOCOL_SELECTION FLOW_DIRECTION"
	set sub_core_params(v_tc) "BASEADDR"
	set sub_core_params(v_dual_splitter) "BASEADDR"
	set sub_core_params(hdcp) "BASEADDR"
	set sub_core_params(axi_timer) "BASEADDR"
	set sub_core_params(hdcp22_tx_dp) "BASEADDR"
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

	array set sub_core_inst {
		dptx 1
		v_tc 1
		v_dual_splitter 1
		hdcp 1
		axi_timer 1
		hdcp22_tx_dp 1
	}

	foreach periph_g $periphs_g {
		::hsi::current_hw_instance $periph_g;

		set child_cells_g [::hsi::get_cells]

		puts $config_file "\n/*"
		puts $config_file "* List of Sub-cores included from the subsystem"
		puts $config_file "*/"
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

					# create dictionary for ip name and it's instance names "ip_name {inst1_name inst2_name}"
					dict lappend ss_ip_list $ip_name $child_cell_name_g
				}
			}
		}

		puts $config_file "\n/*"
		puts $config_file "* List of Sub-cores excluded from the subsystem"
		puts $config_file "*/"

		foreach sub_core [lsort [array names sub_core_inst]] {
			if {[dict exists $ss_ip_list $sub_core]} {
				set max_instances $sub_core_inst($sub_core)
				#check if core can have multiple instances
				#It is possible that not all instances are used in the design
				if {$max_instances > 1} {
					set ip_instances [dict get $ss_ip_list $sub_core]
					set avail_instances [llength $ip_instances]

					#check if available instances are less than MAX
					#if yes, mark the missing instance
					#if all instances are present then skip the core
					if {$avail_instances < $max_instances} {
						set final_child_cell_instance_name_g "XPAR_${periph_g}_${strval}_PRESENT"
						puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_g] 0\n"
					}
				}
			} else {
				set count 0
				while {$count<$sub_core_inst($sub_core)} {
					set final_child_cell_instance_name_g "XPAR_${periph_g}_${sub_core}_${count}_PRESENT"
					puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_g] 0\n"
					incr count
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
		set child_cells_g [::hsi::get_cells]
		puts $config_file ",\n"

		#This is to get the dictionary of included subcores.
		set ss_ip_list [dict create]
		foreach child_cell_g $child_cells_g {
			set child_cell_vlnv [::common::get_property VLNV $child_cell_g]
			set child_cell_name_g [common::get_property NAME $child_cell_g]
			set vlnv_arr [split $child_cell_vlnv :]
			lassign $vlnv_arr ip_vendor ip_library ip_name ip_version
			set ip_type_g [common::get_property IP_TYPE $child_cell_g]

			if {[string compare -nocase "BUS" $ip_type_g] != 0} {
				set interfaces [hsi::get_intf_pins -of_objects $child_cell_g]
				set is_slave 0
				foreach interface $interfaces {
					set intf_type [common::get_property TYPE $interface]
					if { [string compare -nocase "SLAVE" $intf_type] == 0 } {
						set is_slave 1
					}
				}
				if {$is_slave != 0} {
					# create dictionary for ip name and it's instance names "ip_name {inst1_name inst2_name}"
					dict lappend ss_ip_list $ip_name $child_cell_name_g
				}
			}
		}

		#Check for each subcore if it is included or excluded
		foreach sub_core [lsort [array names sub_core_inst]] {
			set max_instances $sub_core_inst($sub_core)

			if {[dict exists $ss_ip_list $sub_core]} {
				#subcore include
				set ip_instances [dict get $ss_ip_list $sub_core]
				set ip_inst_name [lindex $ip_instances 0]
				set final_child_cell_instance_name_present "XPAR_${ip_inst_name}_PRESENT"
				set final_child_cell_instance_name "XPAR_${ip_inst_name}_DEVICE_ID"

				#puts $config_file [format "JB:%s, ::%s" $ip_inst_name, $sub_core]

				if {$sub_core == "v_tc"} {
					set avail_instances [llength $ip_instances]
					set count 0
					puts $config_file "\t\t\{"
					while {$count < $avail_instances} {
						set ip_inst_name [lindex $ip_instances $count]
						set final_child_cell_instance_name_present "XPAR_${ip_inst_name}_PRESENT"
						set final_child_cell_instance_name "XPAR_${ip_inst_name}_DEVICE_ID"
						puts $config_file "\t\t\t\{"
						puts -nonewline $config_file [format "\t\t\t\t%s" [string toupper $final_child_cell_instance_name_present]]
						puts $config_file ","
						puts $config_file "\t\t\t\t\{"
						puts -nonewline $config_file [format "\t\t\t\t\t%s" [string toupper $final_child_cell_instance_name]]

						set params_str $sub_core_params($sub_core)
						set params_arr [split $params_str " " ]

						foreach param $params_arr {
							set final_child_cell_param_name XPAR_${ip_inst_name}_$param
							puts $config_file ","
							puts -nonewline $config_file [format "\t\t\t\t\t%s" [string toupper $final_child_cell_param_name]]
						}

						puts $config_file "\n\t\t\t\t\}"
						puts $config_file "\t\t\t\},"
						incr count
					}
					puts $config_file "\t\t\}"
				} else {
					set comma ",\n"
					puts $config_file "\t\t\{"
					puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_name_present]]
					puts $config_file ","
					puts $config_file "\t\t\t\{"
					puts -nonewline $config_file [format "\t\t\t\t%s" [string toupper $final_child_cell_instance_name]]

					set params_str $sub_core_params($sub_core)
					set params_arr [split $params_str " " ]

					foreach param $params_arr {
						set final_child_cell_param_name XPAR_${ip_inst_name}_$param
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
			} else {
				#subcore excluded
				set count 0
				set comma ",\n"
				while {$count<$max_instances} {
					set final_child_cell_instance_name_present "XPAR_${periph}_${sub_core}_${count}_PRESENT"
					puts $config_file "\t\t\{"
					puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_name_present]]
					puts $config_file ","
					puts $config_file "\t\t\t\{"
					puts -nonewline $config_file "\t\t\t\t0"
					puts $config_file "\n\t\t\t\}"
					if { $brace < $total_subcores - 1 } {
						puts $config_file "\t\t\},"
						incr brace
					} else {
						puts $config_file "\t\t\}"
					}
					incr count
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
