################################################################################
#
# Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and#or sell
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
################################################################################

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XHdcp22_Rx" \
	"NUM_INSTANCES" \
	"DEVICE_ID" \
	"C_BASEADDR" \
	"C_HIGHADDR" \
	"C_PROTOCOL" \
	"C_MODE"

  hier_ip_define_config_file $drv_handle "xhdcp22_rx_g.c" "XHdcp22_Rx" \
	"DEVICE_ID" \
	"C_BASEADDR" \
	"C_PROTOCOL" \
	"C_MODE"

  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "XHdcp22_Rx" \
	"NUM_INSTANCES" \
	"DEVICE_ID" \
	"C_BASEADDR" \
	"C_HIGHADDR"
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
	puts $config_file "* The configuration table for devices"
	puts $config_file "*/\n"
	puts $config_file [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
	puts $config_file "\{"

	set periphs_g [::hsi::utils::get_common_driver_ips $drv_handle]

	array set sub_core_inst {
		hdcp22_cipher 1
		hdcp22_mmult 1
		hdcp22_rng 1
		axi_timer 1
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

			#puts "IP type $ip_type_g\n"
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
					# create dictionary for ip name and it's instance names "ip_name {inst1_name inst2_name}"
					dict lappend ss_ip_list $ip_name $child_cell_name_g
				}
			}
		}
	}

	set start_comma ""

	# Loop through each hdcp22_rx subsystem
	foreach periph $periphs_g {
		puts $config_file [format "%s\t\{" $start_comma]
		set comma ""

		# Loop through each parameter argument and update the config structure
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

		puts -nonewline $config_file ",\n"
		::hsi::current_hw_instance  $periph
		set child_cells [::hsi::get_cells -hier]
		set comma ""
		foreach sub_core [lsort [array names sub_core_inst]] {
			if {[dict exists $ss_ip_list $sub_core]} {
				#puts "****Sub-core found in dictionary****"
				set ip_instances [dict get $ss_ip_list $sub_core]
				set idx [lsearch $ip_instances ${periph}*]
				set ip_inst_name [lindex $ip_instances $idx]
				#puts "IP Inst. Name: $ip_inst_name"
				set final_child_cell_instance_devid "XPAR_${ip_inst_name}_DEVICE_ID"
				puts -nonewline $config_file [format "%s\t\t%s" $comma [string toupper $final_child_cell_instance_devid]]
				set comma ",\n"
			}
		}
		::hsi::current_hw_instance

		puts $config_file "\n";
		puts $config_file "\t\}"
		set start_comma ",\n"
    }

    puts $config_file "\n\};"
    puts $config_file "\n";
    close $config_file
}
