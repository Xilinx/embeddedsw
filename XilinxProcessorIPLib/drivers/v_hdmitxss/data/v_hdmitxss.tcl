################################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
################################################################################
##
## MODIFICATION HISTORY:
##  Ver      Who    Date       Changes
## -------- ------ -------- ----------------------------------------------------
##  1.0     gmagnay 08/14/15 Initial version of subsystem tcl
##  1.1     MG      03/03/16 Added HDCP 2.2
##  1.2     YH      30/07/16 No HDCP22_SS sub-cores included in _g.c generation
##  1.3     YH      16/11/16 Remove Remapper & axi_gpio
##  1.4     MMO     03/01/16 Added sub-core base address computation in TCL
##  1.5     MMO      3/01/17 Fix the TCL to support multiple instance and sync
##                           with XV_HdmiTxSs_Config Data Structure
##  1.6     MM0     13/02/17 Fix by adding typecasting for C++ Fix
##  1.7     MMO     08/02/18 Adding YUV420 Support and Low Resolution
##                           (NTSC/PAL) Support capability
#
################################################################################

proc generate {drv_handle} {
  hier_ip_define_include_file $drv_handle "xparameters.h" "XV_HdmiTxSs" \
                    "NUM_INSTANCES" \
                    "C_BASEADDR" \
                    "C_HIGHADDR" \
                    "DEVICE_ID" \
                    "C_INPUT_PIXELS_PER_CLOCK" \
                    "C_MAX_BITS_PER_COMPONENT" \
		    "C_INCLUDE_LOW_RESO_VID" \
		    "C_INCLUDE_YUV420_SUP" \
                    "AXI_LITE_FREQ_HZ"

  hier_ip_define_config_file $drv_handle "xv_hdmitxss_g.c" "XV_HdmiTxSs" \
                    "DEVICE_ID" \
                    "C_BASEADDR" \
                    "C_HIGHADDR" \
                    "C_INPUT_PIXELS_PER_CLOCK" \
                    "C_MAX_BITS_PER_COMPONENT" \
		    "C_INCLUDE_LOW_RESO_VID" \
		    "C_INCLUDE_YUV420_SUP" \
                    "AXI_LITE_FREQ_HZ"

  hier_ip_define_canonical_xpars $drv_handle "xparameters.h" \
                    "XV_HdmiTxSs" \
                    "C_BASEADDR" \
                    "C_HIGHADDR" \
                    "DEVICE_ID" \
                    "C_INPUT_PIXELS_PER_CLOCK" \
                    "C_MAX_BITS_PER_COMPONENT" \
		    "C_INCLUDE_LOW_RESO_VID" \
		    "C_INCLUDE_YUV420_SUP" \
                    "AXI_LITE_FREQ_HZ"
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
	set low_res_supp 0
	set yuv420_supp 0
    foreach periph $periphs {
        set periph_name [string toupper [common::get_property NAME $periph]]
	    set freq [::hsi::utils::get_clk_pin_freq  $periph "s_axi_cpu_aclk"]

        puts $file_handle ""
        puts $file_handle "/* Definitions for peripheral $periph_name */"
        foreach arg $args {

            if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                set value $device_id
                incr device_id
            } elseif {[string compare -nocase "AXI_LITE_FREQ_HZ" $arg] == 0} {
                if {[llength $freq] == 0} {
                    set freq "100000000"
                    puts "WARNING: Clock frequency information is not available in the design, \
                          for peripheral $periph_name. Assuming a default frequency of 100MHz. \
                          If this is incorrect, the peripheral $periph_name will be non-functional"
                }
                set value $freq
            } else {
                set value [common::get_property CONFIG.$arg $periph]
            }
            if {[llength $value] == 0} {
                set value 0
            }
            set value [::hsi::utils::format_addr_string $value $arg]
			if {[string compare -nocase "C_INCLUDE_LOW_RESO_VID" $arg] == 0} {
				if {$value=="TRUE"} {
					set value 1
				}
				if {$value=="true"} {
					set value 1
				}
				if {$value=="FALSE"} {
					set value 0
				}
				if {$value=="false"} {
					set value 0
				}
			}

			if {[string compare -nocase "C_INCLUDE_YUV420_SUP" $arg] == 0} {
				if {$value=="TRUE"} {
					set value 1
				}
				if {$value=="true"} {
					set value 1
				}
				if {$value=="FALSE"} {
					set value 0
				}
				if {$value=="false"} {
					set value 0
				}
			}
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

		set freq [::hsi::utils::get_clk_pin_freq  $periph "s_axi_cpu_aclk"]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {
            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            foreach arg $args {
                set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]

                #handle AXI_LITE_FREQ_HZ as a special case
                if {[string compare -nocase "AXI_LITE_FREQ_HZ" $arg] == 0} {
				    if {[llength $freq] == 0} {
                        set freq "100000000"
                        puts "WARNING: Clock frequency information is not available in the design, \
                              for peripheral $periph_name. Assuming a default frequency of 100MHz. \
                              If this is incorrect, the peripheral $periph_name will be non-functional"
                    }
                    set rvalue $freq
                } else {
                    set rvalue [common::get_property CONFIG.$arg $periph]
                    if {[llength $rvalue] == 0} {
                        set rvalue 0
                    }
                    set rvalue [::hsi::utils::format_addr_string $rvalue $arg]
                }

				if {[string compare -nocase "C_INCLUDE_LOW_RESO_VID" $arg] == 0} {
					if {$rvalue=="TRUE"} {
						set rvalue 1
					}
					if {$rvalue=="true"} {
						set rvalue 1
					}
					if {$rvalue=="FALSE"} {
						set rvalue 0
					}
					if {$rvalue=="false"} {
						set rvalue 0
					}
				}

				if {[string compare -nocase "C_INCLUDE_YUV420_SUP" $arg] == 0} {
					if {$rvalue=="TRUE"} {
						set rvalue 1
					}
					if {$rvalue=="true"} {
						set rvalue 1
					}
					if {$rvalue=="FALSE"} {
						set rvalue 0
					}
					if {$rvalue=="false"} {
						set rvalue 0
					}
				}

                puts $file_handle "#define $lvalue $rvalue"
            }

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
        axi_timer 1
        hdcp 1
        hdcp22_tx 1
        v_hdmi_tx 1
        v_tc 1
    }

    set count_main 0
    foreach periph_g $periphs_g {
	    #For 2017.1
	    set str_sub_core_baseaddr ABSOLUTE_BASEADDR

        #Clear the dictionary, ensure during multiple instance of sub-system
        #a new dictionary will be created
        foreach sub_core [lsort [array names sub_core_inst]] {
            #clear the dictionary
            dict unset ss_ip_list $sub_core
        }

        #Search sub-system main-baseaddress
        foreach arg $args {
            if {[string compare -nocase "C_BASEADDR" $arg] == 0} {
                set subsystem_baseaddr [::hsi::utils::get_ip_param_name $periph_g $arg]
                continue
            }
        }

        #Set current instance level to sub-system level
        ::hsi::current_hw_instance $periph_g;

        #Prints Header
        puts $config_file "\n/*"
        puts $config_file "* List of Sub-cores included in the Subsystem for Device ID ${count_main}"
        puts $config_file "* Sub-core device id will be set by its driver in xparameters.h"
        puts $config_file "*/\n"

        set child_cells_g [::hsi::get_cells]

        foreach child_cell_g $child_cells_g {
            set child_cell_vlnv [::common::get_property VLNV $child_cell_g]
            set child_cell_name_g [common::get_property NAME $child_cell_g]
            set vlnv_arr [split $child_cell_vlnv :]

            lassign $vlnv_arr ip_vendor ip_library ip_name ip_version
            set ip_type_g [common::get_property IP_TYPE $child_cell_g]

            #puts "IP type $ip_type_g\n"
            if { [string compare -nocase "BUS" $ip_type_g] != 0 } {
                set interfaces [hsi::get_intf_pins -of_objects $child_cell_g]
                set is_slave 0

                foreach interface $interfaces {
                    set intf_type [common::get_property TYPE $interface]
                    #puts "Interface type $intf_type\n"
                    if { [string compare -nocase "SLAVE" $intf_type] == 0 } {
                        set is_slave 1
                    }
                }
                if { $is_slave != 0 } {
                    #puts "Processing Periph: $ip_name  $child_cell_name_g"
                    set final_child_cell_instance_name_present_g XPAR_${child_cell_name_g}_PRESENT
                    set base_addr_name "BASEADDR"
                    set final_child_cell_instance_baseaddress_g XPAR_${child_cell_name_g}_$base_addr_name
                    set final_child_cell_instance_baseaddress_final_g XPAR_${child_cell_name_g}_${str_sub_core_baseaddr}

                    puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_present_g]\t 1\n"

					#For 2017.1 Release
                    puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_baseaddress_final_g]\t [string toupper \($subsystem_baseaddr] + [string toupper $final_child_cell_instance_baseaddress_g]\)\n\n"

                    # create dictionary for ip name and it's instance names "ip_name {inst1_name inst2_name}"
                    dict lappend ss_ip_list $ip_name $child_cell_name_g
                }
            }
        }

        puts $config_file "\n/*"
        puts $config_file "* List of Sub-cores excluded from the subsystem for Device ID ${count_main}"
        puts $config_file "*   - Excluded sub-core device id is set to 255"
        puts $config_file "*   - Excluded sub-core baseaddr is set to 0"
        puts $config_file "*/\n"

        foreach sub_core [lsort [array names sub_core_inst]] {
            #puts $config_file "//sub_core_inst: $sub_core $sub_core_inst($sub_core)\n"

            #Check whether the the sub-core is available in sub-system
            if {[dict exists $ss_ip_list $sub_core]} {
                set max_instances $sub_core_inst($sub_core)
                #check if core can have multiple instances
                #It is possible that not all instances are used in the design
                if {$max_instances > 1} {
                    set ip_instances [dict get $ss_ip_list $sub_core]
                    set avail_instances [llength $ip_instances]

                    #puts "Sub-Core: $sub_core"
                    #puts "instances: $ip_instances"
                    #puts "Available Instance: $avail_instances"

                    #check if available instances are less than MAX
                    #if yes, mark the missing instance
                    #if all instances are present then skip the core
                    if {$avail_instances < $max_instances} {
                        #puts "String Selected: $strval"
                        set final_child_cell_instance_name_g "XPAR_${periph_g}_${strval}_PRESENT"
                        set final_child_cell_instance_devid_g "XPAR_${periph_g}_${strval}_DEVICE_ID"
                        set final_child_cell_instance_baseaddr_g "XPAR_${periph_g}_${strval}_${str_sub_core_baseaddr}"
                        puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_g] 0\n"
                        puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_devid_g] 255\n"
                        puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_baseaddr_g] 0\n\n"
                    }

                }
            } else {
                set count 0
                while {$count<$sub_core_inst($sub_core)} {
				#MMO: Need to ensure the naming of parameter for sub-core
				#     to be the same with common::get_property NAME $child_cell_g
				#     which is same naming as xparameter.h
				#     Need to update, whenever there are productization update
				switch $sub_core {
				  hdcp           {set sub_core_name hdcp_1_4}
				  hdcp22_tx      {set sub_core_name hdcp22_tx_ss}
				  default        {set sub_core_name $sub_core}
				}
                    set final_child_cell_instance_name_g "XPAR_${periph_g}_${sub_core_name}_PRESENT"
                    set final_child_cell_instance_devid_g "XPAR_${periph_g}_${sub_core_name}_DEVICE_ID"
                    set final_child_cell_instance_baseaddr_g "XPAR_${periph_g}_${sub_core_name}_${str_sub_core_baseaddr}"
                    puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_name_g] 0\n"
                    puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_devid_g] 255\n"
                    puts -nonewline $config_file "#define [string toupper $final_child_cell_instance_baseaddr_g] 0\n\n"
                    incr count
                }
            }
        }
        ::hsi::current_hw_instance
        incr count_main
    }

    puts $config_file "\n"
    set num_insts [::hsi::utils::get_driver_param_name $drv_string "NUM_INSTANCES"]
    puts $config_file [format "%s_Config %s_ConfigTable\[%s\] =" $drv_string $drv_string $num_insts]
    puts $config_file "\{"
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
    set start_comma ""
    foreach periph $periphs {
        ::hsi::current_hw_instance $periph;
        set child_cells_g [::hsi::get_cells]
        foreach sub_core [lsort [array names sub_core_inst]] {
            #clear the dictionary
            dict unset ss_ip_list $sub_core
        }

        #MMO: Added to create new dictionary with clean contain
        #     Fix for multiple Sub-system declaration
        #     Notes: There might be better way writing this in TCL
        foreach child_cell_g $child_cells_g {
            set child_cell_vlnv [::common::get_property VLNV $child_cell_g]
            set child_cell_name_g [common::get_property NAME $child_cell_g]
            set vlnv_arr [split $child_cell_vlnv :]

            lassign $vlnv_arr ip_vendor ip_library ip_name ip_version
            set ip_type_g [common::get_property IP_TYPE $child_cell_g]

            #puts "IP type $ip_type_g\n"
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
                    dict lappend ss_ip_list $ip_name $child_cell_name_g
                    #puts "\nss_ip_list = ${ss_ip_list}"
                }
            }
        }

        puts $config_file [format "%s\t\{" $start_comma]
        set comma ""
        foreach arg $args {
            if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s,\n" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
                continue
            }

            if {[string compare -nocase "C_INPUT_PIXELS_PER_CLOCK" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s%s" $comma "(XVidC_PixelsPerClock) " [::hsi::utils::get_ip_param_name $periph $arg]]
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
			set base_addr_name "ABSOLUTE_BASEADDR"

            #Process available Sub-Core in the Subsystem
            #Max Instance, is the Maximum Instance of the sub-core in Subsystem
            #puts "\nProcessing sub-core: $sub_core"
            #puts "Max Instances: $max_instances"

            if {[dict exists $ss_ip_list $sub_core]} {
                #puts "****Sub-core found in dictionary****"
                set ip_instances [dict get $ss_ip_list $sub_core]
                set avail_instances [llength $ip_instances]

                #check if core can have multiple instances
                #It is possible that not all instances are used in the design
                if {$max_instances > 1} {

                    #check if available instances are less than MAX
                    #if yes, include the missing instance
                    if {$avail_instances < $max_instances} {
                        set ip_inst_name [lindex $ip_instances 0]
                        set count 0
                        set str_name "unknown"
                        #puts "IP Inst. Name: $ip_inst_name"
                        while {$count < $max_instances} {
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
                        foreach ip_inst $ip_instances {
                            #puts "instance = $ip_inst"
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
                } else {
                    set ip_inst_name [lindex $ip_instances 0]
                    #puts "IP Instance = $ip_inst_name"
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
                #puts "****sub-core not in dictionary****"
                set count 0

				#MMO: Need to ensure the naming of parameter for sub-core
				#     to be the same with common::get_property NAME $child_cell_g
				#     which is same naming as xparameter.h
				#     Need to update, whenever there are productization update
				switch $sub_core {
				  hdcp           {set sub_core_name hdcp_1_4}
				  hdcp22_tx      {set sub_core_name hdcp22_tx_ss}
				  default        {set sub_core_name $sub_core}
				}


                while {$count< $max_instances} {
                    set final_child_cell_instance_name_present "XPAR_${periph}_${sub_core_name}_PRESENT"
                    set final_child_cell_instance_devid "XPAR_${periph}_${sub_core_name}_DEVICE_ID"
                    set final_child_cell_instance_name_baseaddr "XPAR_${periph}_${sub_core_name}_${base_addr_name}"

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
