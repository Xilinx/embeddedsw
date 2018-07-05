###############################################################################
# Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 3.0      adk    12/10/13 Updated as per the New Tcl API's
# 3.1 	   adk    20/08/14 Fixed CR:816989 Canonical Definition for Multiple
#			   Instances of UARTSNS550 have the same Device Id.
# 3.2 	   adk    15/10/14 Fixed CR:826435 external clock speed is not
#			   being updated with proper value in xparametrs.h file.
# 3.4      sk     11/09/15 Removed delete filename statement CR# 784758.
# 3.5      ms     04/18/17 Modified tcl file to add suffix U for all macros
#                          definitions of uartns550 in xparameters.h
##############################################################################
## @BEGIN_CHANGELOG EDK_L
##    Deprecated the CLOCK_HZ parameter in mdd and updated the Tcl to obtain the
##    bus frequency during libgen.
##
## @END_CHANGELOG
##
## @BEGIN_CHANGELOG EDK_LS3
##    Updated to obtain external clock frequency from either the port "xin" or
##    the new parameter C_EXTERNAL_XIN_CLK_HZ (if frequecy can't be read from xin)
##    when C_HAS_EXTERNAL_XIN is set to 1
##
## @END_CHANGELOG


proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XUartNs550" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "CLOCK_FREQ_HZ"
    xdefine_config_file $drv_handle "xuartns550_g.c" "XUartNs550"  "DEVICE_ID" "C_BASEADDR" "CLOCK_FREQ_HZ"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "UartNs550" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "CLOCK_FREQ_HZ"
}


#
# Given a list of arguments, define them all in an include file.
# Handles mpd and mld parameters, as well as the special parameters NUM_INSTANCES,
# DEVICE_ID
#
proc xdefine_include_file {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    set uSuffix "U"
    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
        # Define NUM_INSTANCES
        puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [llength $periphs]$uSuffix"
        set args [lreplace $args $posn $posn]
    }

    # define XPAR_XUARTNS550_CLOCK_HZ as bus freq of the
    # 1st instance of the core, for backward compatibility
    set arg "CLOCK_HZ"
    set periph [lindex $periphs 0]
    set freq [xget_freq $periph]
    if {[llength $freq] == 0} {
        set freq [common::get_property CONFIG.C_S_AXI_ACLK_FREQ_HZ $periph]
        if {[llength $freq] == 0} {
            set freq "100000000"
        }
    }
    puts $file_handle "#define [format "%s" [::hsi::utils::get_driver_param_name $drv_string $arg]] $freq$uSuffix"

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
        set periph_name [string toupper [common::get_property NAME $periph]]
        set freq [xget_freq $periph]

        puts $file_handle ""
        puts $file_handle "/* Definitions for peripheral $periph_name */"
        foreach arg $args {

            if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                set value $device_id
                incr device_id
            } elseif {[string compare -nocase "CLOCK_FREQ_HZ" $arg] == 0} {
                if {[llength $freq] == 0} {
                    set freq [common::get_property CONFIG.C_S_AXI_ACLK_FREQ_HZ $periph]
                    if {[llength $freq] == 0} {
                        set freq "100000000"
                        puts "WARNING: Clock frequency information is not available in the design, \
                              for peripheral $periph_name. Assuming a default frequency of 100MHz. \
                              If this is incorrect, the peripheral $periph_name will be non-functional. \
                              See AR 33102 for a solution to work around this problem\n"
                    }
                }
                set value $freq
            } else {
                set value [common::get_property CONFIG.$arg $periph]
            }
            if {[llength $value] == 0} {
                set value 0
            }
            set value [::hsi::utils::format_addr_string $value $arg]
            puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value$uSuffix"
        }
        puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}


#
# Create configuration C file as required by Xilinx driver
#
proc xdefine_config_file {drv_handle file_name drv_string args} {
    set filename [file join "src" $file_name]
    set config_file [open $filename w]
    ::hsi::utils::write_c_header $config_file "Driver configuration"
    puts $config_file "#include \"xparameters.h\""
    puts $config_file "#include \"[string tolower $drv_string].h\""
    puts $config_file "\n/*"
    puts $config_file " * The configuration table for devices"
    puts $config_file " */\n"
    puts $config_file [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
    puts $config_file "\{"
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
    set start_comma ""
    set device_id 0
    foreach periph $periphs {
        puts $config_file [format "%s\t\{" $start_comma]
        set comma ""
        set canonical_name [format "%s_%s" "UartNs550" $device_id]

        foreach arg $args {
            puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_driver_param_name $canonical_name $arg]]
            set comma ",\n"
        }
        puts -nonewline $config_file "\n\t\}"
        set start_comma ",\n"
        incr device_id
    }
    puts $config_file "\n\};"

    puts $config_file "\n";

    close $config_file
}


#
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
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
    set device_id 0
    foreach periph $periphs {
        set periph_name [string toupper [common::get_property NAME $periph]]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {
            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            foreach arg $args {
                set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]

                #handle CLOCK_FREQ_HZ as a special case
                if {[string compare -nocase "CLOCK_FREQ_HZ" $arg] == 0} {
                    set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
                } elseif {[string compare -nocase "DEVICE_ID" $arg] == 0} {
				set rvalue $device_id
				incr device_id
		} else {
                    set rvalue [common::get_property CONFIG.$arg $periph]
                    if {[llength $rvalue] == 0} {
                        set rvalue 0
                    }
                    set rvalue [::hsi::utils::format_addr_string $rvalue $arg]
                }

		set uSuffix [xdefine_getSuffix $lvalue $rvalue]
                puts $file_handle "#define $lvalue $rvalue$uSuffix"
            }

            puts $file_handle ""
            incr i
        }
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}


# Returns the frequency of the UartNs550 peripheral
proc xget_freq {periph} {
        set freq ""

        # Check if the device uses external XIN
        set use_xin_clk [common::get_property CONFIG.C_HAS_EXTERNAL_XIN $periph]
        if { $use_xin_clk == "1" } {
            set port_name "xin"
        }

	set freq [::hsi::utils::get_clk_pin_freq  $periph "S_AXI_ACLK"]

        # If the clock frequency can not be obtained from "xin" port,
        # read the value of the parameter C_EXTERNAL_XIN_CLK_HZ to get
        # the frequency
        if { $use_xin_clk == "1" } {
            set freq [common::get_property CONFIG.C_EXTERNAL_XIN_CLK_HZ $periph]
        }
        return $freq
}
proc xdefine_getSuffix {arg_name value} {
		set uSuffix ""
		if { [string match "*CLOCK_FREQ_HZ" $value] == 0 } {
			set uSuffix "U"
		}
		return $uSuffix
}
