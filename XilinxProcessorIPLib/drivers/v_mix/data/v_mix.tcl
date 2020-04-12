##############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XV_mix" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "C_S_AXI_CTRL_HIGHADDR" "SAMPLES_PER_CLOCK" "MAX_COLS" "MAX_ROWS" "MAX_DATA_WIDTH" "VIDEO_FORMAT" "NR_LAYERS" \
    "LOGO_LAYER" "MAX_LOGO_COLS" "MAX_LOGO_ROWS" "LOGO_TRANSPARENCY_COLOR" "LOGO_PIXEL_ALPHA" "ENABLE_CSC_COEFFICIENT_REGISTERS" \
    "LAYER1_ALPHA" "LAYER2_ALPHA" "LAYER3_ALPHA" "LAYER4_ALPHA" "LAYER5_ALPHA" "LAYER6_ALPHA" "LAYER7_ALPHA" "LAYER8_ALPHA" \
    "LAYER9_ALPHA" "LAYER10_ALPHA" "LAYER11_ALPHA" "LAYER12_ALPHA" "LAYER13_ALPHA" "LAYER14_ALPHA" "LAYER15_ALPHA" "LAYER16_ALPHA" \
    "LAYER1_UPSAMPLE" "LAYER2_UPSAMPLE" "LAYER3_UPSAMPLE" "LAYER4_UPSAMPLE" "LAYER5_UPSAMPLE" "LAYER6_UPSAMPLE" "LAYER7_UPSAMPLE" "LAYER8_UPSAMPLE" \
    "LAYER9_UPSAMPLE" "LAYER10_UPSAMPLE" "LAYER11_UPSAMPLE" "LAYER12_UPSAMPLE" "LAYER13_UPSAMPLE" "LAYER14_UPSAMPLE" "LAYER15_UPSAMPLE" "LAYER16_UPSAMPLE" \
    "LAYER1_MAX_WIDTH" "LAYER2_MAX_WIDTH" "LAYER3_MAX_WIDTH" "LAYER4_MAX_WIDTH" "LAYER5_MAX_WIDTH" "LAYER6_MAX_WIDTH" "LAYER7_MAX_WIDTH" "LAYER8_MAX_WIDTH" \
    "LAYER9_MAX_WIDTH" "LAYER10_MAX_WIDTH" "LAYER11_MAX_WIDTH" "LAYER12_MAX_WIDTH" "LAYER13_MAX_WIDTH" "LAYER14_MAX_WIDTH" "LAYER15_MAX_WIDTH" "LAYER16_MAX_WIDTH" \
    "LAYER1_INTF_TYPE" "LAYER2_INTF_TYPE" "LAYER3_INTF_TYPE" "LAYER4_INTF_TYPE" "LAYER5_INTF_TYPE" "LAYER6_INTF_TYPE" "LAYER7_INTF_TYPE" "LAYER8_INTF_TYPE" \
    "LAYER9_INTF_TYPE" "LAYER10_INTF_TYPE" "LAYER11_INTF_TYPE" "LAYER12_INTF_TYPE" "LAYER13_INTF_TYPE" "LAYER14_INTF_TYPE" "LAYER15_INTF_TYPE" "LAYER16_INTF_TYPE" \
    "LAYER1_VIDEO_FORMAT" "LAYER2_VIDEO_FORMAT" "LAYER3_VIDEO_FORMAT" "LAYER4_VIDEO_FORMAT" "LAYER5_VIDEO_FORMAT" "LAYER6_VIDEO_FORMAT" "LAYER7_VIDEO_FORMAT" "LAYER8_VIDEO_FORMAT" \
    "LAYER9_VIDEO_FORMAT" "LAYER10_VIDEO_FORMAT" "LAYER11_VIDEO_FORMAT" "LAYER12_VIDEO_FORMAT" "LAYER13_VIDEO_FORMAT" "LAYER14_VIDEO_FORMAT" "LAYER15_VIDEO_FORMAT" "LAYER16_VIDEO_FORMAT"

    xdefine_config_file $drv_handle "xv_mix_g.c" "XV_mix" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "SAMPLES_PER_CLOCK" "MAX_COLS" "MAX_ROWS" "MAX_DATA_WIDTH" "VIDEO_FORMAT" "NR_LAYERS" \
    "LOGO_LAYER" "MAX_LOGO_COLS" "MAX_LOGO_ROWS" "LOGO_TRANSPARENCY_COLOR" "LOGO_PIXEL_ALPHA" "ENABLE_CSC_COEFFICIENT_REGISTERS" \
    "LAYER1_ALPHA" "LAYER2_ALPHA" "LAYER3_ALPHA" "LAYER4_ALPHA" "LAYER5_ALPHA" "LAYER6_ALPHA" "LAYER7_ALPHA" "LAYER8_ALPHA" \
    "LAYER9_ALPHA" "LAYER10_ALPHA" "LAYER11_ALPHA" "LAYER12_ALPHA" "LAYER13_ALPHA" "LAYER14_ALPHA" "LAYER15_ALPHA" "LAYER16_ALPHA" \
    "LAYER1_UPSAMPLE" "LAYER2_UPSAMPLE" "LAYER3_UPSAMPLE" "LAYER4_UPSAMPLE" "LAYER5_UPSAMPLE" "LAYER6_UPSAMPLE" "LAYER7_UPSAMPLE" "LAYER8_UPSAMPLE" \
    "LAYER9_UPSAMPLE" "LAYER10_UPSAMPLE" "LAYER11_UPSAMPLE" "LAYER12_UPSAMPLE" "LAYER13_UPSAMPLE" "LAYER14_UPSAMPLE" "LAYER15_UPSAMPLE" "LAYER16_UPSAMPLE" \
    "LAYER1_MAX_WIDTH" "LAYER2_MAX_WIDTH" "LAYER3_MAX_WIDTH" "LAYER4_MAX_WIDTH" "LAYER5_MAX_WIDTH" "LAYER6_MAX_WIDTH" "LAYER7_MAX_WIDTH" "LAYER8_MAX_WIDTH" \
    "LAYER9_MAX_WIDTH" "LAYER10_MAX_WIDTH" "LAYER11_MAX_WIDTH" "LAYER12_MAX_WIDTH" "LAYER13_MAX_WIDTH" "LAYER14_MAX_WIDTH" "LAYER15_MAX_WIDTH" "LAYER16_MAX_WIDTH" \
    "LAYER1_INTF_TYPE" "LAYER2_INTF_TYPE" "LAYER3_INTF_TYPE" "LAYER4_INTF_TYPE" "LAYER5_INTF_TYPE" "LAYER6_INTF_TYPE" "LAYER7_INTF_TYPE" "LAYER8_INTF_TYPE" \
    "LAYER9_INTF_TYPE" "LAYER10_INTF_TYPE" "LAYER11_INTF_TYPE" "LAYER12_INTF_TYPE" "LAYER13_INTF_TYPE" "LAYER14_INTF_TYPE" "LAYER15_INTF_TYPE" "LAYER16_INTF_TYPE" \
    "LAYER1_VIDEO_FORMAT" "LAYER2_VIDEO_FORMAT" "LAYER3_VIDEO_FORMAT" "LAYER4_VIDEO_FORMAT" "LAYER5_VIDEO_FORMAT" "LAYER6_VIDEO_FORMAT" "LAYER7_VIDEO_FORMAT" "LAYER8_VIDEO_FORMAT" \
    "LAYER9_VIDEO_FORMAT" "LAYER10_VIDEO_FORMAT" "LAYER11_VIDEO_FORMAT" "LAYER12_VIDEO_FORMAT" "LAYER13_VIDEO_FORMAT" "LAYER14_VIDEO_FORMAT" "LAYER15_VIDEO_FORMAT" "LAYER16_VIDEO_FORMAT"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XV_mix" "DEVICE_ID" "C_S_AXI_CTRL_BASEADDR" "C_S_AXI_CTRL_HIGHADDR" "SAMPLES_PER_CLOCK" "MAX_COLS" "MAX_ROWS" "MAX_DATA_WIDTH" "VIDEO_FORMAT" "NR_LAYERS" \
    "LOGO_LAYER" "MAX_LOGO_COLS" "MAX_LOGO_ROWS" "LOGO_TRANSPARENCY_COLOR" "LOGO_PIXEL_ALPHA" "ENABLE_CSC_COEFFICIENT_REGISTERS" \
    "LAYER1_ALPHA" "LAYER2_ALPHA" "LAYER3_ALPHA" "LAYER4_ALPHA" "LAYER5_ALPHA" "LAYER6_ALPHA" "LAYER7_ALPHA" "LAYER8_ALPHA" \
    "LAYER9_ALPHA" "LAYER10_ALPHA" "LAYER11_ALPHA" "LAYER12_ALPHA" "LAYER13_ALPHA" "LAYER14_ALPHA" "LAYER15_ALPHA" "LAYER16_ALPHA" \
    "LAYER1_UPSAMPLE" "LAYER2_UPSAMPLE" "LAYER3_UPSAMPLE" "LAYER4_UPSAMPLE" "LAYER5_UPSAMPLE" "LAYER6_UPSAMPLE" "LAYER7_UPSAMPLE" "LAYER8_UPSAMPLE" \
    "LAYER9_UPSAMPLE" "LAYER10_UPSAMPLE" "LAYER11_UPSAMPLE" "LAYER12_UPSAMPLE" "LAYER13_UPSAMPLE" "LAYER14_UPSAMPLE" "LAYER15_UPSAMPLE" "LAYER16_UPSAMPLE" \
    "LAYER1_MAX_WIDTH" "LAYER2_MAX_WIDTH" "LAYER3_MAX_WIDTH" "LAYER4_MAX_WIDTH" "LAYER5_MAX_WIDTH" "LAYER6_MAX_WIDTH" "LAYER7_MAX_WIDTH" "LAYER8_MAX_WIDTH" \
    "LAYER9_MAX_WIDTH" "LAYER10_MAX_WIDTH" "LAYER11_MAX_WIDTH" "LAYER12_MAX_WIDTH" "LAYER13_MAX_WIDTH" "LAYER14_MAX_WIDTH" "LAYER15_MAX_WIDTH" "LAYER16_MAX_WIDTH" \
    "LAYER1_INTF_TYPE" "LAYER2_INTF_TYPE" "LAYER3_INTF_TYPE" "LAYER4_INTF_TYPE" "LAYER5_INTF_TYPE" "LAYER6_INTF_TYPE" "LAYER7_INTF_TYPE" "LAYER8_INTF_TYPE" \
    "LAYER9_INTF_TYPE" "LAYER10_INTF_TYPE" "LAYER11_INTF_TYPE" "LAYER12_INTF_TYPE" "LAYER13_INTF_TYPE" "LAYER14_INTF_TYPE" "LAYER15_INTF_TYPE" "LAYER16_INTF_TYPE" \
    "LAYER1_VIDEO_FORMAT" "LAYER2_VIDEO_FORMAT" "LAYER3_VIDEO_FORMAT" "LAYER4_VIDEO_FORMAT" "LAYER5_VIDEO_FORMAT" "LAYER6_VIDEO_FORMAT" "LAYER7_VIDEO_FORMAT" "LAYER8_VIDEO_FORMAT" \
    "LAYER9_VIDEO_FORMAT" "LAYER10_VIDEO_FORMAT" "LAYER11_VIDEO_FORMAT" "LAYER12_VIDEO_FORMAT" "LAYER13_VIDEO_FORMAT" "LAYER14_VIDEO_FORMAT" "LAYER15_VIDEO_FORMAT" "LAYER16_VIDEO_FORMAT"

}

#
# Given a list of arguments, define them all in an include file.
# Handles IP model/user parameters, as well as the special parameters NUM_INSTANCES,
# DEVICE_ID
# Will not work for a processor.
#
proc xdefine_include_file {drv_handle file_name drv_string args} {
    set list_alpha {LAYER1_ALPHA LAYER2_ALPHA LAYER3_ALPHA LAYER4_ALPHA LAYER5_ALPHA LAYER6_ALPHA LAYER7_ALPHA LAYER8_ALPHA \
			LAYER9_ALPHA LAYER10_ALPHA LAYER11_ALPHA LAYER12_ALPHA LAYER13_ALPHA LAYER14_ALPHA LAYER15_ALPHA LAYER16_ALPHA}
    set list_upsmpl {LAYER1_UPSAMPLE LAYER2_UPSAMPLE LAYER3_UPSAMPLE LAYER4_UPSAMPLE LAYER5_UPSAMPLE LAYER6_UPSAMPLE LAYER7_UPSAMPLE LAYER8_UPSAMPLE \
			LAYER9_UPSAMPLE LAYER10_UPSAMPLE LAYER11_UPSAMPLE LAYER12_UPSAMPLE LAYER13_UPSAMPLE LAYER14_UPSAMPLE LAYER15_UPSAMPLE LAYER16_UPSAMPLE}

    set args [::hsi::utils::get_exact_arg_list $args]
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [common::get_property name $drv_handle]] */"
        # Define NUM_INSTANCES
        puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [llength $periphs]"
        set args [lreplace $args $posn $posn]
    }

    # Check if it is a driver parameter
    lappend newargs
    foreach arg $args {
        set value [common::get_property CONFIG.$arg $drv_handle]
        if {[llength $value] == 0} {
            lappend newargs $arg
        } else {
            puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property $arg $drv_handle]"
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
            } else {
                set value [common::get_property CONFIG.$arg $periph]
            }
            if {[llength $value] == 0} {
                set value 0
            }
            set value [::hsi::utils::format_addr_string $value $arg]

            if {([string compare -nocase "LOGO_LAYER" $arg] == 0) ||
			    ([string compare -nocase "LOGO_PIXEL_ALPHA" $arg] == 0) ||
			    ([string compare -nocase "LOGO_TRANSPARENCY_COLOR" $arg] == 0)} {
                if {[string compare -nocase "true" "$value"] == 0} {
                    set value 1
                } else {
                    set value 0
                }
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value"
            } elseif {[lsearch -nocase $list_alpha $arg] >= 0} {
                if {[string compare -nocase "true" "$value"] == 0} {
                    set value 1
                } else {
                    set value 0
                }
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value"
            } elseif {[lsearch -nocase $list_upsmpl $arg] >= 0} {
                if {[string compare -nocase "true" "$value"] == 0} {
                    set value 1
                } else {
                    set value 0
                }
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value"
            } else {
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value"
            }
        }
        puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

#
# define_canonical_xpars - Used to print out canonical defines for a driver.
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
    set list_alpha {LAYER1_ALPHA LAYER2_ALPHA LAYER3_ALPHA LAYER4_ALPHA LAYER5_ALPHA LAYER6_ALPHA LAYER7_ALPHA LAYER8_ALPHA
	LAYER9_ALPHA LAYER10_ALPHA LAYER11_ALPHA LAYER12_ALPHA LAYER13_ALPHA LAYER14_ALPHA LAYER15_ALPHA LAYER16_ALPHA}
    set list_upsmpl {LAYER1_UPSAMPLE LAYER2_UPSAMPLE LAYER3_UPSAMPLE LAYER4_UPSAMPLE LAYER5_UPSAMPLE LAYER6_UPSAMPLE LAYER7_UPSAMPLE LAYER8_UPSAMPLE \
			LAYER9_UPSAMPLE LAYER10_UPSAMPLE LAYER11_UPSAMPLE LAYER12_UPSAMPLE LAYER13_UPSAMPLE LAYER14_UPSAMPLE LAYER15_UPSAMPLE LAYER16_UPSAMPLE}

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
               set rvalue [::hsi::utils::get_param_value $periph $arg]
               if {[llength $rvalue] == 0} {
                   set rvalue 0
               }
               set rvalue [::hsi::utils::format_addr_string $rvalue $arg]


               if {[string compare -nocase "LOGO_LAYER" $arg] == 0} {
                    if {[string compare -nocase "true" "$rvalue"] == 0} {
                        set rvalue 1
                    } else {
                        set rvalue 0
                    }
                    puts $file_handle "#define [string toupper $lvalue] $rvalue"
               } elseif {[lsearch -nocase $list_alpha $arg] >= 0} {
                    if {[string compare -nocase "true" "$rvalue"] == 0} {
                        set rvalue 1
                    } else {
                        set rvalue 0
                    }
                    puts $file_handle "#define [string toupper $lvalue] $rvalue"
               } elseif {[lsearch -nocase $list_upsmpl $arg] >= 0} {
                    if {[string compare -nocase "true" "$rvalue"] == 0} {
                        set rvalue 1
                    } else {
                        set rvalue 0
                    }
                    puts $file_handle "#define [string toupper $lvalue] $rvalue"
               } else {
                   puts $file_handle "#define [string toupper $lvalue] $rvalue"
               }
           }
           puts $file_handle ""
           incr i
       }
   }

   puts $file_handle "\n/******************************************************************/\n"
   close $file_handle
}

#
# Create configuration C file as required by Xilinx  drivers
#
proc xdefine_config_file {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
    set filename [file join "src" $file_name]
    #Fix for CR 784758
    #file delete $filename
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
	set start_brace "{{"
	set end_brace "}}"
    foreach periph $periphs {
        puts $config_file [format "%s\t\{" $start_comma]
        set comma ""
        foreach arg $args {
            if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s,\n" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
                continue
            }

            if {[string compare -nocase "LAYER1_ALPHA" $arg] == 0} {
				puts -nonewline $config_file [format "%s\t\t%s\n" $comma $start_brace]
				set comma ""
			}

            if {([string compare -nocase "LAYER1_UPSAMPLE" $arg] == 0)  ||
				([string compare -nocase "LAYER1_MAX_WIDTH" $arg] == 0) ||
				([string compare -nocase "LAYER1_INTF_TYPE" $arg] == 0) ||
				([string compare -nocase "LAYER1_VIDEO_FORMAT" $arg] == 0)} {
					puts -nonewline $config_file [format "\n\t\t%s,\n" $end_brace]
					puts -nonewline $config_file [format "\t\t%s\n" $start_brace]
					set comma ""
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
		puts -nonewline $config_file [format "\n\t\t%s\n" $end_brace]
        puts -nonewline $config_file "\n\t\}"
        set start_comma ",\n"
    }
    puts $config_file "\n\};"

    puts $config_file "\n";

    close $config_file
}
