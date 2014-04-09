##############################################################################
#
# (c) Copyright 2004-2014 Xilinx, Inc. All rights reserved.
#
# This file contains confidential and proprietary information of Xilinx, Inc.
# and is protected under U.S. and international copyright and other
# intellectual property laws.
#
# DISCLAIMER
# This disclaimer is not a license and does not grant any rights to the
# materials distributed herewith. Except as otherwise provided in a valid
# license issued to you by Xilinx, and to the maximum extent permitted by
# applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
# FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
# MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
# and (2) Xilinx shall not be liable (whether in contract or tort, including
# negligence, or under any other theory of liability) for any loss or damage
# of any kind or nature related to, arising under or in connection with these
# materials, including for any direct, or any indirect, special, incidental,
# or consequential loss or damage (including loss of data, profits, goodwill,
# or any type of loss or damage suffered as a result of any action brought by
# a third party) even if such damage or loss was reasonably foreseeable or
# Xilinx had been advised of the possibility of the same.
#
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-safe, or for use in
# any application requiring fail-safe performance, such as life-support or
# safety devices or systems, Class III medical devices, nuclear facilities,
# applications related to the deployment of airbags, or any other applications
# that could lead to death, personal injury, or severe property or
# environmental damage (individually and collectively, "Critical
# Applications"). Customer assumes the sole risk and liability of any use of
# Xilinx products in Critical Applications, subject only to applicable laws
# and regulations governing limitations on product liability.
#
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
# AT ALL TIMES.
#
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 4.0     adk    10/12/13 Updated as per the New Tcl API's
##############################################################################

## @BEGIN_CHANGELOG EDK_P
##
##  - Addded the generation of C_*E_FAILING_DATA_REGISTERS to the config structure
##    to distinguish between AXI BRAM and LMB BRAM. These registers are not present 
##    in the current version of the AXI BRAM Controller.
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
    set addr_params [xfind_addr_params $periph]

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
    set file_handle [xopen_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [xget_sw_iplist_for_driver $drv_handle] 
    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [get_property NAME $drv_handle]] */"
        # Define NUM_INSTANCES
        puts $file_handle "#define [xget_dname $drv_string $arg] [llength $periphs]"
        set args [lreplace $args $posn $posn]
    }

    # Check if it is a driver parameter
    lappend newargs 
    foreach arg $args {
        #set value [xget_value $drv_handle "PARAMETER" $arg]
        set value [get_property "$arg" $drv_handle]
        if {[llength $value] == 0} {
            lappend newargs $arg
        } else {
            puts $file_handle "#define [xget_dname $drv_string $arg] [xget_value $drv_handle "PARAMETER" $arg]"
        }
    }
    set args $newargs

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
        set have_ecc [get_property CONFIG.C_ECC $periph]
        set periph_type [get_property IP_NAME $periph]

        puts $file_handle ""
        puts $file_handle "/* Definitions for peripheral [string toupper [get_property NAME $periph]] */"
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
                    set value [xget_param_value $periph $arg]
                }
            } elseif { $periph_type == "lmb_bram_if_cntlr" && [string compare -nocase "C_S_AXI_DATA_WIDTH" $arg] == 0 } {
                # DATA_WIDTH = 32 for lmb_bram_if_cntlr
                set value 32
            } else {
                set value [get_property CONFIG.$arg $periph]
            }
            if {[llength $value] == 0} {
                set value 0
            }

            set value [xformat_addr_string $value $arg]
            set arg_name [xget_dname $periph $arg]
            if { [string compare -nocase "C_S_AXI_DATA_WIDTH" $arg] == 0 } {
                regsub "S_AXI_" $arg_name "" arg_name
            }
            if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define $arg_name \"$value\""
            } else {
                    puts $file_handle "#define $arg_name $value"
            }
        }
        if {$have_ecc == 0} {
            puts $file_handle "#define [xget_dname $periph "C_S_AXI_CTRL_BASEADDR" ] 0xFFFFFFFF "
            puts $file_handle "#define [xget_dname $periph "C_S_AXI_CTRL_HIGHADDR" ] 0xFFFFFFFF "
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
    file delete $filename
    set config_file [open $filename w]
    xprint_generated_header $config_file "Driver configuration"    
    puts $config_file "#include \"xparameters.h\""
    puts $config_file "#include \"[string tolower $drv_string].h\""
    puts $config_file "\n/*"
    puts $config_file "* The configuration table for devices"
    puts $config_file "*/\n"
    puts $config_file [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
    puts $config_file "\{"
    set periphs [xget_sw_iplist_for_driver $drv_handle]
    set start_comma ""
    foreach periph $periphs {
        set have_ecc [get_property CONFIG.C_ECC $periph]
        puts $config_file [format "%s\t\{" $start_comma]
        set comma ""
        set addr_params ""
        set addr_params [find_addr_params $periph]
        set arguments [concat $args $addr_params]
        foreach arg $arguments {
            set local_value [get_property CONFIG.$arg $periph]
            set arg_name [xget_dname $periph $arg]
            # If a parameter isn't found locally (in the current
            # peripheral), we will (for some obscure and ancient reason)
            # look in peripherals connected via point to point links
            if { [string compare -nocase $local_value ""] == 0} { 
                set p2p_name ""
                set p2p_name [xget_p2p_name $periph $arg]
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
            # since the current version doesn't support FFD and FFE registers
            if {[regexp {C_.E_FAILING_REGISTERS} $arg]} {
                set periph_type [get_property IP_NAME $periph]
                if {$periph_type == "axi_bram_ctrl"} {
                    puts -nonewline $config_file [format "%s\t\t%s" $comma "0"]
                } else {
                    puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
                }
            }
            set comma ",\n"
        }
        if {$have_ecc == 0} {
            set arg_name [xget_dname $periph "C_S_AXI_CTRL_BASEADDR"]
            puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
            set arg_name [xget_dname $periph "C_S_AXI_CTRL_HIGHADDR"]
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
    set file_handle [xopen_include_file $file_name]

    # Get all the peripherals connected to this driver
    set periphs [xget_sw_iplist_for_driver $drv_handle]

    # Get the names of all the peripherals connected to this driver
    foreach periph $periphs {
        set peripheral_name [string toupper [get_property NAME $periph]]
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
        #set have_ecc [xget_param_value $periph "C_ECC"]
        set have_ecc [get_property CONFIG.C_ECC $periph]
        set periph_type [get_property IP_NAME $periph]

        set periph_name [string toupper [get_property NAME $periph]]

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
                set lvalue [xget_dname $canonical_name $arg]
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
                    set rvalue [xget_param_value $periph $arg]
                }
                if {[llength $rvalue] == 0} {
                    set rvalue 0
                }
                set rvalue [xformat_addr_string $rvalue $arg]

                puts $file_handle "#define $lvalue $rvalue"
            }
            puts $file_handle ""
            incr i
        }
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}
