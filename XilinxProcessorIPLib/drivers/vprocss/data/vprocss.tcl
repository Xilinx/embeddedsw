##############################################################################
#
# Copyright (C) 2015 Xilinx, Inc. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"),to deal
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
# XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
# MODIFICATION HISTORY:
#  Ver      Who    Date       Changes
# -------- ------ -------- ----------------------------------------------------
#  1.0      rco    07/21/15 Initial version of subsystem tcl
#  1.1      rco    11/20/15 Bug fix for designs with single instance of
#                           vcresampler core
###############################################################################

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XVprocSs" "NUM_INSTANCES" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID"   "C_SCALER_ALGORITHM" "C_TOPOLOGY" "C_SAMPLES_PER_CLK" "C_MAX_DATA_WIDTH" "C_NUM_VIDEO_COMPONENTS" "C_MAX_COLS" "C_MAX_ROWS" "C_H_SCALER_TAPS" "C_V_SCALER_TAPS" "C_H_SCALER_PHASES" "C_V_SCALER_PHASES" "C_CHROMA_ALGORITHM" "C_H_CHROMA_TAPS" "C_V_CHROMA_TAPS"

  hier_ip_define_config_file $drv_handle "xvprocss_g.c" "XVprocSs" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_TOPOLOGY" "C_SAMPLES_PER_CLK" "C_MAX_DATA_WIDTH" "C_NUM_VIDEO_COMPONENTS" "C_MAX_COLS" "C_MAX_ROWS"

  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "XVprocSs" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID"  "C_SCALER_ALGORITHM" "C_TOPOLOGY" "C_SAMPLES_PER_CLK" "C_MAX_DATA_WIDTH" "C_NUM_VIDEO_COMPONENTS" "C_MAX_COLS" "C_MAX_ROWS" "C_H_SCALER_TAPS" "C_V_SCALER_TAPS" "C_H_SCALER_PHASES" "C_V_SCALER_PHASES" "C_CHROMA_ALGORITHM" "C_H_CHROMA_TAPS" "C_V_CHROMA_TAPS"
}


proc hier_ip_define_config_file {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
    set hw_instance_name [::common::get_property HW_INSTANCE $drv_handle];

    set filename [file join "src" $file_name]
    set config_file [open $filename w]

    ::hsi::utils::write_c_header $config_file "Driver configuration"

    puts $config_file "#include \"xparameters.h\""
    puts $config_file "#include \"[string tolower $drv_string].h\""
    puts $config_file "\n/*"
    puts $config_file "* List of Sub-cores included in the subsystem"
    puts $config_file "* Sub-core device id will be set by its driver in xparameters.h"
    puts $config_file "*/\n"

    set periphs_g [::hsi::utils::get_common_driver_ips $drv_handle]

	array set sub_core_inst {
		axi_gpio 2
		v_vscaler 1
		v_hscaler 1
		axi_vdma 1
		v_letterbox 1
		v_csc 1
		v_hcresampler 1
		v_vcresampler 2
		v_deinterlacer 1
		axis_switch 1
	}

    foreach periph_g $periphs_g {
		set mem_ranges [::hsi::get_mem_ranges $periph_g]

		::hsi::current_hw_instance $periph_g;

		set child_cells_g [::hsi::get_cells -hier]

		foreach child_cell_g $child_cells_g {
			set child_cell_vlnv [::common::get_property VLNV $child_cell_g]
			set child_cell_name_g [common::get_property NAME $child_cell_g]
			set vlnv_arr [split $child_cell_vlnv :]

			lassign $vlnv_arr ip_vendor ip_library ip_name ip_version
			set ip_type_g [common::get_property IP_TYPE $child_cell_g]

			if { [string compare -nocase "BUS" $ip_type_g] != 0 } {
				set interfaces [hsi::get_intf_pins -of_objects $child_cell_g]
				set is_slave 0

				foreach interface $interfaces {
					set intf_type [common::get_property TYPE $interface]
					#puts "Interface type $intf_type\n"
					if { [string compare -nocase "SLAVE" $intf_type] == 0 } {
						set is_slave 1
					}
				}
				if { $is_slave != 0 } {
					#puts "Processing Periph: $ip_name  $child_cell_name_g"
					set final_child_cell_instance_name_present_g XPAR_${child_cell_name_g}_PRESENT
					puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_present_g]\t 1\n"

					# create dictionary for ip name and it's instance names "ip_name {inst1_name inst2_name}"
					dict lappend ss_ip_list $ip_name $child_cell_name_g
				}
			}
		}

		puts $config_file "\n\n/*"
		puts $config_file "* List of Sub-cores excluded from the subsystem"
		puts $config_file "*   - Excluded sub-core device id is set to 255"
		puts $config_file "*   - Excluded sub-core baseaddr is set to 0"
		puts $config_file "*/\n"

		foreach sub_core [lsort [array names sub_core_inst]] {
			if {[dict exists $ss_ip_list $sub_core]} {
				set max_instances $sub_core_inst($sub_core)
				#check if core can have multiple instances
				#It is possible that not all instances are used in the design
			    if {$max_instances > 1} {
					set ip_instances [dict get $ss_ip_list $sub_core]
					set avail_instances [llength $ip_instances]

					#puts "Sub-Core: $sub_core"
					#puts "instances: $ip_instances"

					#check if available instances are less than MAX
					#if yes, mark the missing instance
					#if all instances are present then skip the core
					set strval "Unknown"
					set strbaseaddr "BASEADDR"
					if {$avail_instances < $max_instances} {
						if {[string compare -nocase "axi_gpio" $sub_core] == 0} {
							set ip_inst_name [lindex $ip_instances 0]
							set srcstr "${periph_g}_reset_sel_axi_mm"
							if {[string compare -nocase $srcstr $ip_inst_name] == 0} {
								set strval "RESET_SEL_AXIS"
							} else {
								set strval "RESET_SEL_AXI_MM"
							}
						} elseif {[string compare -nocase "v_vcresampler" $sub_core] == 0} {
							#All hls ip base address string is S_AXI_CTRL_BASEADDR"
							#This check is needed only for IP's with multiple optional instances
							set strbaseaddr "S_AXI_CTRL_BASEADDR"
							set ip_inst_name [lindex $ip_instances 0]
							set srcstr "${periph_g}_v_vcresampler_in"
							if {[string compare -nocase $srcstr $ip_inst_name] == 0} {
								set strval "V_VCRESAMPLER_OUT"
							} else {
								set strval "V_VCRESAMPLER_IN"
							}
						}
						#puts "String Selected: $strval"
						set final_child_cell_instance_name_g "XPAR_${periph_g}_${strval}_PRESENT"
						set final_child_cell_instance_devid_g "XPAR_${periph_g}_${strval}_DEVICE_ID"
						set final_child_cell_instance_baseaddr_g "XPAR_${periph_g}_${strval}_${strbaseaddr}"
						puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_g] 0\n"
						puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_devid_g] 255\n"
						puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_baseaddr_g] 0\n\n"
					}

				}
				#puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_g] 1\n"
			} else {
			    set count 0
				while {$count<$sub_core_inst($sub_core)} {
					set final_child_cell_instance_name_g "XPAR_${periph_g}_${sub_core}_${count}_PRESENT"
					set final_child_cell_instance_devid_g "XPAR_${periph_g}_${sub_core}_${count}_DEVICE_ID"
					set final_child_cell_instance_baseaddr_g "XPAR_${periph_g}_${sub_core}_${count}_BASEADDR"
					puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_g] 0\n"
					puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_devid_g] 255\n"
					puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_baseaddr_g] 0\n\n"
					incr count
				}
			}
		}
		::hsi::current_hw_instance
    }


	puts $config_file "\n\n"
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

            # Check if this is a driver parameter or a peripheral parameter
            set value [common::get_property CONFIG.$arg $drv_handle]
            if {[llength $value] == 0} {
                set local_value [common::get_property CONFIG.$arg $periph ]
                # If a parameter isn't found locally (in the current
                # peripheral), we will (for some obscure and ancient reason)
                # look in peripherals connected via point to point links
                if { [string compare -nocase $local_value ""] == 0} {
                    set p2p_name [::hsi::utils::get_p2p_name $periph $arg]
                    if { [string compare -nocase $p2p_name ""] == 0} {
                        puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
                    } else {
                        puts -nonewline $config_file [format "%s\t\t%s" $comma $p2p_name]
                    }
                } else {
                    puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
                }
            } else {
                puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_driver_param_name $drv_string $arg]]
            }
            set comma ",\n"
        }

		::hsi::current_hw_instance  $periph
		set child_cells [::hsi::get_cells -hier]
		puts $config_file ",\n"

		foreach sub_core [lsort [array names sub_core_inst]] {
			set max_instances $sub_core_inst($sub_core)
			#puts "\nProcessing sub-core: $sub_core"
			#puts "Max Instances: $max_instances"

			if {[dict exists $ss_ip_list $sub_core]} {
				#puts "****Sub-core found in dictionary****"
				if {[string match -nocase v_* $sub_core]} {
						set base_addr_name "S_AXI_CTRL_BASEADDR"
					} else {
						set base_addr_name "BASEADDR"
				}

				set ip_instances [dict get $ss_ip_list $sub_core]
				set avail_instances [llength $ip_instances]

				#check if core can have multiple instances
				#It is possible that not all instances are used in the design
			    if {$max_instances > 1} {

					#check if available instances are less than MAX
					#if yes, include the missing instance
					if {$avail_instances < $max_instances} {
						set ip_inst_name [lindex $ip_instances 0]
						set count 0
						set str_name "unknown"
						#puts "IP Inst. Name: $ip_inst_name"
						while {$count < $max_instances} {
							if {[string compare -nocase "axi_gpio" $sub_core] == 0} {
								set str_name [expr {$count == 0 ? "RESET_SEL_AXI_MM" : "RESET_SEL_AXIS"}]
							} elseif {[string compare -nocase "v_vcresampler" $sub_core] == 0} {
								set str_name [expr {$count == 0 ? "V_VCRESAMPLER_IN" : "V_VCRESAMPLER_OUT"}]
							}
							#write the ip instance entry to the table
							set final_child_cell_instance_name_present "XPAR_${periph}_${str_name}_PRESENT"
							set final_child_cell_instance_devid "XPAR_${periph}_${str_name}_DEVICE_ID"
							set final_child_cell_instance_name_baseaddr "XPAR_${periph}_${str_name}_${base_addr_name}"

							puts $config_file "\t\t\{"
							puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_name_present]]
							puts $config_file ","
							puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_devid]]
							puts $config_file ","
							puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_name_baseaddr]]
							puts $config_file "\n\t\t\},"
							incr count
						}
					} else {
						foreach ip_inst $ip_instances {
							#puts "instance = $ip_inst"
							set final_child_cell_instance_name_present "XPAR_${ip_inst}_PRESENT"
							set final_child_cell_instance_devid "XPAR_${ip_inst}_DEVICE_ID"
							set final_child_cell_instance_name_baseaddr "XPAR_${ip_inst}_${base_addr_name}"

							puts $config_file "\t\t\{"
							puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_name_present]]
							puts $config_file ","
							puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_devid]]
							puts $config_file ","
							puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_name_baseaddr]]
							puts $config_file "\n\t\t\},"
						}
					}
				} else {
					set ip_inst_name [lindex $ip_instances 0]
					#puts "instance = $ip_inst"
					set final_child_cell_instance_name_present "XPAR_${ip_inst_name}_PRESENT"
					set final_child_cell_instance_devid "XPAR_${ip_inst_name}_DEVICE_ID"
					set final_child_cell_instance_name_baseaddr "XPAR_${ip_inst_name}_${base_addr_name}"

					puts $config_file "\t\t\{"
					puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_name_present]]
					puts $config_file ","
					puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_devid]]
					puts $config_file ","
					puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_name_baseaddr]]
					puts $config_file "\n\t\t\},"
				}
			} else {
				#puts "****sub-core not in dictionary****"
				set count 0

				while {$count< $max_instances} {
					set final_child_cell_instance_name_present "XPAR_${periph}_${sub_core}_${count}_PRESENT"
					set final_child_cell_instance_devid "XPAR_${periph}_${sub_core}_${count}_DEVICE_ID"
					set final_child_cell_instance_name_baseaddr "XPAR_${periph}_${sub_core}_${count}_BASEADDR"

					puts $config_file "\t\t\{"
					puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_name_present]]
					puts $config_file ","
					puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_devid]]
					puts $config_file ","
					puts -nonewline $config_file [format "\t\t\t%s" [string toupper $final_child_cell_instance_name_baseaddr]]
					puts $config_file "\n\t\t\},"
					incr count
				}
			}
		}

		::hsi::current_hw_instance

		puts -nonewline $config_file "\t\}"
		set start_comma ",\n"
    }

    puts $config_file "\n\};"
    puts $config_file "\n";
    close $config_file
}
