##############################################################################
#
# Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
# Ver Who Date     Changes
# --- --- -------- -----------------------------------------------------------
# 1.0 sss 07/21/16 Initial version of subsystem tcl
# 1.1 vsa 08/31/17 Fix for IP with different name
# 1.2 vsa 02/28/18 Add Frame End Generation Feature
###############################################################################

proc generate {drv_handle} {
	::hsi::utils::define_include_file $drv_handle "xparameters.h" "XCsi2TxSs" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_CSI_LANES" "C_CSI_DATATYPE" "C_CSI_PIXEL_MODE" "C_CSI_LINE_BUFR_DEPTH" "C_DPHY_LINERATE" "C_DPHY_EN_REG_IF" "C_EN_REG_BASED_FE_GEN"
	hier_ip_define_config_file $drv_handle "xcsi2txss_g.c" "XCsi2TxSs" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_CSI_LANES" "C_CSI_DATATYPE" "C_CSI_PIXEL_MODE" "C_CSI_LINE_BUFR_DEPTH" "C_DPHY_LINERATE" "C_DPHY_EN_REG_IF" "C_EN_REG_BASED_FE_GEN"
	::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Csi2TxSs" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_CSI_LANES" "C_CSI_DATATYPE" "C_CSI_PIXEL_MODE" "C_CSI_LINE_BUFR_DEPTH" "C_DPHY_LINERATE" "C_DPHY_EN_REG_IF" "C_EN_REG_BASED_FE_GEN"

	set orig_dir [pwd]
	cd ../../include/

	set periphs [hsi::utils::get_common_driver_ips $drv_handle]
	foreach periph $periphs {
		set timestamp [clock format [clock seconds] -format {%Y%m%d%H%M%S}]

		set filename "xparameters.h"
		set temp $filename.new.$timestamp

		set in [open $filename r]
		set out [open $temp w]

		# line-by-line, read the original file
		while {[gets $in line] != -1} {
			# if the peripheral name or the canonical substring is present
			if {[regexp -nocase XPAR_$periph $line] ||
				[regexp -nocase {XPAR_CSI2TXSS} $line]} {

				# if substring DPY_EN_REG_IF is present in the string
				if {[regexp -nocase {DPHY_EN_REG_IF} $line]} {
					# using string map to replace true with 1 and false with 0
					set line [string map {true 1 false 0} $line]
				}

				# if substring CSI_DATATYPE is present in the string
				if {[regexp -nocase {CSI_DATATYPE} $line]} {
					# using string map to replace true with 1 and false with 0
					set line [string map {RGB444 0x20 RGB555 0x21 RGB565 0x22 RGB666 0x23 RGB888 0x24 RAW6 0x28 RAW7 0x29 RAW8 0x2A RAW10 0x2B RAW12 0x2C RAW14 0x2D YUV422_8bit 0x1E} $line]
				}

				# if substring C_CSI_EN_ACTIVELANES is present in the string
				if {[regexp -nocase {CSI_EN_ACTIVELANES} $line]} {
					# using string map to replace true with 1 and false with 0
					set line [string map {true 1 false 0} $line]
				}

				# if substring C_EN_REG_BASED_FE_GEN is present in the string
				if {[regexp -nocase {EN_REG_BASED_FE_GEN} $line]} {
					# using string map to replace true with 1 and false with 0
					set line [string map {true 1 false 0} $line]
				}
			}

			# then write the transformed line
			puts $out $line
		}

		close $in
		close $out

		# move the new data to the proper filename
		file delete $filename
		file rename -force $temp $filename
	}
	cd $orig_dir
}

# This procedure generates parameters XPAR_* of each sub-cores in xcsi2txss_g.c file.
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
		mipi_csi2_tx_ctrl 1
		mipi_dphy 1
	}

	foreach periph_g $periphs_g {
		set mem_ranges [::hsi::get_mem_ranges $periph_g]

		::hsi::current_hw_instance $periph_g;

		set child_cells_g [::hsi::get_cells]

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
					if {[string compare -nocase "SLAVE" $intf_type] == 0} {
						set is_slave 1
					}
				}
				if {$is_slave != 0} {
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

					#check if available instances are less than MAX
					#if yes, mark the missing instance
					#if all instances are present then skip the core
					if {$avail_instances < $max_instances} {
						set final_child_cell_instance_name_g "XPAR_${periph_g}_${strval}_PRESENT"
						set final_child_cell_instance_devid_g "XPAR_${periph_g}_${strval}_DEVICE_ID"
						set final_child_cell_instance_baseaddr_g "XPAR_${periph_g}_${strval}_BASEADDR"
						puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_g] 0\n"
						puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_devid_g] 255\n"
						puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_baseaddr_g] 0\n\n"
					}
				}
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
				set local_value [common::get_property CONFIG.$arg $periph]
				# If a parameter isn't found locally (in the current
				# peripheral), we will (for some obscure and ancient reason)
				# look in peripherals connected via point to point links
				if {[string compare -nocase $local_value ""] == 0} {
					set p2p_name [::hsi::utils::get_p2p_name $periph $arg]
					if {[string compare -nocase $p2p_name ""] == 0} {
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

		::hsi::current_hw_instance $periph
		set child_cells [::hsi::get_cells]
		puts $config_file ",\n"

		foreach sub_core [lsort [array names sub_core_inst]] {
			set max_instances $sub_core_inst($sub_core)

			if {[dict exists $ss_ip_list $sub_core]} {
				set base_addr_name "BASEADDR"

				set ip_instances [dict get $ss_ip_list $sub_core]
				set avail_instances [llength $ip_instances]

				if {$max_instances > 1} {
					if {$avail_instances < $max_instances} {
						set ip_inst_name [lindex $ip_instances 0]
						set count 0
						set str_name "unknown"
						while {$count < $max_instances} {
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
