###############################################################################
#
# Copyright (C) 2004 - 2014 Xilinx, Inc.  All rights reserved.
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
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 4.0     adk    10/12/13 Updated as per the New Tcl API's
# 4.1     sk     11/09/15 Removed delete filename statement CR# 784758.
# 4.2     ms     04/18/17 Modified tcl file to add suffix U for all macros
#                         definitions of bram in xparameters.h
##############################################################################

## @BEGIN_CHANGELOG EDK_P
##
##  - Addded the generation of C_*E_FAILING_DATA_REGISTERS to the config structure
##    to distinguish between AXI BRAM and LMB BRAM. These registers are not present 
##    in the current common::version of the AXI BRAM Controller.
##
##  - Added sorting of address parameters to not depend on the order in the hardware
##    description.
##
## @END_CHANGELOG

#uses "xillib_sw.tcl"

proc generate {drv_handle} {

    xdefine_include_file $drv_handle "xparameters.h" "XBram" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_DATA_WIDTH" "C_ECC" "C_FAULT_INJECT" "C_CE_FAILING_REGISTERS" "C_UE_FAILING_REGISTERS" "C_ECC_STATUS_REGISTERS" "C_CE_COUNTER_WIDTH" "C_ECC_ONOFF_REGISTER" "C_ECC_ONOFF_RESET_VALUE" "C_WRITE_ACCESS"
    xdefine_config_file $drv_handle "xbram_g.c" "XBram"  "DEVICE_ID" "C_S_AXI_DATA_WIDTH" "C_ECC" "C_FAULT_INJECT" "C_CE_FAILING_REGISTERS" "C_UE_FAILING_REGISTERS" "C_ECC_STATUS_REGISTERS" "C_CE_COUNTER_WIDTH" "C_ECC_ONOFF_REGISTER" "C_ECC_ONOFF_RESET_VALUE" "C_WRITE_ACCESS"
    xdefine_canonical_xpars $drv_handle "xparameters.h" "Bram" "DEVICE_ID" "C_S_AXI_DATA_WIDTH" "C_ECC" "C_FAULT_INJECT" "C_CE_FAILING_REGISTERS" "C_UE_FAILING_REGISTERS" "C_ECC_STATUS_REGISTERS" "C_CE_COUNTER_WIDTH" "C_ECC_ONOFF_REGISTER" "C_ECC_ONOFF_RESET_VALUE" "C_WRITE_ACCESS"

}

#
# Given the peripheral handle, get the list of address parameters,
# and sort them in the following order: *BASE* *HIGH* *CTRL*BASE* *CTRL*HIGH*
#
proc find_addr_params {periph} {
    set addr_params [::hsi::utils::find_addr_params $periph]

    set sorted_addr_params {}
    foreach addr_param $addr_params {
       if {[string first "BASE" $addr_param] > 0 && [string first "CTRL_BASE" $addr_param] < 0} {
           lappend sorted_addr_params $addr_param
       }
    }
    foreach addr_param $addr_params {
       if {[string first "HIGH" $addr_param] > 0 && [string first "CTRL_HIGH" $addr_param] < 0} {
           lappend sorted_addr_params $addr_param
       }
    }
    foreach addr_param $addr_params {
       if {[string first "CTRL_BASE" $addr_param] > 0} {
           lappend sorted_addr_params $addr_param
       }
    }
    foreach addr_param $addr_params {
       if {[string first "CTRL_HIGH" $addr_param] > 0} {
           lappend sorted_addr_params $addr_param
       }
    }

    return $sorted_addr_params
}

#
# Given a list of arguments, define them all in an include file.
# Handles mpd and mld parameters, as well as the special parameters NUM_INSTANCES,
# DEVICE_ID
#
proc xdefine_include_file {drv_handle file_name drv_string args} {

# Parameter values for axi_bram_ctrl. These will be used only when C_ECC=1
    array set axi_bram_params [list		\
	"C_CE_FAILING_REGISTERS"	1	\
	"C_UE_FAILING_REGISTERS"	0	\
	"C_ECC_STATUS_REGISTERS"	1	\
	"C_CE_COUNTER_WIDTH"		8	\
	"C_ECC_ONOFF_REGISTER"		1	\
	"C_WRITE_ACCESS"		1	\
    ]

    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
    # Handle special cases
    set arg "NUM_INSTANCES"

    set uSuffix "U"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
        # Define NUM_INSTANCES
        puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [llength $periphs]$uSuffix"
        set args [lreplace $args $posn $posn]
    }

    # Check if it is a driver parameter
    lappend newargs 
    foreach arg $args {
        #set value [xget_value $drv_handle "PARAMETER" $arg]
        set value [common::get_property "$arg" $drv_handle]
        if {[llength $value] == 0} {
            lappend newargs $arg
        } else {
            puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [xget_value $drv_handle "PARAMETER" $arg]$uSuffix"
        }
    }
    set args $newargs

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
        set have_ecc [common::get_property CONFIG.C_ECC $periph]
        set periph_type [common::get_property IP_NAME $periph]

        puts $file_handle ""
        puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
        set addr_params ""
        set addr_params [find_addr_params $periph]
        set arguments [concat $args $addr_params]
        foreach arg $arguments {
            if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                set value $device_id
                incr device_id
            } elseif { $periph_type == "axi_bram_ctrl" && $have_ecc == 1 } {

                set match 0
                foreach {param value} [array get axi_bram_params] {
                    if {[string compare -nocase $param $arg] == 0} {
                    set match 1
                    break;
                    }
                }
                if {$match == 0} {
                    set value [::hsi::utils::get_param_value $periph $arg]
                }
            } elseif { $periph_type == "lmb_bram_if_cntlr" && [string compare -nocase "C_S_AXI_DATA_WIDTH" $arg] == 0 } {
                # DATA_WIDTH = 32 for lmb_bram_if_cntlr
                set value 32
            } else {
                set value [common::get_property CONFIG.$arg $periph]
            }
            if {[llength $value] == 0} {
                set value 0
            }

            set value [::hsi::utils::format_addr_string $value $arg]
            set arg_name [::hsi::utils::get_driver_param_name $periph $arg]
            if { [string compare -nocase "C_S_AXI_DATA_WIDTH" $arg] == 0 } {
                regsub "S_AXI_" $arg_name "" arg_name
            }
            if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define $arg_name \"$value\""
            } else {
                    puts $file_handle "#define $arg_name $value$uSuffix"
            }
        }
        if {$have_ecc == 0} {
            puts $file_handle "#define [::hsi::utils::get_driver_param_name $periph "C_S_AXI_CTRL_BASEADDR" ] 0xFFFFFFFF$uSuffix "
            puts $file_handle "#define [::hsi::utils::get_driver_param_name $periph "C_S_AXI_CTRL_HIGHADDR" ] 0xFFFFFFFF$uSuffix "
        }
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
        set have_ecc [common::get_property CONFIG.C_ECC $periph]
        puts $config_file [format "%s\t\{" $start_comma]
        set comma ""
        set addr_params ""
        set addr_params [find_addr_params $periph]
        set arguments [concat $args $addr_params]
        foreach arg $arguments {
            set local_value [common::get_property CONFIG.$arg $periph]
            set arg_name [::hsi::utils::get_driver_param_name $periph $arg]
            # If a parameter isn't found locally (in the current
            # peripheral), we will (for some obscure and ancient reason)
            # look in peripherals connected via point to point links
            if { [string compare -nocase $local_value ""] == 0} { 
                set p2p_name ""
                set p2p_name [::hsi::utils::get_p2p_name $periph $arg]
                if { [string compare -nocase $p2p_name ""] == 0} {
                    if { [string compare -nocase "C_S_AXI_DATA_WIDTH" $arg] == 0 } {
                        regsub "S_AXI_" $arg_name "" arg_name
                    }
                    puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
                } else {
                    puts -nonewline $config_file [format "%s\t\t%s" $comma $p2p_name]
                }
            } else {
                if { [string compare -nocase "C_S_AXI_DATA_WIDTH" $arg] == 0 } {
                    regsub "S_AXI_" $arg_name "" arg_name
                }
                puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
                
            }
            # Also output C_*E_FAILING_DATA_REGISTERS, set to 0 for axi_bram_ctrl
            # since the current common::version doesn't support FFD and FFE registers
            if {[regexp {C_.E_FAILING_REGISTERS} $arg]} {
                set periph_type [common::get_property IP_NAME $periph]
                if {$periph_type == "axi_bram_ctrl"} {
                    puts -nonewline $config_file [format "%s\t\t%s" $comma "0"]
                } else {
                    puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
                }
            }
            set comma ",\n"
        }
        if {$have_ecc == 0} {
            set arg_name [::hsi::utils::get_driver_param_name $periph "C_S_AXI_CTRL_BASEADDR"]
            puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
            set arg_name [::hsi::utils::get_driver_param_name $periph "C_S_AXI_CTRL_HIGHADDR"]
            puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
        }
        puts -nonewline $config_file "\n\t\}"
        set start_comma ",\n"
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

# Parameter values for axi_bram_ctrl. These will be used only when C_ECC=1
    array set axi_bram_params [list		\
	"C_CE_FAILING_REGISTERS"	1	\
	"C_UE_FAILING_REGISTERS"	0	\
	"C_ECC_STATUS_REGISTERS"	1	\
	"C_CE_COUNTER_WIDTH"		8	\
	"C_ECC_ONOFF_REGISTER"		1	\
	"C_WRITE_ACCESS"		1	\
    ]

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
        #set have_ecc [::hsi::utils::get_param_value $periph "C_ECC"]
        set have_ecc [common::get_property CONFIG.C_ECC $periph]
        set periph_type [common::get_property IP_NAME $periph]

        set periph_name [string toupper [common::get_property NAME $periph]]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {

            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            # Generate canonical definitions for address parameters
            set addr_params ""
            set addr_params [find_addr_params $periph]
            set arguments [concat $args $addr_params]
            foreach arg $arguments {
                set match 0
                set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]
                regsub "S_AXI_" $lvalue "" lvalue

                if { $periph_type == "axi_bram_ctrl" && $have_ecc == 1 } {

                    foreach {param rvalue} [array get axi_bram_params] {
                        if {[string compare -nocase $param $arg] == 0} {
                            set match 1
                            break;
                        }
                    }
                }
                if { $periph_type == "lmb_bram_if_cntlr" && [string compare -nocase "C_S_AXI_DATA_WIDTH" $arg] == 0 } {
                    # DATA_WIDTH = 32 for lmb_bram_if_cntlr
                    set rvalue 32
                } elseif {$match == 0} {
                    set rvalue [::hsi::utils::get_param_value $periph $arg]
                }
                if {[llength $rvalue] == 0} {
                    set rvalue 0
                }
                set rvalue [::hsi::utils::format_addr_string $rvalue $arg]

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

proc xdefine_getSuffix {arg_name value} {
		set uSuffix ""
		if { [string match "*DEVICE_ID" $value] == 0 } {
			set uSuffix "U"
		}
		return $uSuffix
}
