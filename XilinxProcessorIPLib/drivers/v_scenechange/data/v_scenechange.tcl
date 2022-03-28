# ==============================================================
# Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
# ==============================================================

proc generate {drv_handle} {
	xdefine_include_file $drv_handle "xparameters.h" "XV_scenechange" \
		"NUM_INSTANCES" \
		"DEVICE_ID" \
		"C_S_AXI_CTRL_BASEADDR" \
		"C_S_AXI_CTRL_HIGHADDR" \
		"MEMORY_BASED" \
		"MAX_NR_STREAMS" \
		"HISTOGRAM_BITS" \
		"HAS_Y8" \
		"HAS_Y10" \
		"MAX_COLS" \
		"MAX_ROWS"
		xdefine_config_file $drv_handle "xv_scenechange_g.c" "XV_scenechange" \
		"DEVICE_ID" \
		"C_S_AXI_CTRL_BASEADDR" \
		"MEMORY_BASED" \
		"MAX_NR_STREAMS" \
		"HISTOGRAM_BITS" \
		"HAS_Y8" \
		"HAS_Y10" \
		"MAX_COLS" \
		"MAX_ROWS"

		xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_scenechange" \
		"DEVICE_ID" \
		"C_S_AXI_CTRL_BASEADDR" \
		"C_S_AXI_CTRL_HIGHADDR" \
		"MEMORY_BASED" \
		"MAX_NR_STREAMS" \
		"HISTOGRAM_BITS" \
		"HAS_Y8" \
		"HAS_Y10" \
		"MAX_COLS" \
		"MAX_ROWS"
}
#
# Given a list of arguments, define them all in an include file.
# Handles mpd and mld parameters, as well as the special parameters NUM_INSTANCES,
# DEVICE_ID
#

proc hier_ip_define_include_file {drv_handle file_name drv_string args} {
# Open include file
	set file_handle [::hsi::utils::open_include_file $file_name]

# Get all peripherals connected to this driver
		set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

# Handle special cases
		set arg "NUM_INSTANCES"
		set posn [lsearch -exact $args $arg]
		if {$posn > -1} {
			puts $file_handle "/* Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
# Define NUM_INSTANCES
				puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [llength $periphs]"
				set args [lreplace $args $posn $posn]
		}

# Print all parameters for all peripherals
	set device_id 0
		foreach periph $periphs {
			set periph_name [string toupper [common::get_property NAME $periph]]

				puts $file_handle ""
				puts $file_handle "/* Definitions for peripheral $periph_name */"
				foreach arg $args {

					if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
						set value $device_id
							incr device_id
					} elseif {[string compare -nocase "MEMORY_BASED" $arg] == 0} {
						set value [string toupper [common::get_property CONFIG.$arg $periph]]
							puts $value
							if {[string compare -nocase "TRUE" $value] == 0} {
								set value 1
							} else {
								set value 0
							}
					} elseif {[string compare -nocase "HISTOGRAM_BITS" $arg] == 0} {
						set value $HISTOGRAM_BITS
							puts $value
					} elseif {[string compare -nocase "MAX_NR_STREAMS" $arg] == 0} {
						set value $MAX_NR_STREAMS
							puts $value
					} elseif {[string compare -nocase "HAS_Y8" $arg] == 0} {
						set value [string toupper [common::get_property CONFIG.$arg $periph]]
							puts $value
							if {[string compare -nocase "TRUE" $value] == 0} {
								set value 1
							} else {
								set value 0
							}
					} elseif {[string compare -nocase "HAS_Y10" $arg] == 0} {
						set value [string toupper [common::get_property CONFIG.$arg $periph]]
							puts $value
							if {[string compare -nocase "TRUE" $value] == 0} {
								set value 1
							} else {
								set value 0
							}
					} elseif {[string compare -nocase "MAX_ROWS" $arg] == 0} {
						set value $MAX_ROW
							puts $value
					} elseif {[string compare -nocase "MAX_COLS" $arg] == 0} {
						set value $MAX_COL
							puts $value
					} else {
						set value [common::get_property CONFIG.$arg $periph]
					}
					if {[llength $value] == 0} {
						set value 0
					}
					set value [::hsi::utils::format_addr_string $value $arg]
						puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value"
				}
			puts $file_handle ""
		}
	puts $file_handle "\n/******************************************************************/\n"
		close $file_handle
}

#
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc hier_ip_define_canonical_xpars {drv_handle file_name drv_string args} {
# Open include file
	set file_handle [::hsi::utils::open_include_file $file_name]

# Get all the peripherals connected to this driver
		set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

# Get the names of all the peripherals connected to this driver
		foreach periph $periphs {
			set peripheral_name [string toupper [common::get_property NAME $periph]]
				lappend peripherals $peripheral_name
		}

# Get possible canonical names for all the peripherals connected to this driver
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

								if {[string compare -nocase "MEMORY_BASED" $arg] == 0} {
									set value [string toupper [common::get_property CONFIG.$arg $periph]]
										puts $value
										if {[string compare -nocase "TRUE" $value] == 0} {
											set rvalue 1
										} else {
											set rvalue 0
										}
								} elseif {[string compare -nocase "HISTOGRAM_BITS" $arg] == 0} {
									set value $HISTOGRAM_BITS
										puts $value
								} elseif {[string compare -nocase "MAX_NR_STREAMS" $arg] == 0} {
									set value $MAX_NR_STREAMS
										puts $value
								} elseif {[string compare -nocase "HAS_Y8" $arg] == 0} {
									set value [string toupper [common::get_property CONFIG.$arg $periph]]
										puts $value
										if {[string compare -nocase "TRUE" $value] == 0} {
											set value 1
										} else {
											set value 0
										}
								} elseif {[string compare -nocase "HAS_Y10" $arg] == 0} {
									set value [string toupper [common::get_property CONFIG.$arg $periph]]
										puts $value
										if {[string compare -nocase "TRUE" $value] == 0} {
											set value 1
										} else {
											set value 0
										}
								} elseif {[string compare -nocase "MAX_ROWS" $arg] == 0} {
									set value $MAX_ROW
										puts $value
								} elseif {[string compare -nocase "MAX_COLS" $arg] == 0} {
									set value $MAX_COL
										puts $value
								} else {
									set rvalue [common::get_property CONFIG.$arg $periph]
										if {[llength $rvalue] == 0} {
											set rvalue 0
										}
								}
							set rvalue [::hsi::utils::format_addr_string $rvalue $arg]

								puts $file_handle "#define $lvalue $rvalue"
						}

					incr i
				}
		}

	puts $file_handle "\n/******************************************************************/\n"
		close $file_handle
}
