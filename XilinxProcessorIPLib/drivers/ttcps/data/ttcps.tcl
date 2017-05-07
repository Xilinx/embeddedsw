###############################################################################
#
# Copyright (C) 2012 - 2015 Xilinx, Inc.  All rights reserved.
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
###############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a sdm  01/20/12 Initialize release
# 2.0   adk  12/10/13 Updated as per the New Tcl API's
# 3.1   sk   11/09/15 Removed delete filename statement CR# 784758.
# 3.4   ms   04/18/17 Modified tcl file to add suffix U for all macros
#                     definitions of ttcps in xparameters.h
#
##############################################################################
#uses "xillib.tcl"

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XTtcPs" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_TTC_CLK_FREQ_HZ" "C_TTC_CLK_CLKSRC"
    xdefine_config_file $drv_handle "xttcps_g.c" "XTtcPs"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_TTC_CLK_FREQ_HZ"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XTtcPs" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_TTC_CLK_FREQ_HZ" "C_TTC_CLK_CLKSRC"
}


proc xdefine_include_file {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]
    set uSuffix "U"
    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
	puts $file_handle "/* Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
	# Define NUM_INSTANCES
	puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [expr [llength $periphs] * 3]$uSuffix"
	set args [lreplace $args $posn $posn]
    }
    # Check if it is a driver parameter

    lappend newargs
    foreach arg $args {
	set value [common::get_property CONFIG.$arg $drv_handle]
	if {[llength $value] == 0} {
	    lappend newargs $arg
	} else {
	    puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property CONFIG.$arg $drv_handle]$uSuffix"
	}
    }
    set args $newargs

    # Print all parameters for all peripherals
    foreach periph $periphs {
        # Set device_id to ttc instance number
        set device_id [string index $periph end]
	puts $file_handle ""
	puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
	for {set x 0} {$x<3} {incr x} {
	    foreach arg $args {
		if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
		    set value [expr $device_id * 3 + $x]
		} elseif {[string compare -nocase "C_TTC_CLK_CLKSRC" $arg] == 0} {
                    set value [::hsi::utils::get_param_value $periph [format "C_TTC_CLK%d_CLKSRC" $x]]
		} elseif {[string compare -nocase "C_TTC_CLK_FREQ_HZ" $arg] == 0} {
                    set value [::hsi::utils::get_param_value $periph [format "C_TTC_CLK%d_FREQ_HZ" $x]]
                } else {
		    set value [::hsi::utils::get_param_value $periph $arg]
		}
		if {[llength $value] == 0} {
		    set value 0
		}
		set value [::hsi::utils::format_addr_string $value $arg]
		if {[string match C_* $arg]} {
		    set arg_name [string range $arg 2 end]
		} else {
		    set arg_name $arg
		}
		set arg_name [format "XPAR_%s_%d_%s" [string toupper [string range [common::get_property NAME $periph] 0 end-2]] [expr $device_id * 3 + $x] $arg_name]
		regsub "S_AXI_" $arg_name "" arg_name
		if {[string compare -nocase "C_S_AXI_BASEADDR" $arg] == 0} {
		    puts $file_handle "#define $arg_name [string toupper [format 0x%08x [expr $value + $x * 4]]]$uSuffix"
		} else {
		    puts $file_handle "#define $arg_name $value$uSuffix"
		}
	    }
	}
	incr device_id
	puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}


#
# Create configuration C file as required by Xilinx  drivers
#
proc xdefine_config_file {drv_handle file_name drv_string args} {
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
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
    set start_comma ""
    foreach periph $periphs {
        # Set device_id to ttc instance number
        set device_id [string index $periph end]
	for {set x 0} {$x<3} {incr x} {
	    puts $config_file [format "%s\t\{" $start_comma]
	    set comma ""
	    foreach arg $args {
		# Check if this is a driver parameter or a peripheral parameter
		set value [common::get_property CONFIG.$arg $drv_handle]
		if {[llength $value] == 0} {
		    set local_value [common::get_property CONFIG.$arg $periph]
		    # If a parameter isn't found locally (in the current
		    # peripheral), we will (for some obscure and ancient reason)
		    # look in peripherals connected via point to point links
		    if { [string compare -nocase $local_value ""] == 0} {
			set p2p_name [::hsi::utils::get_p2p_name $periph $arg]
			if { [string compare -nocase $p2p_name ""] == 0} {
			    if {[string match C_* $arg]} {
				set arg_name [string range $arg 2 end]
			    } else {
				set arg_name $arg
			    }
			    set arg_name [format "XPAR_%s_%d_%s" [string toupper [string range [common::get_property NAME $periph] 0 end-2]] [expr $device_id * 3 + $x] $arg_name]
			    regsub "S_AXI_" $arg_name "" arg_name
			    puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
			} else {
			    regsub "S_AXI_" $p2p_name "" p2p_name
			    puts -nonewline $config_file [format "%s\t\t%s" $comma $p2p_name]
			}
		    } else {
			if {[string match C_* $arg]} {
			    set arg_name [string range $arg 2 end]
			} else {
			    set arg_name $arg
			}
			set arg_name [format "XPAR_%s_%d_%s" [string toupper [string range [common::get_property NAME $periph] 0 end-2]] [expr $device_id * 3 + $x] $arg_name]
			regsub "S_AXI_" $arg_name "" arg_name
			puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
		    }
		} else {
		    set arg_name [::hsi::utils::get_driver_param_name $drv_string $arg]
		    regsub "S_AXI_" $arg_name "" arg_name
		    puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
		}
		set comma ",\n"
	    }
	    puts -nonewline $config_file "\n\t\}"
	    set start_comma ",\n"
	}
	incr device_id
    }
    puts $config_file "\n\};"

    puts $config_file "\n";

    close $config_file
}

#-----------------------------------------------------------------------------
# xdefine_canonical_xpars - Used to print out canonical defines for a driver.
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#-----------------------------------------------------------------------------
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
            lappend indices [expr $device_id + 1]
            lappend indices [expr $device_id + 2]
        }
        incr device_id
    }

    set i 0
    set device_id_l 0
    foreach periph $periphs {
        # Set device_id to ttc instance number
        set device_id [string index $periph end]
        set periph_name [string toupper [common::get_property NAME $periph]]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {
            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            for {set x 0} {$x<3} {incr x} {
                set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

                foreach arg $args {
                    if {[string match C_* $arg]} {
                        set arg_name [string range $arg 2 end]
                    } else {
                        set arg_name $arg
                    }
                    set lvalue [format "XPAR_%s_%d_%s" [string toupper $drv_string] [expr $device_id_l * 3 + $x] $arg_name]
                    regsub "S_AXI_" $lvalue "" lvalue

                    # The commented out rvalue is the name of the instance-specific constant
                    # set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
                    # The rvalue set below is the actual value of the parameter
                    if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                        set rvalue [format "XPAR_%s_%d_%s" [string toupper [string range [common::get_property NAME $periph] 0 end-2]] [expr $device_id * 3 + $x] $arg]
                    } else {
                        if {[string compare -nocase "C_TTC_CLK_CLKSRC" $arg] == 0} {
                            set rvalue [::hsi::utils::get_param_value $periph [format "C_TTC_CLK%d_CLKSRC" $x]]
		        } elseif {[string compare -nocase "C_TTC_CLK_FREQ_HZ" $arg] == 0} {
                            set rvalue [::hsi::utils::get_param_value $periph [format "C_TTC_CLK%d_FREQ_HZ" $x]]
                        } else {
                            set rvalue [::hsi::utils::get_param_value $periph $arg]
                            if {[string compare -nocase "C_S_AXI_BASEADDR" $arg] == 0} {
                                set rvalue [string toupper [format 0x%08x [expr $rvalue + $x * 4]]]
                            }
                        }
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
        incr device_id
	incr device_id_l
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

proc xdefine_getSuffix {arg_name value} {
		set uSuffix ""
		if { [string match "*DEVICE_ID" $value] == 0} {
			set uSuffix "U"
		}
		return $uSuffix
}