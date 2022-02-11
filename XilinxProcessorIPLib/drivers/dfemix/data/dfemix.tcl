###############################################################################
# Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date      Changes
# -------- ------ --------- ----------------------------------------------------
# 1.0      dc     02/12/20  Initial Version.
#          dc     03/18/21  New model parameter list
###############################################################################

proc generate {drv_handle} {
    mix_define_include_file $drv_handle "xparameters.h" "XDfeMix" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_MODE" "C_NUM_ANTENNA" "C_MAX_USEABLE_CCIDS" "C_LANES" "C_ANTENNA_INTERLEAVE" "C_MIXER_CPS" "C_DATA_IWIDTH" "C_DATA_OWIDTH" "C_TUSER_WIDTH"
    ::hsi::utils::define_config_file $drv_handle "xdfemix_g.c" "XDfeMix" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_MODE" "C_NUM_ANTENNA" "C_MAX_USEABLE_CCIDS" "C_LANES" "C_ANTENNA_INTERLEAVE" "C_MIXER_CPS" "C_DATA_IWIDTH" "C_DATA_OWIDTH" "C_TUSER_WIDTH"
    mix_define_canonical_xpars $drv_handle "xparameters.h" "XDfeMix" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_MODE" "C_NUM_ANTENNA" "C_MAX_USEABLE_CCIDS" "C_LANES" "C_ANTENNA_INTERLEAVE" "C_MIXER_CPS" "C_DATA_IWIDTH" "C_DATA_OWIDTH" "C_TUSER_WIDTH"
}

proc mix_define_include_file {drv_handle file_name drv_string args} {
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
            } elseif {[string compare -nocase "C_S_AXI_BASEADDR" $arg] == 0} {
                set value [common::get_property CONFIG.$arg $periph]
                if {[llength $value] == 0} {
                    set value [common::get_property CONFIG.C_S_AXI_BASEADDR $periph]
                }
            } elseif {[string compare -nocase "C_MODE" $arg] == 0} {
                if {[string compare -nocase "downlink" [common::get_property CONFIG.C_MODE $periph]] == 0} {
                    set value 0
                } else {
                    set value 1
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

proc mix_define_canonical_xpars {drv_handle file_name drv_string args} {
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

                if {[string compare -nocase "C_S_AXI_BASEADDR" $arg] == 0} {
                    set rvalue [::hsi::utils::get_param_value $periph $arg]
                    if {[llength $rvalue] == 0} {
                        set rvalue [common::get_property CONFIG.C_S_AXI_BASEADDR $periph]
                    }
                } elseif {[string compare -nocase "C_MODE" $arg] == 0} {
                    if {[string compare -nocase "downlink" [common::get_property CONFIG.C_MODE $periph]] == 0} {
                        set rvalue 0
                    } else {
                        set rvalue 1
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

        puts $file_handle "/* Xilinx DUC/DDC Mixer Device Name */"
        set lvalue [::hsi::utils::get_driver_param_name $canonical_name DEV_NAME]
        set rvalue [::hsi::utils::get_param_value $periph C_S_AXI_BASEADDR]
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