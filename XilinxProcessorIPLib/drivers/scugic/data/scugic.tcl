###############################################################################
#
# Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a sdm  04/18/11 Created
# 1.05a hk   06/26/13 Modified to export external interrupts correctly
#                     to xparameters.h. Fix for CR's 690505, 708928 & 719359
# 2.0   adk  12/10/13 Updated as per the New Tcl API's
# 2.1   adk  25/04/14 Added support for corenIRQ/FIQ interrupts.Fix for the
#		      CR#789373
# 3.0	pkp  12/09/14 Added support for Zynq Ultrascale Mp
##############################################################################

#uses "xillib.tcl"

############################################################
# "generate" procedure
############################################################
proc generate {drv_handle} {
    xdefine_gic_params $drv_handle

    xdefine_zynq_include_file $drv_handle "xparameters.h" "XScuGic" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_DIST_BASEADDR"
    xdefine_zynq_config_file $drv_handle "xscugic_g.c" "XScuGic" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_DIST_BASEADDR"
    xdefine_zynq_canonical_xpars $drv_handle "xparameters.h" "ScuGic" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_DIST_BASEADDR"

}

#
# Given a list of arguments, define them all in an include file.
# Similar to proc xdefine_include_file, except that uses regsub
# to replace "S_AXI_" with "".
#
proc xdefine_zynq_include_file {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    #Get the processor instance name
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [common::get_property IP_NAME $hw_proc_handle]
    #Get proper gic instance for periphs in case of zynqmp
    foreach periph $periphs {
	if {([string compare -nocase $proctype "ps7_cortexa9"] == 0)||
	    (([string compare -nocase $proctype "pss_cortexa53"] == 0)&&([string compare -nocase $periph "pss_acpu_gic"] == 0))||
	    (([string compare -nocase $proctype "pss_cortexr5"] == 0)&&([string compare -nocase $periph "pss_rcpu_gic"] == 0))} {
		lappend newperiphs $periph
	}
    }
    set periphs $newperiphs


    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
	puts $file_handle "/* Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
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
	    puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property CONFIG.$arg $drv_handle]"
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
			if {[string compare -nocase $proctype "pss_cortexr5"] == 0} {
				set value 0xF9001000
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set value [common::get_property CONFIG.$arg $periph]
			} else {
				set value 0xF9020000
			}
		} elseif {[string compare -nocase "C_S_AXI_HIGHADDR" $arg] == 0} {
			if {[string compare -nocase $proctype "pss_cortexr5"] == 0} {
				set value 0xF9001FFF
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set value [common::get_property CONFIG.$arg $periph]
			} else {
				set value 0xF9020FFF
			}
		   } elseif {[string compare -nocase "C_DIST_BASEADDR" $arg] == 0} {
			if {[string compare -nocase $proctype "pss_cortexr5"] == 0} {
				set value 0xF9000000
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set value 0xf8f01000
			} else {
				set value 0xF9010000
			}
		} else {
			set value [common::get_property CONFIG.$arg $periph]
		}
	    if {[llength $value] == 0} {
		set value 0
	    }
	    set value [::hsi::utils::format_addr_string $value $arg]
	    set arg_name [::hsi::utils::get_ip_param_name $periph $arg]
	    regsub "S_AXI_" $arg_name "" arg_name
	    if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define $arg_name \"$value\""
	    } else {
                puts $file_handle "#define $arg_name $value"
            }
	}
	puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

#-----------------------------------------------------------------------------
# xdefine_zynq_canonical_xpars - Used to print out canonical defines for a driver.
# Similar to proc xdefine_config_file, except that uses regsub to replace "S_AXI_"
# with "".
#-----------------------------------------------------------------------------
proc xdefine_zynq_canonical_xpars {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all the peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    #Get the processor instance name
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [common::get_property IP_NAME $hw_proc_handle]

    #Get proper gic instance for periphs in case of zynqmp
     foreach periph $periphs {
	if {([string compare -nocase $proctype "ps7_cortexa9"] == 0)||
	    (([string compare -nocase $proctype "pss_cortexa53"] == 0)&&([string compare -nocase $periph "pss_acpu_gic"] == 0))||
	    (([string compare -nocase $proctype "pss_cortexr5"] == 0)&&([string compare -nocase $periph "pss_rcpu_gic"] == 0))} {
			lappend newperiphs $periph
	}
     }
    set periphs $newperiphs

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
                # replace S_SXI_ with CPU_. This is a temporary fix. Revist when the
                # S_AXI_DIST_BASEADDR is generated by the tools
                regsub "S_AXI_" $lvalue "CPU_" lvalue

                # The commented out rvalue is the name of the instance-specific constant
                # set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
                # The rvalue set below is the actual value of the parameter
                if {[string compare -nocase "C_S_AXI_BASEADDR" $arg] == 0} {
			if {[string compare -nocase $proctype "pss_cortexr5"] == 0} {
				set rvalue 0xF9001000
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set rvalue [common::get_property CONFIG.$arg $periph]
			} else {
				set rvalue 0xF9020000
			}
		} elseif {[string compare -nocase "C_S_AXI_HIGHADDR" $arg] == 0} {
			if {[string compare -nocase $proctype "pss_cortexr5"] == 0} {
				set rvalue 0xF9001FFF
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set rvalue [common::get_property CONFIG.$arg $periph]
			} else {
				set rvalue 0xF9020FFF
			}
		} elseif {[string compare -nocase "C_DIST_BASEADDR" $arg] == 0} {
			if {[string compare -nocase $proctype "pss_cortexr5"] == 0} {
				set rvalue 0xF9000000
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set rvalue 0xf8f01000
			} else {
				set rvalue 0xF9010000
			}
		} else {
			set rvalue [common::get_property CONFIG.$arg $periph]
		}
		if {[llength $rvalue] == 0} {
			set rvalue 0
		}
		set rvalue [::hsi::utils::format_addr_string $rvalue $arg]

                puts $file_handle "#define $lvalue $rvalue"

            }
            puts $file_handle ""
            incr i
        }
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

#
# Create configuration C file as required by Xilinx Zynq drivers
# Similar to proc define_config_file, except that uses regsub
# to replace "S_AXI_" with ""
#

proc xdefine_zynq_config_file {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
   set filename [file join "src" $file_name]
   #file delete $filename
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

   #Get the processor instance name
   set sw_proc_handle [hsi::get_sw_processor]
   set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle] ]
   set proctype [common::get_property IP_NAME $hw_proc_handle]

   #Get proper gic instance for periphs in case of zynqmp
   foreach periph $periphs {
	if {([string compare -nocase $proctype "ps7_cortexa9"] == 0)||
	    (([string compare -nocase $proctype "pss_cortexa53"] == 0)&&([string compare -nocase $periph "pss_acpu_gic"] == 0))||
	    (([string compare -nocase $proctype "pss_cortexr5"] == 0)&&([string compare -nocase $periph "pss_rcpu_gic"] == 0))} {
			lappend newperiphs $periph
	}
   }
   set periphs $newperiphs
   set start_comma ""
   foreach periph $periphs {
       puts $config_file [format "%s\t\{" $start_comma]
       set comma ""
       foreach arg $args {
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
                   set arg_name [::hsi::utils::get_ip_param_name $periph $arg]
                   regsub "S_AXI_" $arg_name "" arg_name
                   puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
               } else {
                   regsub "S_AXI_" $p2p_name "" p2p_name
                   puts -nonewline $config_file [format "%s\t\t%s" $comma $p2p_name]
               }
           } else {
               set arg_name [::hsi::utils::get_ip_param_name $periph $arg]
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
   puts $config_file "\n\};"

   puts $config_file "\n";

   close $config_file
}

proc xdefine_gic_params {drvhandle} {

    #Get the processor instance name
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [common::get_property IP_NAME $hw_proc_handle]
    # Fix me
    # Avoid generating fabric interrupts for zynqmp till the issue is fixed
    if {([string compare -nocase $proctype "pss_cortexa53"] == 0) || ([string compare -nocase $proctype "pss_cortexr5"] == 0)} {
    } else {

    set config_inc [::hsi::utils::open_include_file "xparameters.h"]
    # Next define interrupt IDs for each connected peripheral

    set periphs [::hsi::utils::get_common_driver_ips $drvhandle]
    set device_id 0

    foreach periph $periphs {

	# get the gic mode information
	set scugic_mode [common::get_property CONFIG.C_IRQ_F2P_MODE $periph]

        # Get the edk based name of peripheral for printing redefines
        set edk_periph_name [common::get_property NAME $periph]

	# Handle CorenIRQ/FIQ interrupts
	xhandle_coreirq_interrupts

        # Get ports that are driving the interrupt
        #set source_ports [::hsi::utils::get_interrupt_sources $periph]
	set pin [hsi::get_pins -of_objects $periph IRQ_F2P]
	set source_ports [::hsi::utils::get_source_pins $pin]
        set i 0
        lappend source_list
        foreach source_port $source_ports {

            if {[llength $source_port] ==0 } {
                set source_port_name($i) "DUMMY"
                set source_periph($i) "DUMMY"
                set source_name($i) "DUMMY"
            } else {
				set external_pin [::hsi::utils::is_external_pin $source_port]
				if {$external_pin} {
				set source_port_name($i) [common::get_property NAME $source_port]
				set source_periph($i) "system"
				set source_name($i) "system"
				} else {
                set source_port_name($i) [common::get_property NAME $source_port]
                set source_periph($i) [hsi::get_cells -of_object $source_port]
                set source_name($i) [common::get_property NAME $source_periph($i)]
            }
			}
            lappend source_list $source_name($i)
            incr i
        }

        set num_intr_inputs [llength $source_ports]
        if {$num_intr_inputs == 0} {
            close $config_inc
            return
        }


        puts $config_inc "/* Definitions for Fabric interrupts connected to $edk_periph_name */"
        set k 0

	if {[string compare -nocase $scugic_mode "DIRECT"] == 0} {

		for {set i 0} {$i < $num_intr_inputs} {incr i} {

		# Skip dummy ports
		if {[string compare -nocase $source_port_name($i) "DUMMY"] == 0} {
			incr k
			continue
		}

		set j [expr 61 + $k]
		if {$j >68} {
			set j [expr 84 + $k - 8]
		}
		puts $config_inc [format "#define XPAR_%s_%s_%s_INTR %d" "FABRIC" [string toupper $source_name($i)] [string toupper $source_port_name($i)] $j]
		incr k
		}

	} else {

			for {set i [expr ($num_intr_inputs-1)]} {$i >= 0} {incr i -1} {

		# Skip dummy ports
		if {[string compare -nocase $source_port_name($i) "DUMMY"] == 0} {
			incr k
			continue
		}

		set j [expr 91 - $k]
		if {$j < 84} {
			set j [expr 68 - $k + 8]
		}
		puts $config_inc [format "#define XPAR_%s_%s_%s_INTR %d" "FABRIC" [string toupper $source_name($i)] [string toupper $source_port_name($i)] $j]
		incr k
		}

	}

        puts $config_inc "\n/******************************************************************/\n"
        puts $config_inc "/* Canonical definitions for Fabric interrupts connected to $edk_periph_name */"

        for {set i 0} {$i < $num_intr_inputs} {incr i} {

            # Skip dummy ports
            if {[string compare -nocase $source_port_name($i) "DUMMY"] == 0} {
                continue
            }

            # Skip global (external) ports
			if {[string compare -nocase $source_periph($i) "system"] != 0} {
            set port_type [common::get_property TYPE $source_periph($i)]
            if {[string compare -nocase $port_type "global"] == 0} {
                continue
            }
            set drv [hsi::get_drivers -filter "HW_INSTANCE==$$source_name($i)"]
            #set iptype [xget_value $source_periph($i) "OPTION" "IPTYPE"]

            if {[llength $source_name($i)] != 0 && [llength $drv] != 0} {

                set instance [xfind_instance $drv $source_name($i)]
                set drvname [common::get_property  NAME $drv]
                set drvname [string toupper $drvname]

                #
                # Treat sources with multiple interrupt ports slightly different
                # by including the interrupt port name in the canonical constant
                # name
                #
                if { [lcount $source_list $source_name($i)] > 1} {
                    set port_name [string toupper $source_port_name($i)]

                    #
                    # If there are multiple interrupt ports for axi_ethernet, do not include
                    # the port name in the canonical name, for the port "INTERRUPT". Other ports
                    # will have the port name in the canonical name. This is to make sure that
                    # the canonical name for the port INTERRUPT will remain same irrespective of
                    # whether the design has a single interrupt port or multiple interrupt ports
                    #
                    if {[string compare -nocase $drvname "axiethernet"] == 0} {
                        if {[string compare -nocase $port_name "INTERRUPT"] == 0} {
                            set first_part [format "#define XPAR_%s_%s_%s_VEC_ID" "FABRIC" $drvname $instance]
                        } else {
                            set first_part [format "#define XPAR_%s_%s_%s_%s_VEC_ID" "FABRIC" $drvname $instance $port_name]
                        }
                    } else {
                        set first_part [format "#define XPAR_%s_%s_%s_%s_VEC_ID" "FABRIC" $drvname $instance $port_name]
                    }
                } else {
                    set first_part [format "#define XPAR_%s_%s_%s_VEC_ID" "FABRIC" $drvname $instance]
                }

                set second_part [format "XPAR_%s_%s_%s_INTR" "FABRIC" [string toupper $source_name($i)] [string toupper $source_port_name($i)] ]

                if {[string compare -nocase $drvname "generic"] != 0} {
                    puts $config_inc "$first_part $second_part"
                }
                }
            }
        }
        incr device_id
    }

    puts $config_inc "\n/******************************************************************/\n"
    close $config_inc
    }
}

###################################################################
#
# Get the number of elements in the given list that match the
# given entry.  Assume elements are strings.
#
###################################################################
proc lcount {list match_entry} {

    set len [llength $list]
    set count 0

    for {set i 0} {$i < $len} {incr i} {
        set entry [lindex $list $i]
        if { [string compare -nocase $entry $match_entry] == 0} {
            incr count
        }
    }

    return $count
}

###################################################################
#
# Get the HW instance number for a particular device. This will be used to enumerate
# the vector ID defines if more than one interrupt from the core is connected to the
# interrupt controller.
#
###################################################################
proc xfind_instance {drvhandle instname} {

    set instlist [::hsi::utils::get_common_driver_ips $drvhandle]
    set i 0
    foreach inst $instlist {
        set name [common::get_property  NAME $inst]
        if {[string compare -nocase $instname $name] == 0} {
            return $i
        }
        incr i
    }
    set i 0
    return $i
}

###################################################################
#
# Get the type of port, whether it is "local" (from an IP), or
# "global" (from external source).
#
###################################################################
proc xget_port_type {port} {
	set periph [hsi::get_cells -of_objects $port]
	if {[llength $periph] == 0} {
        return "global"
    } else {
        return "local"
    }
}

# ----------------------------------------------------------------------------------
# Tcl procedure for generating Core IRQ interrupts in xparameters.h file
# ----------------------------------------------------------------------------------
proc xhandle_coreirq_interrupts { } {
	set file_handle [::hsi::utils::open_include_file "xparameters.h"]
	puts $file_handle "\n/***Definitions for Core_nIRQ/nFIQ interrupts ****/"
	close $file_handle
	set periphs [hsi::get_cells *]
	foreach periph $periphs {
		set intr_pins [hsi::get_pins -of_objects $periph -filter "TYPE == INTERRUPT"]
		foreach intr_piin $intr_pins {
			set sink_pin [::hsi::utils::get_sink_pins $intr_piin]
			if { $sink_pin == "Core1_nIRQ" || $sink_pin == "Core0_nIRQ" } {
				set ip_name [common::get_property IP_NAME [hsi::get_cells $periph]]
				set periph  [string toupper $periph]
				set intr_pin [string toupper $intr_piin]
				if {$ip_name != "ps7_scugic" } {
					set intr_id "XPAR_FABRIC_${periph}_${intr_pin}_INTR"
					set file_handle [::hsi::utils::open_include_file "xparameters.h"]
					puts $file_handle "#define ${intr_id} 31"
					close $file_handle
				}
			} elseif { $sink_pin == "Core0_nFIQ" || $sink_pin == "Core1_nFIQ" } {
				set ip_name [common::get_property IP_NAME [hsi::get_cells $periph]]
				set periph  [string toupper $periph]
				set intr_pin [string toupper $intr_piin]
				if {$ip_name != "ps7_scugic" } {
					set intr_id "XPAR_FABRIC_${periph}_${intr_pin}_INTR"
					set file_handle [::hsi::utils::open_include_file "xparameters.h"]
					puts $file_handle "#define ${intr_id} 28"
					close $file_handle
				}
			}
		}
	}
}
