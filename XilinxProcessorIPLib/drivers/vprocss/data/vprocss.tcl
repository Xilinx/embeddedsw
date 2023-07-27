##############################################################################
# Copyright (C) 2015 - 2022 Xilinx, Inc. All rights reserved.
# Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
#  Ver      Who    Date       Changes
# -------- ------ -------- ----------------------------------------------------
#  1.0      rco    07/21/15 Initial version of subsystem tcl
#  1.1      rco    11/20/15 Bug fix for designs with single instance of
#                           vcresampler core
#  1.2      rco    06/06/16 Extended to support multiple instances
#                  08/22/16 Bug fix for dictionary get without exists check
#  1.3      rco    12/15/16 Added C_DEINT_MOTION_ADAPTIVE option to param list
###############################################################################

proc generate {drv_handle} {
  xdefine_include_file $drv_handle "xparameters.h" "XVprocSs" "NUM_INSTANCES" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID"   "C_SCALER_ALGORITHM" "C_TOPOLOGY" "C_SAMPLES_PER_CLK" "C_MAX_DATA_WIDTH" "C_NUM_VIDEO_COMPONENTS" "C_MAX_COLS" "C_MAX_ROWS" "C_H_SCALER_TAPS" "C_V_SCALER_TAPS" "C_H_SCALER_PHASES" "C_V_SCALER_PHASES" "C_CHROMA_ALGORITHM" "C_H_CHROMA_TAPS" "C_V_CHROMA_TAPS" "C_DEINT_MOTION_ADAPTIVE"

  hier_ip_define_config_file $drv_handle "xvprocss_g.c" "XVprocSs" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_TOPOLOGY" "C_SAMPLES_PER_CLK" "C_MAX_DATA_WIDTH" "C_NUM_VIDEO_COMPONENTS" "C_MAX_COLS" "C_MAX_ROWS" "C_DEINT_MOTION_ADAPTIVE"

  xdefine_canonical_xpars $drv_handle "xparameters.h" "XVprocSs" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID"  "C_SCALER_ALGORITHM" "C_TOPOLOGY" "C_SAMPLES_PER_CLK" "C_MAX_DATA_WIDTH" "C_NUM_VIDEO_COMPONENTS" "C_MAX_COLS" "C_MAX_ROWS" "C_H_SCALER_TAPS" "C_V_SCALER_TAPS" "C_H_SCALER_PHASES" "C_V_SCALER_PHASES" "C_CHROMA_ALGORITHM" "C_H_CHROMA_TAPS" "C_V_CHROMA_TAPS" "C_DEINT_MOTION_ADAPTIVE"
}

#
# Given a list of arguments, define them all in an include file.
# Handles IP model/user parameters, as well as the special parameters NUM_INSTANCES,
# DEVICE_ID
# Will not work for a processor.
#
proc xdefine_include_file {drv_handle file_name drv_string args} {
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

            if {([string compare -nocase "C_DEINT_MOTION_ADAPTIVE" $arg] == 0)} {
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

               if {[string compare -nocase "C_DEINT_MOTION_ADAPTIVE" $arg] == 0} {
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

proc hier_ip_define_config_file {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
    set hw_instance_name [::common::get_property HW_INSTANCE $drv_handle];

    set filename [file join "src" $file_name]
    set config_file [open $filename w]

    ::hsi::utils::write_c_header $config_file "Driver configuration"

    puts $config_file "#include \"xparameters.h\""
    puts $config_file "#include \"[string tolower $drv_string].h\""

    set periphs_g [::hsi::utils::get_common_driver_ips $drv_handle]

    array set sub_core_inst {
        axi_gpio 2
        v_vscaler 1
        v_hscaler 1
        axi_vdma 1
        v_letterbox 1
        v_csc 1
        v_hcresampler 1
        v_vcresampler 2
        v_deinterlacer 1
        axis_switch 1
    }

    foreach periph_g $periphs_g {
        set mem_ranges [::hsi::get_mem_ranges $periph_g]

        ::hsi::current_hw_instance $periph_g;

        puts $config_file "\n/*"
        puts $config_file "* Subsystem Instance: <$periph_g>"
        puts $config_file "*   - List of sub-cores included in the subsystem"
        puts $config_file "*/\n"

        set child_cells_g [::hsi::get_cells -hier]

        foreach child_cell_g $child_cells_g {
            set child_cell_vlnv [::common::get_property VLNV $child_cell_g]
            set child_cell_name_g [common::get_property NAME $child_cell_g]
            set vlnv_arr [split $child_cell_vlnv :]

            lassign $vlnv_arr ip_vendor ip_library ip_name ip_version
            set ip_type_g [common::get_property IP_TYPE $child_cell_g]

            if { [string compare -nocase "BUS" $ip_type_g] != 0 } {
                set interfaces [hsi::get_intf_pins -of_objects $child_cell_g]
                set is_slave 0

                foreach interface $interfaces {
                    set intf_type [common::get_property TYPE $interface]
                    if { [string compare -nocase "SLAVE" $intf_type] == 0 } {
                        set is_slave 1
                    }
                }
                if { $is_slave != 0 } {
                    set final_child_cell_instance_name_present_g XPAR_${child_cell_name_g}_PRESENT
                    puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_present_g]\t 1\n"

                    # create dictionary for ip name and it's instance names "ip_name {inst1_name inst2_name}"
                    dict lappend ss_ip_list $ip_name $child_cell_name_g
                }
            }
        }

        puts $config_file "\n\n/*"
        puts $config_file "* List of sub-cores excluded from the subsystem <$periph_g>"
        puts $config_file "*   - Excluded sub-core device id is set to 255"
        puts $config_file "*   - Excluded sub-core base address is set to 0"
        puts $config_file "*/\n"

        foreach sub_core [lsort [array names sub_core_inst]] {
            if {[IsSubcoreInSubsystem $sub_core $periph_g $ss_ip_list]} {
                set max_instances $sub_core_inst($sub_core)
                #check if core can have multiple instances
                #It is possible that not all instances are used in the design
                if {$max_instances > 1} {
                    set ip_instances [dict get $ss_ip_list $sub_core]
                    set avail_instances [GetNumSubcoreInstances $periph_g $ip_instances]

                    #check if available instances are less than MAX
                    #if yes, mark the missing instance
                    #if all instances are present then skip the core
                    set strval "Unknown"
                    set strbaseaddr "BASEADDR"
                    if {$avail_instances < $max_instances} {
                        if {[string compare -nocase "axi_gpio" $sub_core] == 0} {
                            set ip_inst_name [GetSubcoreInstanceName $periph_g $ip_instances]
                            set srcstr "${periph_g}_reset_sel_axi_mm"
                            if {[string compare -nocase $srcstr $ip_inst_name] == 0} {
                                set strval "RESET_SEL_AXIS"
                            } else {
                                set strval "RESET_SEL_AXI_MM"
                            }
                        } elseif {[string compare -nocase "v_vcresampler" $sub_core] == 0} {
                            #All hls ip base address string is S_AXI_CTRL_BASEADDR"
                            #This check is needed only for IP's with multiple optional instances
                            set strbaseaddr "S_AXI_CTRL_BASEADDR"
                            set ip_inst_name [GetSubcoreInstanceName $periph_g $ip_instances]
                            set srcstr "${periph_g}_vcr_i"
                            if {[string compare -nocase $srcstr $ip_inst_name] == 0} {
                                set strval "VCR_O"
                            } else {
                                set strval "VCR_I"
                            }
                        }

                        set final_child_cell_instance_name_g "XPAR_${periph_g}_${strval}_PRESENT"
                        set final_child_cell_instance_devid_g "XPAR_${periph_g}_${strval}_DEVICE_ID"
                        set final_child_cell_instance_baseaddr_g "XPAR_${periph_g}_${strval}_${strbaseaddr}"
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

    puts $config_file "\n\n"
    set num_insts [::hsi::utils::get_driver_param_name $drv_string "NUM_INSTANCES"]
    puts $config_file [format "%s_Config %s_ConfigTable\[%s\] =" $drv_string $drv_string $num_insts]
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

        ::hsi::current_hw_instance  $periph
        set child_cells [::hsi::get_cells -hier]
        puts $config_file ",\n"

        foreach sub_core [lsort [array names sub_core_inst]] {
            set max_instances $sub_core_inst($sub_core)

            if {[IsSubcoreInSubsystem $sub_core $periph $ss_ip_list]} {
                if {[string match -nocase v_* $sub_core]} {
                        set base_addr_name "S_AXI_CTRL_BASEADDR"
                    } else {
                        set base_addr_name "BASEADDR"
                }

                set ip_instances [dict get $ss_ip_list $sub_core]
                set avail_instances [GetNumSubcoreInstances $periph $ip_instances]

                #check if core can have multiple instances
                #It is possible that not all instances are used in the design
                if {$max_instances > 1} {

                    #check if available instances are less than MAX
                    #if yes, include the missing instance
                    if {$avail_instances < $max_instances} {
                        #retrieve ip instance name from subsystem instance being processed
                        set ip_inst_name [GetSubcoreInstanceName $periph $ip_instances]
                        set count 0
                        set str_name "unknown"

                        while {$count < $max_instances} {
                            if {[string compare -nocase "axi_gpio" $sub_core] == 0} {
                                set str_name [expr {$count == 0 ? "RESET_SEL_AXI_MM" : "RESET_SEL_AXIS"}]
                            } elseif {[string compare -nocase "v_vcresampler" $sub_core] == 0} {
                                set str_name [expr {$count == 0 ? "VCR_I" : "VCR_O"}]
                            }
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
                        set periph_first [concat $periph\_]
                        foreach ip_inst $ip_instances {
                            #write, only if, it belongs to current subsystem instance
                            if {[string first $periph_first $ip_inst] == 0} {
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
                    }
                } else {
                    #retrieve ip instance name from subsystem instance being processed
                    set ip_inst_name [GetSubcoreInstanceName $periph $ip_instances]
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

# Function to retrieve sub-core instance name for subsystem instance being processed
# Param: subsys_inst is Subsystem Instance Name
# Param: instance_list is the list of sub-core instance names in all subsystem instances
#          ex for sub-core vcr:   (v_proc_ss_0_vcr_i  v_proc_ss_0_vcr_0  v_proc_ss_1_vcr_i  v_proc_ss_1_vcr_o)
proc GetSubcoreInstanceName {subsys_inst instance_list} {

    set ip_inst_name "Unknown"
    foreach ip_inst $instance_list {
	set subsys [concat $subsys_inst\_]
	if {[regexp $subsys $ip_inst]} {
            set ip_inst_name $ip_inst
            break
        }
    }
    return $ip_inst_name
}

# Function to check if sub-core instance is present in subsystem instance being processed
# Param: subcore is the core being processed
# Param: subsys_inst is Subsystem Instance Name
# Param: ss_ip_list is the dictionary
proc IsSubcoreInSubsystem {subcore subsys_inst ss_ip_list} {

    set is_present [dict exists $ss_ip_list $subcore]
    if {$is_present == 1} {
        set subcore_instances [dict get $ss_ip_list $subcore]
        # ex for axi_vdma {v_proc_ss_0_axi_vdma v_proc_ss_1_axi_vdma}
        set subcore_inst_name [GetSubcoreInstanceName $subsys_inst $subcore_instances]

        if {[string match "Unknown" $subcore_inst_name]} {
            set is_present 0
        } else {
            set is_present 1
        }
    }
    return $is_present
}

# Function to determine number of sub-core instances available in specified subsystem instance
# Param: subsys_inst is Subsystem Instance Name
# Param: instance_list is the list of sub-core instance names in all subsystem instances in the design
#         ex for axi_vdma {v_proc_ss_0_axi_vdma v_proc_ss_1_axi_vdma}
proc GetNumSubcoreInstances {subsys_inst instance_list} {

    set num_instances 0
    foreach ip_inst $instance_list {
        if {[string first $subsys_inst $ip_inst] == 0} {
            incr num_instances
        }
    }
    return $num_instances
}
