##############################################################################
#
# Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
# MODIFICATION HISTORY:
# Ver  Who Date     Changes
# ---- --- -------- -----------------------------------------------------------
# 2.00 sha 08/07/15 Added HDCP support to work with DP pass-through.
# 2.00 sha 09/28/15 Added Timer Counter support to work with DP pass-through.
# 3.0  sha 02/05/16 Added support to generate XPAR_* parameters for multiple
#                   subsystems in a design.
###############################################################################

proc generate {drv_handle} {
	::hsi::utils::define_include_file $drv_handle "xparameters.h" "XDpTxSs" "NUM_INSTANCES" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID" "AUDIO_ENABLE" "BITS_PER_COLOR" "HDCP_ENABLE" "HDCP22_ENABLE" "LANE_COUNT" "MODE" "NUM_STREAMS" "SIM_MODE"
	hier_ip_define_config_file $drv_handle "xdptxss_g.c" "XDpTxSs" "DEVICE_ID" "C_BASEADDR" "AUDIO_ENABLE" "BITS_PER_COLOR" "HDCP_ENABLE" "HDCP22_ENABLE" "LANE_COUNT" "MODE" "NUM_STREAMS"
	::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "DpTxSs" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID" "AUDIO_ENABLE" "BITS_PER_COLOR" "HDCP_ENABLE" "HDCP22_ENABLE" "LANE_COUNT" "MODE" "NUM_STREAMS" "SIM_MODE"
}

# This procedure creates parameters XPAR_* of each sub-cores in xdptxss_g.c file.
# to align with subsystem config structure. Basically, subsystem is hierarchical IP (HIP),
# includes configuration parameters of each sub-cores along with its configuration
# parameters.
proc hier_ip_define_config_file {drv_handle file_name drv_string args} {
	set args [::hsi::utils::get_exact_arg_list $args]
	set brace 0
	array set sub_core_params {}

	set sub_core_params(displayport) "BASEADDR S_AXI_ACLK LANE_COUNT LINK_RATE MAX_BITS_PER_COLOR QUAD_PIXEL_ENABLE DUAL_PIXEL_ENABLE YCRCB_ENABLE YONLY_ENABLE GT_DATAWIDTH SECONDARY_SUPPORT AUDIO_CHANNELS MST_ENABLE NUMBER_OF_MST_STREAMS PROTOCOL_SELECTION FLOW_DIRECTION"
	set sub_core_params(v_tc) "BASEADDR"
	set sub_core_params(v_dual_splitter) "BASEADDR ACTIVE_COLS ACTIVE_ROWS MAX_SEGMENTS AXIS_VIDEO_MAX_TDATA_WIDTH AXIS_VIDEO_MAX_ITDATASMPLS_PER_CLK AXIS_VIDEO_MAX_OTDATASMPLS_PER_CLK MAX_OVRLAP MAX_SMPL_WIDTH HAS_AXI4_LITE HAS_IRQ"
	set sub_core_params(hdcp) "BASEADDR S_AXI_FREQUENCY IS_RX IS_HDMI"
	set sub_core_params(axi_timer) "BASEADDR CLOCK_FREQ_HZ"
	set sub_core_params(hdcp22_tx_dp) "BASEADDR"

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

		set child_cells_g [::hsi::get_cells -hier]

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
		set child_cells [::hsi::get_cells -hier]
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
					if {$ip_name == "v_tc"} {
						if {$brace == 1} {
							puts $config_file "\t\t\t\},"
						}

						if { $brace == 0 } {
							puts $config_file "\t\t\{"
							incr brace
						}

						puts $config_file "\t\t\t\{"
						puts -nonewline $config_file [format "\t\t\t\t%s" [string toupper $final_child_cell_instance_name_present]]
						puts $config_file ","
						puts $config_file "\t\t\t\t\{"
						puts -nonewline $config_file [format "\t\t\t\t\t%s" [string toupper $final_child_cell_instance_name]]

						set params_str $sub_core_params($ip_name)
						set params_arr [split $params_str " " ]

						foreach param $params_arr {
								set final_child_cell_param_name XPAR_${child_cell_name}_$param
								puts $config_file ","
								puts -nonewline $config_file [format "\t\t\t\t\t%s" [string toupper $final_child_cell_param_name]]
						}

						puts $config_file "\n\t\t\t\t\}"
					} elseif {($ip_name ne "hdcp22_cipher_dp") && ($ip_name ne "hdcp22_rng")} {
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
						puts $config_file "\t\t\},"
					}
				}
			}
		}

		if {$brace == 1} {
			puts $config_file "\t\t\t\}"
			puts $config_file "\t\t\}"
			set brace 0
		}

		::hsi::current_hw_instance

		puts -nonewline $config_file "\t\}"
		set start_comma ",\n"
	}

	puts $config_file "\n\};"
	puts $config_file "\n";

	close $config_file
}
