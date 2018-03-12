################################################################################
#
# Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
    hier_ip_define_include_file $drv_handle "xparameters.h" "XSDIAUD" \
    "NUM_INSTANCES" \
    "DEVICE_ID" \
    "C_BASEADDR" \
    "C_HIGHADDR" \
    "C_AUDIO_FUNCTION" \
    "C_LINE_RATE" \
    "C_MAX_AUDIO_CHANNELS"


    ::hsi::utils::define_config_file $drv_handle "xsdiaud_g.c" "XSdiAud" \
    "DEVICE_ID" \
    "C_BASEADDR" \
    "C_AUDIO_FUNCTION" \
    "C_LINE_RATE" \
    "C_MAX_AUDIO_CHANNELS"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "XSDIAUD" \
    "DEVICE_ID" \
    "C_BASEADDR" \
    "C_HIGHADDR" \
    "C_MAX_AUDIO_CHANNELS"

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
			} elseif {[string compare -nocase "C_LINE_RATE" $arg] == 0} {
                set value [string toupper [common::get_property CONFIG.$arg $periph]]
				puts $value
				if {[string compare -nocase "3G_SDI" $value] == 0} {
					set value 0
				} elseif {[string compare -nocase "6G_SDI" $value] == 0} {
					set value 1
				} elseif {[string compare -nocase "12G_SDI_8DS" $value] == 0} {
					set value 2
				} elseif {[string compare -nocase "12G_SDI_16DS" $value] == 0} {
					set value 3
				} else {
				    set value 4
				}
            }  elseif {[string compare -nocase "C_AUDIO_FUNCTION" $arg] == 0} {
                set value [string toupper [common::get_property CONFIG.$arg $periph]]
				puts $value
				if {[string compare -nocase "EMBED" $value] == 0} {
					set value 0
				} elseif {[string compare -nocase "EXTRACT" $value] == 0} {
					set value 1
				} else {
				    set value 2
				}
            } elseif {[string compare -nocase "C_MAX_AUDI0_CHANNELS" $arg] == 0} {
                set value $C_MAX_AUDI0_CHANNELS
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

				if {[string compare -nocase "C_LINE_RATE" $arg] == 0} {
					set value [string toupper [common::get_property CONFIG.$arg $periph]]
					puts $value
					if {[string compare -nocase "3G_SDI" $value] == 0} {
						set rvalue 0
					} elseif {[string compare -nocase "6G_SDI" $value] == 0} {
						set rvalue 1
					} elseif {[string compare -nocase "12G_SDI_8DS" $value] == 0} {
						set rvalue 2
					} elseif {[string compare -nocase "12G_SDI_16DS" $value] == 0} {
						set rvalue 3
					} else {
						set rvalue 4
					}
                } elseif {[string compare -nocase "C_AUDIO_FUNCTION" $arg] == 0} {
					set value [string toupper [common::get_property CONFIG.$arg $periph]]
					puts $value
					if {[string compare -nocase "EMBED" $value] == 0} {
						set rvalue 0
					} elseif {[string compare -nocase "EXTRACT" $value] == 0} {
						set rvalue 1
					} else {
						set rvalue 2
					}
                }  elseif {[string compare -nocase "C_MAX_AUDI0_CHANNELS" $arg] == 0} {
                   set value $C_MAX_AUDI0_CHANNELS
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
