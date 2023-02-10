###############################################################################
# Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 1.0      dc     03/08/21 Initial Version.
#          dc     04/21/21 Update due to restructured registers
# 1.5      dc     01/02/23 Multiband registers update
#
###############################################################################

proc generate {drv_handle} {
	prach_define_include_file $drv_handle "xparameters.h" "XDfePrach" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_NUM_ANTENNA0" "C_NUM_ANTENNA1" "C_NUM_ANTENNA2" "C_NUM_CC_PER_ANTENNA0" "C_NUM_CC_PER_ANTENNA1" "C_NUM_CC_PER_ANTENNA2" "C_NUM_SLOT_CHANNELS0" "C_NUM_SLOT_CHANNELS1" "C_NUM_SLOT_CHANNELS2" "C_NUM_SLOTS0" "C_NUM_SLOTS1" "C_NUM_SLOTS2" "C_NUM_RACH_LANES" "C_NUM_RACH_CHANNELS" "C_HAS_AXIS_CTRL" "C_HAS_IRQ" "C_NUM_BANDS"
	generate_config_file $drv_handle "xdfeprach_g.c" "XDfePrach" "DEVICE_ID" "C_BASEADDR" "C_NUM_ANTENNA0" "C_NUM_ANTENNA1" "C_NUM_ANTENNA2" "C_NUM_CC_PER_ANTENNA0" "C_NUM_CC_PER_ANTENNA1" "C_NUM_CC_PER_ANTENNA2" "C_NUM_SLOT_CHANNELS0"  "C_NUM_SLOT_CHANNELS1" "C_NUM_SLOT_CHANNELS2" "C_NUM_SLOTS0" "C_NUM_SLOTS1" "C_NUM_SLOTS2" "C_NUM_RACH_LANES" "C_NUM_RACH_CHANNELS" "C_HAS_AXIS_CTRL" "C_HAS_IRQ" "C_NUM_BANDS"
	prach_define_canonical_xpars $drv_handle "xparameters.h" "XDfePrach" "DEVICE_ID" "C_BASEADDR" "C_NUM_ANTENNA0" "C_NUM_ANTENNA1" "C_NUM_ANTENNA2" "C_NUM_CC_PER_ANTENNA0" "C_NUM_CC_PER_ANTENNA1" "C_NUM_CC_PER_ANTENNA2" "C_NUM_SLOT_CHANNELS0" "C_NUM_SLOT_CHANNELS1" "C_NUM_SLOT_CHANNELS2" "C_NUM_SLOTS0" "C_NUM_SLOTS1" "C_NUM_SLOTS2" "C_NUM_RACH_LANES" "C_NUM_RACH_CHANNELS" "C_HAS_AXIS_CTRL" "C_HAS_IRQ" "C_NUM_BANDS"
}
proc generate_config_file {drv_handle file_name drv_string args} {
	set args [::hsi::utils::get_exact_arg_list $args]
	set filename [file join "src" $file_name]
	set config_file [open $filename w]
	::hsi::utils::write_c_header $config_file "Driver configuration"
	puts $config_file "#include \"xparameters.h\""
	puts $config_file "#include \"[string tolower $drv_string].h\""
	puts $config_file "\n/*"
	puts $config_file "* The configuration table for devices"
	puts $config_file "*/\n"
	set num_insts [::hsi::utils::get_driver_param_name $drv_string "NUM_INSTANCES"]
	puts $config_file [format "%s_Config %s_ConfigTable\[%s\] =" $drv_string $drv_string $num_insts]
	puts $config_file "\{"
	set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
	set start_comma ""
	set ofs 0
	foreach periph $periphs {
		puts $config_file [format "%s\t\{" $start_comma]
		set comma ""
		set len [llength $args]
		foreach arg $args {
			if {$ofs > 0} {
				incr ofs -1
				continue
			}
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
					if {[string compare -nocase "C_NUM_ANTENNA0" $arg] == 0} {
						set pos [lsearch $args $arg]
						set ofs 2
						set list [lrange $args $pos $pos+$ofs]
						puts $config_file ",\n\t\t{"
						foreach arg $list {
							puts -nonewline $config_file [format "\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
						}
						puts -nonewline $config_file "\t\t}"
					} elseif {[string compare -nocase "C_NUM_CC_PER_ANTENNA0" $arg] == 0} {
						set pos [lsearch $args $arg]
						set ofs 2
						set list [lrange $args $pos $pos+$ofs]
						puts $config_file ",\n\t\t{"
						foreach arg $list {
							puts -nonewline $config_file [format "\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
						}
						puts -nonewline $config_file "\t\t}"
					} elseif {[string compare -nocase "C_NUM_SLOT_CHANNELS0" $arg] == 0} {
						set pos [lsearch $args $arg]
						set ofs 2
						set list [lrange $args $pos $pos+$ofs]
						puts $config_file ",\n\t\t{"
						foreach arg $list {
							puts -nonewline $config_file [format "\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
						}
						puts -nonewline $config_file "\t\t}"
					} elseif {[string compare -nocase "C_NUM_SLOTS0" $arg] == 0} {
						set pos [lsearch $args $arg]
						set ofs 2
						set list [lrange $args $pos $pos+$ofs]
						puts $config_file ",\n\t\t{"
						foreach arg $list {
							puts -nonewline $config_file [format "\t\t\t%s,\n" [::hsi::utils::get_ip_param_name $periph $arg]]
						}
						puts -nonewline $config_file "\t\t}"
					} else {
						puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
					}
				}
			} else {
				puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_driver_param_name $drv_string $arg]]
			}
			set comma ",\n"
			set pos [lsearch $args $arg]
			set args1 [lrange $args $pos+1 $len-1]
			if {$args1 == ""} {break}
		}
		puts -nonewline $config_file "\n\t\}"
		set start_comma ",\n"
	}
	puts $config_file "\n\};"

	puts $config_file "\n";

	close $config_file
}
proc prach_define_include_file {drv_handle file_name drv_string args} {
	set args [::hsi::utils::get_exact_arg_list $args]
	set uSuffix "U"
	set hexPrefix ""
	# Open include file
	set file_handle [::hsi::utils::open_include_file $file_name]

	# Get all peripherals connected to this driver
	set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

	# Handle special cases
	set arg "NUM_INSTANCES"
	set posn [lsearch -exact $args $arg]
	if {$posn > -1} {
		puts $file_handle "\n/* Definitions for driver [string toupper [common::get_property name $drv_handle]] */"
		# Define NUM_INSTANCES
		puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [llength $periphs]$uSuffix"
		set args [lreplace $args $posn $posn]
	}

	# Check if it is a driver parameter
	lappend newargs
	foreach arg $args {
		set value [common::get_property CONFIG.$arg $drv_handle]
		if {[llength $value] == 0} {
			lappend newargs $arg
		} else {
			puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property $arg $drv_handle]$uSuffix"
		}
	}
	set args $newargs

	# Print all parameters for all peripherals
	set device_id 0
	foreach periph $periphs {
		puts $file_handle ""
		puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
		foreach arg $args {
			if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
				set value $device_id
				incr device_id
			} elseif {[string compare -nocase "C_BASEADDR" $arg] == 0} {
				set value [common::get_property CONFIG.$arg $periph]
				if {[llength $value] == 0} {
					set value [common::get_property CONFIG.C_BASEADDR $periph]
				}
			} else {
				set value [common::get_property CONFIG.$arg $periph]
			}

			if {[llength $value] == 0} {
				set value 0
			}
			set value [::hsi::utils::format_addr_string $value $arg]
			if {[string compare -nocase "HW_VER" $arg] == 0} {
				puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] \"$value\""
			} else {
				puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $hexPrefix$value$uSuffix"
			}
		}
		puts $file_handle ""
	}
	puts $file_handle "\n/******************************************************************/\n"
	close $file_handle
}

proc prach_define_canonical_xpars {drv_handle file_name drv_string args} {
	set hexPrefix ""
	set args [::hsi::utils::get_exact_arg_list $args]
	# Open include file
	set file_handle [::hsi::utils::open_include_file $file_name]

	# Get all the peripherals connected to this driver
	set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

	# Get the names of all the peripherals connected to this driver
	foreach periph $periphs {
		set peripheral_name [string toupper [common::get_property NAME $periph]]
		lappend peripherals $peripheral_name
	}

	# Get possible canonical names for all the peripherals connected to this
	# driver
	set device_id 0
	foreach periph $periphs {
		set canonical_name [string toupper [format "%s_%s" $drv_string $device_id]]
		lappend canonicals $canonical_name

		# Create a list of IDs of the peripherals whose hardware instance name
		# doesn't match the canonical name. These IDs can be used later to
		# generate canonical definitions
		if { [lsearch $peripherals $canonical_name] < 0 } {
			lappend indices $device_id
		}
		incr device_id
	}

	set i 0
	foreach periph $periphs {
		set periph_name [string toupper [common::get_property NAME $periph]]

		# Generate canonical definitions only for the peripherals whose
		# canonical name is not the same as hardware instance name
		if { [lsearch $canonicals $periph_name] < 0 } {
			puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
			set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

			foreach arg $args {
				set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]

				# The commented out rvalue is the name of the instance-specific constant
				# set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
				# The rvalue set below is the actual value of the parameter

				if {[string compare -nocase "C_BASEADDR" $arg] == 0} {
					set rvalue [::hsi::utils::get_param_value $periph $arg]
					if {[llength $rvalue] == 0} {
						set rvalue [common::get_property CONFIG.C_BASEADDR $periph]
					}
				} else {
					set rvalue [::hsi::utils::get_param_value $periph $arg]
				}

				if {[llength $rvalue] == 0} {
					set rvalue 0
				}
				set rvalue [::hsi::utils::format_addr_string $rvalue $arg]
				set uSuffix [xdefine_getSuffix $lvalue $rvalue]
				puts $file_handle "#define $lvalue $hexPrefix$rvalue$uSuffix"

			}
			puts $file_handle ""
			incr i
		}

		puts $file_handle "\n/******************************************************************/\n"

		puts $file_handle "/* Xilinx PRACH Device Name */"
		set lvalue [::hsi::utils::get_driver_param_name $canonical_name DEV_NAME]
		set rvalue [::hsi::utils::get_param_value $periph C_BASEADDR]
		regsub -all {^0x} $rvalue {} rvalue
		set ipname [get_property IP_NAME [get_cells -hier $drv_handle]]
		puts $file_handle "#define $lvalue \"[string tolower $rvalue].$ipname\""

		puts $file_handle "\n/******************************************************************/\n"
	}
	close $file_handle
}

proc xdefine_getSuffix {arg_name value} {
	set uSuffix ""
	if { [string match "*DEVICE_ID" $value] == 0} {
		set uSuffix "U"
	}
	return $uSuffix
}
