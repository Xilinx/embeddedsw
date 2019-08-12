###############################################################################
#
# Copyright (C) 2011 - 2019 Xilinx, Inc.  All rights reserved.
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
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#
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
# 3.2	pkp  27/01/16 Added the support for PL IP interrupts for ZynqMP Soc
# 3.2	pkp  09/03/16 Compute the interrupt ID instead of reading from
#		      interrupt pin property for PL ips in get_psu_interrupt_id
#		      for zynqmpsoc to fix CR#940127
# 3.4	pkp  29/06/16 Updated get_psu_interrupt_id to return correct PL ips'
#		      interruptIDs when no interrupt is connected to pl_ps_irq0
# 3.5	mus  14/10/16 Modified xdefine_gic_params and get_psu_interrupt_id
#		      functions to get correct PL-PS interrupt IDs.Fix for the
#                     CR#961257
# 3.6	pkp  01/22/17 Modified xdefine_zynq_canonical_xpars and
#		      xdefine_zynq_include_file to add hypervisor guest
#		      application support for cortex-a53 64bit mode
# 3.7   ms   04/11/17 Modified tcl file to add U suffix for all macros
#                     in xparameters.h
# 3.8   mus  05/25/17 Updated proc xdefine_gic_params to declare "valid_periph"
#                     variable at start of the proc, to avoid the tcl errors
#                     in case of unsupported processor.It fixes CR#976861
# 3.8   mus  07/05/17 Added support for interrupts connected through
#                     util_reduced_vector IP(OR gate).
# 3.8   mus  07/05/17 Updated xdefine_zynq_canonical_xpars proc to initialize
#                     the HandlerTable in XScuGic_ConfigTable to 0, it removes
#                     the compilation warning in xscugic_g.c. Fix for CR#978736.
# 3.8   mus  07/25/17 Updated xdefine_gic_params proc to export correct canonical
#                     definitions for pl to ps interrupts.Fix for CR#980534
# 3.8   mus  08/17/17 Updated get_psu_interrupt_id proc to check if the sink
#                     pin is connected to peripheral.Fix for CR#980414.
# 3.10  mus  04/23/18 Updated get_psu_interrupt_id to generate correct
#                     interrupt id's, when output of utility reduced logic is
#                     connected to pl-ps interrupt as well as ILA probe. Fix
#                     for CR#999732.
# 3.10  mus  09/10/18 Added -hier option while using get_cells command to
#                     support hierarchical designs.
# 3.10  mus  10/05/18 Updated get_psu_interrupt_id proc to return multiple
#                     interrupt ID's, in case if specific interrupt port of
#                     PL based IP is connected to the pl_ps_irq0 and
#                     pl_ps_irq1 directly or through same concat block pin.
#                     Fix for CR#100266.
# 3.10  mus  10/05/18 Updated get_psu_interrupt_id proc, to fix interrupt id
#                     computation for vectored interrupts. It fixes CR#998583
# 4.0   mus  04/15/19 Updated get_concat_number proc to avoid executing
#                     get_pins command twice. It fixes CR#1028356
# 4.1   mus  04/09/19 Add pl-ps interrupt id generation support for versal
# 4.1   mus  06/20/19 Updated get_concat_number proc to check if
#                     common::get_property LEFT is returning empty. It
#                     fixes CR#1033637.
# 4.1   mus  07/09/19 Unlike ZynqMP, Versal doesnt have IRQ0_F2P/IRQ01_F2P
#                     ports and interrupt source from PL can be directly
#                     connected to the pl_ps_irq0, pl_ps_irq1...pl_ps_irq15
#                     pins. Updated get_psu_interrupt_id proc to generate correct
#                     pl-ps interrupt IDs for Versal. It fixes CR#1017942
#
##############################################################################

#uses "xillib.tcl"

############################################################
# "generate" procedure
############################################################
proc generate {drv_handle} {
    global pl_ps_irq1 pl_ps_irq0
    global or_id
    global or_cnt
    set or_id 0
    set or_cnt 0
    set pl_ps_irq1 0
    set pl_ps_irq0 0

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
    set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [common::get_property IP_NAME $hw_proc_handle]
    set valid_periph 0
    #Get proper gic instance for periphs in case of zynqmp
    foreach periph $periphs {
	if {([string compare -nocase $proctype "ps7_cortexa9"] == 0)|| ([string compare -nocase $proctype "psu_cortexa72"] == 0) ||
	     (([string compare -nocase $proctype "psv_cortexa72"] == 0) && ([string compare -nocase $periph "psv_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psv_cortexr5"] == 0) && ([string compare -nocase $periph "psv_rcpu_gic"] == 0)) ||
	    (([string compare -nocase $proctype "psu_cortexa53"] == 0)&&([string compare -nocase $periph "psu_acpu_gic"] == 0))||
	    (([string compare -nocase $proctype "psu_cortexr5"] == 0)&&([string compare -nocase $periph "psu_rcpu_gic"] == 0))} {
		lappend newperiphs $periph
		set valid_periph 1
	}
    }

    if {!$valid_periph } {
	error "The scugic driver is not supported for the combination of selected IP and processor";
	return;
    }
    set periphs $newperiphs


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
	set value [common::get_property CONFIG.$arg $drv_handle]
	if {[llength $value] == 0} {
	    lappend newargs $arg
	} else {
	    puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property CONFIG.$arg $drv_handle]$uSuffix"
	}
    }
    set args $newargs
    # Print all parameters for all peripherals
    set device_id 0
    set hypervisor_guest [common::get_property CONFIG.hypervisor_guest [get_os] ]
    set procdrv [hsi::get_sw_processor]
    set compiler [get_property CONFIG.compiler $procdrv]
    foreach periph $periphs {
	puts $file_handle ""
	puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
	foreach arg $args {
		if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
			set value $device_id
			incr device_id
		} elseif {[string compare -nocase "C_S_AXI_BASEADDR" $arg] == 0} {
			if {([string compare -nocase $proctype "psu_cortexr5"] == 0) || ([string compare -nocase $proctype "psv_cortexr5"] == 0)} {
				set value 0xF9001000
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set value [common::get_property CONFIG.$arg $periph]
			} elseif {([string compare -nocase $proctype "psu_cortexa72"] == 0) || ([string compare -nocase $proctype "psv_cortexa72"] == 0) } {
			        set value 0xF9040000
			} else {
				if { ($hypervisor_guest == "true") && ([string compare -nocase $compiler "arm-none-eabi-gcc"] != 0) } {
                                     set value 0x03002000
                                } else {
                                     set value 0xF9020000
                                }
			}
		} elseif {[string compare -nocase "C_S_AXI_HIGHADDR" $arg] == 0} {
			if {([string compare -nocase $proctype "psu_cortexr5"] == 0) || ([string compare -nocase $proctype "psv_cortexr5"] == 0) } {
				set value 0xF9001FFF
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set value [common::get_property CONFIG.$arg $periph]
			} elseif {([string compare -nocase $proctype "psu_cortexa72"] == 0) || ([string compare -nocase $proctype "psv_cortexa72"] == 0) } {
			        set value 0xF9041000
			} else {
                                if { ($hypervisor_guest == "true") && ([string compare -nocase $compiler "arm-none-eabi-gcc"] != 0) } {
				   set value 0x03002FFF
                                } else {
                                     set value 0xF9020FFF
                                }
			}
		   } elseif {[string compare -nocase "C_DIST_BASEADDR" $arg] == 0} {
			if {([string compare -nocase $proctype "psu_cortexr5"] == 0) || ([string compare -nocase $proctype "psv_cortexr5"] == 0) } {
				set value 0xF9000000
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set value 0xf8f01000
			} elseif {([string compare -nocase $proctype "psu_cortexa72"] == 0) || ([string compare -nocase $proctype "psv_cortexa72"] == 0) } {
                                set value 0xf9000000
                        } else {
                                if { ($hypervisor_guest == "true") && ([string compare -nocase $compiler "arm-none-eabi-gcc"] != 0) } {
				   set value 0x03001000
                                } else {
                                     set value 0xF9010000
                                }
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
                puts $file_handle "#define $arg_name $value$uSuffix"
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
    set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [common::get_property IP_NAME $hw_proc_handle]

    set valid_periph 0
	set uSuffix "U"
    #Get proper gic instance for periphs in case of zynqmp
    foreach periph $periphs {
	if {([string compare -nocase $proctype "ps7_cortexa9"] == 0) || ([string compare -nocase $proctype "psu_cortexa72"] == 0) ||
	     (([string compare -nocase $proctype "psv_cortexa72"] == 0) && ([string compare -nocase $periph "psv_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psv_cortexr5"] == 0) && ([string compare -nocase $periph "psv_rcpu_gic"] == 0)) ||
	    (([string compare -nocase $proctype "psu_cortexa53"] == 0)&&([string compare -nocase $periph "psu_acpu_gic"] == 0))||
	    (([string compare -nocase $proctype "psu_cortexr5"] == 0)&&([string compare -nocase $periph "psu_rcpu_gic"] == 0))} {
		lappend newperiphs $periph
		set valid_periph 1
	}
    }

    if {!$valid_periph } {
	error "The scugic driver is not supported for the combination of selected IP and processor";
	return;
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
    set hypervisor_guest [common::get_property CONFIG.hypervisor_guest [get_os] ]
    set procdrv [hsi::get_sw_processor]
    set compiler [get_property CONFIG.compiler $procdrv]
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
			if {([string compare -nocase $proctype "psu_cortexr5"] == 0) || ([string compare -nocase $proctype "psv_cortexr5"] == 0)} {
				set rvalue 0xF9001000
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set rvalue [common::get_property CONFIG.$arg $periph]
			} elseif {([string compare -nocase $proctype "psu_cortexa72"] == 0) || ([string compare -nocase $proctype "psv_cortexa72"] == 0) } {
			        set rvalue 0xF9040000
			} else {
                                if { ($hypervisor_guest == "true") && ([string compare -nocase $compiler "arm-none-eabi-gcc"] != 0) } {
                                     set rvalue 0x03002000
                                } else {
                                     set rvalue 0xF9020000
                                }
			}
		} elseif {[string compare -nocase "C_S_AXI_HIGHADDR" $arg] == 0} {
			if {([string compare -nocase $proctype "psu_cortexr5"] == 0) || ([string compare -nocase $proctype "psv_cortexr5"] == 0)} {
				set rvalue 0xF9001FFF
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set rvalue [common::get_property CONFIG.$arg $periph]
			} elseif {([string compare -nocase $proctype "psu_cortexa72"] == 0) || ([string compare -nocase $proctype "psv_cortexa72"] == 0)} {
			        set rvalue 0xF9041000
			} else {
                                if { ($hypervisor_guest == "true") && ([string compare -nocase $compiler "arm-none-eabi-gcc"] != 0) } {
				   set rvalue 0x03002FFF
                                } else {
                                     set rvalue 0xF9020FFF
                                }
			}
		} elseif {[string compare -nocase "C_DIST_BASEADDR" $arg] == 0} {
			if {([string compare -nocase $proctype "psu_cortexr5"] == 0) || ([string compare -nocase $proctype "psv_cortexr5"] == 0)} {
				set rvalue 0xF9000000
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set rvalue 0xf8f01000
                        } elseif {([string compare -nocase $proctype "psu_cortexa72"] == 0) || ([string compare -nocase $proctype "psv_cortexa72"] == 0)} {
                                set rvalue 0xF9000000
			} else {
                                if { ($hypervisor_guest == "true") && ([string compare -nocase $compiler "arm-none-eabi-gcc"] != 0) } {
				   set rvalue 0x03001000
                                } else {
                                     set rvalue 0xF9010000
                                }
			}
		} else {
			set rvalue [common::get_property CONFIG.$arg $periph]
		}
		if {[llength $rvalue] == 0} {
			set rvalue 0
		}
		set rvalue [::hsi::utils::format_addr_string $rvalue $arg]

                puts $file_handle "#define $lvalue $rvalue$uSuffix"

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
   set num_insts [::hsi::utils::get_driver_param_name $drv_string "NUM_INSTANCES"]
   puts $config_file [format "%s_Config %s_ConfigTable\[%s\] =" $drv_string $drv_string $num_insts]
   puts $config_file "\{"
   set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

   #Get the processor instance name
   set sw_proc_handle [hsi::get_sw_processor]
   set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
   set proctype [common::get_property IP_NAME $hw_proc_handle]

    set valid_periph 0
    #Get proper gic instance for periphs in case of zynqmp
    foreach periph $periphs {
	if {([string compare -nocase $proctype "ps7_cortexa9"] == 0)|| ([string compare -nocase $proctype "psu_cortexa72"] == 0)||
	     (([string compare -nocase $proctype "psv_cortexa72"] == 0) && ([string compare -nocase $periph "psv_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psv_cortexr5"] == 0) && ([string compare -nocase $periph "psv_rcpu_gic"] == 0)) ||
	    (([string compare -nocase $proctype "psu_cortexa53"] == 0)&&([string compare -nocase $periph "psu_acpu_gic"] == 0))||
	    (([string compare -nocase $proctype "psu_cortexr5"] == 0)&&([string compare -nocase $periph "psu_rcpu_gic"] == 0))} {
		lappend newperiphs $periph
		set valid_periph 1
	}
    }

    if {!$valid_periph } {
	error "The scugic driver is not supported for the combination of selected IP and processor";
	return;
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
       puts -nonewline $config_file [format "%s\t\t{{0}}\t\t/**< Initialize the HandlerTable to 0 */" $comma]
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
    set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [common::get_property IP_NAME $hw_proc_handle]
    set is_ip_port_detected 0

    set config_inc [::hsi::utils::open_include_file "xparameters.h"]
    # Next define interrupt IDs for each connected peripheral

    set periphs [::hsi::utils::get_common_driver_ips $drvhandle]
    set valid_periph 0
    #Get proper gic instance for periphs in case of zynqmp
    foreach periph $periphs {
	if {([string compare -nocase $proctype "ps7_cortexa9"] == 0)|| ([string compare -nocase $proctype "psu_cortexa72"] == 0)||
	     (([string compare -nocase $proctype "psv_cortexa72"] == 0) && ([string compare -nocase $periph "psv_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psv_cortexr5"] == 0) && ([string compare -nocase $periph "psv_rcpu_gic"] == 0)) ||
	   (([string compare -nocase $proctype "psu_cortexa53"] == 0)&&([string compare -nocase $periph "psu_acpu_gic"] == 0))||
	   (([string compare -nocase $proctype "psu_cortexr5"] == 0)&&([string compare -nocase $periph "psu_rcpu_gic"] == 0))} {
		lappend newperiphs $periph
		set valid_periph 1
	}
    }

    if {!$valid_periph } {
	error "The scugic driver is not supported for the combination of selected IP and processor";
	return;
    }
    set periphs $newperiphs
    set device_id 0

    foreach periph $periphs {

        # get the gic mode information
        set scugic_mode [common::get_property CONFIG.C_IRQ_F2P_MODE $periph]

        # Get the edk based name of peripheral for printing redefines
        set edk_periph_name [common::get_property NAME $periph]

        # Get ports that are driving the interrupt
        set source_ports [::hsi::utils::get_interrupt_sources $periph]
        set i 0
        lappend source_list
        foreach source_port $source_ports {

            if {[llength $source_port] ==0 } {
                continue
            }
            set portType [common::get_property TYPE $source_port]
            if { [string compare -nocase $portType "INTERRUPT"] } {
                continue
            }

            set external_pin [::hsi::utils::is_external_pin $source_port]
            if {$external_pin} {
				set source_port_name($i) [common::get_property NAME $source_port]
				set source_periph($i) ""
				set source_name($i) ""
            } else {
                set source_ip [hsi::get_cells -of_objects $source_port]
                if { [common::get_property IS_PL $source_ip] == 0 } {
                    #add only PL IP. Return all PS IPs
                    continue
                }
		set source_port_name_temp [common::get_property NAME $source_port]
		set source_periph_temp $source_ip
                set source_name_temp [common::get_property NAME $source_periph_temp]
                for {set count 0} {$count < $i} {incr count} {
                    if {([string compare -nocase $source_name_temp $source_name($count)] == 0) &&  ([string compare -nocase $source_port_name($count) $source_port_name_temp ] == 0)} {
                        #IP name and corresponding interrupt pair has been already detected
			set is_ip_port_detected 1
                    }
		}
		if { $is_ip_port_detected == 1} {
                      #Skip the repeated instances of IP name and interrupt port pair
		      continue
		}
	        set source_port_name($i) $source_port_name_temp
		set source_periph($i) $source_periph_temp
                set source_name($i) $source_name_temp
			}
            lappend source_list $source_name($i)
            incr i
        }

        set num_intr_inputs $i
        if {$num_intr_inputs == 0} {
            close $config_inc
            return
        }


        puts $config_inc "/* Definitions for Fabric interrupts connected to $edk_periph_name */"
		set uSuffix "U"
		for {set i 0} {$i < $num_intr_inputs} {incr i} {
            set ip_name   $source_name($i)
            set port_name $source_port_name($i)
            set port_obj  [::hsi::get_ports $port_name]
            if {([string compare -nocase $proctype "psu_cortexa53"] == 0) || ([string compare -nocase $proctype "psu_cortexr5"] == 0) || ([string compare -nocase $proctype "psv_cortexa72"] == 0) || ([string compare -nocase $proctype "psv_cortexr5"] == 0) } {
		set port_intr_id [get_psu_interrupt_id $ip_name $port_name]
                if {[llength $port_intr_id] == 1} {
		      set port_intr_id [expr $port_intr_id + 32]
	        } else {
		      set port_intr_id_temp ""
		      for {set count 0} {$count < [llength $port_intr_id]} {incr count} {
		           lappend port_intr_id_temp [expr [lindex $port_intr_id $count] + 32]
		      }
		      set port_intr_id $port_intr_id_temp
		}
	    } else {
		set port_intr_id [::hsi::utils::get_interrupt_id $ip_name $port_name]
            }
            if { [string compare -nocase $ip_name "system"] } {
                set ip_obj      [::hsi::get_cells -hier $ip_name]
                if {[llength $ip_obj]} {
                    set port_obj    [::hsi::get_pins -of_objects $ip_obj $port_name]
                }
            } else {
                if {([string compare -nocase $proctype "psu_cortexa53"] == 0) || ([string compare -nocase $proctype "psu_cortexr5"] == 0) || ([string compare -nocase $proctype "psv_cortexa72"] == 0) || ([string compare -nocase $proctype "psv_cortexr5"] == 0) } {
			set port_intr_id [get_psu_interrupt_id $ip_name $port_name]
			if {[llength $port_intr_id] == 1} {
				set port_intr_id [expr $port_intr_id + 32]
			} else {
				set port_intr_id_temp ""
				for {set count 0} {$count < [llength $port_intr_id]} {incr count} {
					lappend port_intr_id_temp [expr [lindex $port_intr_id $count] + 32]
				}
				set port_intr_id $port_intr_id_temp
			}
		} else {
			set port_intr_id [::hsi::utils::get_interrupt_id "" $port_name]
		}
            }
            if { [llength $port_intr_id] > 1 } {
                set j 0
                foreach intr_id $port_intr_id {
                    if { [string compare -nocase $ip_name ""] } {
                            puts $config_inc [format "#define XPAR_FABRIC_%s_%s_INTR %d$uSuffix" \
                            [string toupper $ip_name] [string toupper "${port_name}$j"] $intr_id ]
                    } else {
                            puts $config_inc [format "#define XPAR_FABRIC_%s_INTR %d$uSuffix" \
                            [string toupper "${port_name}$j"] $intr_id  ]
                    }
                    incr j
                }
            } else {
                if { [string compare -nocase $ip_name ""] } {
                         puts $config_inc [format "#define XPAR_FABRIC_%s_%s_INTR %d$uSuffix" \
                        [string toupper $ip_name] [string toupper $port_name] $port_intr_id]
                } else {
                        puts $config_inc [format "#define XPAR_FABRIC_%s_INTR %d$uSuffix" \
                        [string toupper $port_name] $port_intr_id]
                }
            }

        }

        puts $config_inc "\n/******************************************************************/\n"
        puts $config_inc "/* Canonical definitions for Fabric interrupts connected to $edk_periph_name */"

        for {set i 0} {$i < $num_intr_inputs} {incr i} {

            # Skip global (external) ports
			if {[string compare -nocase $source_periph($i) ""] != 0} {
            set drv [::hsi::get_drivers -filter "HW_INSTANCE==$source_name($i)"]

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

proc is_orgate { intc_src_port ip_name} {
	set ret -1

	set intr_sink_pins [::hsi::utils::get_sink_pins $intc_src_port]
	set sink_periph [::hsi::get_cells -of_objects $intr_sink_pins]
	set ipname [get_property IP_NAME $sink_periph]
	if { $ipname == "xlconcat" } {
		set intf "dout"
		set intr1_pin [::hsi::get_pins -of_objects $sink_periph -filter "NAME==$intf"]
		set intr_sink_pins [::hsi::utils::get_sink_pins $intr1_pin]
		set sink_periph [::hsi::get_cells -of_objects $intr_sink_pins]
		set ipname [get_property IP_NAME $sink_periph]
	        if {$ipname == "util_reduced_logic"} {
			set width [get_property CONFIG.C_SIZE $sink_periph]
			return $width
		}
	}

	return $ret
}
###################################################################
#
# Get interrupt offset based on accumulated ports
#
###################################################################
proc get_concat_number {ip pin} {
	set number 0
	set pins [hsi::get_pins -of_objects [hsi::get_cells -hier $ip] -filter {DIRECTION=="I"}]
	set pin_num [regexp -all -inline -- {[0-9]+} $pin]

	if {[llength $pins] == 1 || $pin_num == 0} {
		return 0
	}

	for {set p 0} {$p < [llength $pins]} {incr p} {
		if {$pin ==  [lindex $pins $p]} {
			break;
		}
		set offset [common::get_property LEFT [lindex $pins $p]]
		if {[llength $offset] > 1} {
			set offset [lindex $offset 0]
		}
		if {[llength $offset] != 0} {
			set temp [expr {$offset +1}]
			set number [expr {$number + $temp}]
		}

	}
	return $number
}

###################################################################
#
# Get interrupt ID for Versal pl-ps interrupts
#
###################################################################
proc get_psv_interrupt_id { sink_pin } {
    if {[string compare -nocase "$sink_pin" "pl_ps_irq0"] == 0 } {
        return 84
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq1"] == 0 } {
        return 85
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq2"] == 0 } {
        return 86
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq3"] == 0 } {
        return 87
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq4"] == 0 } {
        return 88
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq5"] == 0 } {
        return 89
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq6"] == 0 } {
        return 90
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq7"] == 0 } {
        return 91
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq8"] == 0 } {
        return 92
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq9"] == 0 } {
        return 93
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq10"] == 0 } {
        return 94
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq11"] == 0 } {
        return 95
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq12"] == 0 } {
        return 96
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq13"] == 0 } {
        return 97
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq14"] == 0 } {
        return 98
    } elseif {[string compare -nocase "$sink_pin" "pl_ps_irq15"] == 0 } {
        return 99
    }
}

###################################################################
#
# Get interrupt ID for zynqmpsoc
#
###################################################################
proc get_psu_interrupt_id { ip_name port_name } {
    set ret -1
    set periph ""
    set intr_pin ""
    set is_pl_ps_irq1 0
    set is_pl_ps_irq0 0
    global pl_ps_irq1
    global pl_ps_irq0
    global or_id
    global or_cnt
    set is_versal [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa72" || IP_NAME=="psv_cortexa72"}]

    if { [llength $port_name] == 0 } {
        return $ret
    }

    if { [llength $ip_name] != 0 } {
        #This is the case where IP pin is interrupting
        set periph [::hsi::get_cells -hier -filter "NAME==$ip_name"]
        if { [llength $periph] == 0 } {
            return $ret
        }
        set intr_pin [::hsi::get_pins -of_objects $periph -filter "NAME==$port_name"]
        if { [llength $intr_pin] == 0 } {
            return $ret
        }
        set pin_dir [common::get_property DIRECTION $intr_pin]
        if { [string match -nocase $pin_dir "I"] } {
          return $ret
        }
    } else {
        #This is the case where External interrupt port is interrupting
        set intr_pin [::hsi::get_ports $port_name]
        if { [llength $intr_pin] == 0 } {
            return $ret
        }
        set pin_dir [common::get_property DIRECTION $intr_pin]
        if { [string match -nocase $pin_dir "O"] } {
          return $ret
        }
    }

    set intc_periph [::hsi::utils::get_interrupt_parent $ip_name $port_name]
    if {[llength $intc_periph] > 1} {
        foreach intr_cntr $intc_periph {
            if { [::hsi::utils::is_ip_interrupting_current_proc $intr_cntr] } {
                set intc_periph $intr_cntr
            }
        }
    }
    if { [llength $intc_periph]  ==  0 } {
        return $ret
    }

    set intc_type [common::get_property IP_NAME $intc_periph]
    if {[llength $intc_type] > 1} {
        foreach intr_cntr $intc_type {
            if { [::hsi::utils::is_ip_interrupting_current_proc $intr_cntr] } {
                set intc_type $intr_cntr
            }
        }
    }
    set ip_intr_pin [::hsi::get_pins -of_objects $intc_periph "IRQ0_F2P"]
    set intc_src_ports [::hsi::utils::get_intr_src_pins $ip_intr_pin]
    set total_intr_irq0_count 0
    # Count number of pins connected to IRQ0_F2P
    foreach intc_src_port $intc_src_ports {
        set intr_periph [::hsi::get_cells -of_objects $intc_src_port]
        set intr_width [::hsi::utils::get_port_width $intc_src_port]
        if { [llength $intr_periph] } {
            #case where an a pin of IP is interrupt
            if {[common::get_property IS_PL $intr_periph] == 0} {
                continue
            }
        }
        set total_intr_irq0_count [expr $total_intr_irq0_count + $intr_width]
    }
    set ip_intr_pin [::hsi::get_pins -of_objects $intc_periph "IRQ1_F2P"]
    set intc_src_ports [::hsi::utils::get_intr_src_pins $ip_intr_pin]

    # Count number of pins connected to IRQ1_F2P
    set total_intr_irq1_count 0
    foreach intc_src_port $intc_src_ports {
        set intr_periph [::hsi::get_cells -of_objects $intc_src_port]
        set intr_width [::hsi::utils::get_port_width $intc_src_port]
        if { [llength $intr_periph] } {
            #case where an a pin of IP is interrupt
            if {[common::get_property IS_PL $intr_periph] == 0} {
                continue
            }
        }
        set total_intr_irq1_count [expr $total_intr_irq1_count + $intr_width]
    }
    set intc_src_ports [::hsi::utils::get_interrupt_sources $intc_periph]

    #Special Handling for cascading case of axi_intc Interrupt controller
    set cascade_id 0
    if { [string match -nocase "$intc_type" "axi_intc"] } {
        set cascade_id [::hsi::__internal::get_intc_cascade_id_offset $intc_periph]
    }

    set i $cascade_id
    set found 0
    foreach intc_src_port $intc_src_ports {
        if { [llength $intc_src_port] == 0 } {
            incr i
            continue
        }
        set intr_width [::hsi::utils::get_port_width $intc_src_port]
        set intr_periph [::hsi::get_cells -of_objects $intc_src_port]
        if { [llength $intr_periph] && [is_interrupt $intc_type] } {
            if {[common::get_property IS_PL $intr_periph] == 0 } {
                continue
            }
        }
        set width [is_orgate $intc_src_port $ip_name]
        if { [string compare -nocase "$port_name"  "$intc_src_port" ] == 0 } {
            if { [string compare -nocase "$intr_periph" "$periph"] == 0 && $width != -1} {
                 set or_cnt [expr $or_cnt + 1]
	         if { $or_cnt == $width} {
	                set or_cnt 0
			set or_id [expr $or_id + 1]
		}
                set ret $i
                set found 1
                break
            } elseif { [string compare -nocase "$intr_periph" "$periph"] == 0 } {
		set ret $i
		set found 1
		break
	    }

        }
        if { $width != -1} {
	    set i [expr $or_id]
	} else {
	    set i [expr $i + $intr_width]
	}

    }
    if {[llength $is_versal] == 0} {
        set irq0_base 89
        set irq1_base 104
        set intr_list_irq0 [list 89 90 91 92 93 94 95 96]
        set intr_list_irq1 [list 104 105 106 107 108 109 110 111]
    } else {
        set irq0_base 84
        set irq1_base 92
        set intr_list_irq0 [list 84 85 86 87 88 89 90 91]
        set intr_list_irq1 [list 92 93 94 95 96 97 98 99]
    }
    set sink_pins [::hsi::utils::get_sink_pins $intr_pin]
    if { [llength $sink_pins] == 0 } {
        return
     }

    if { $i > 9} {
        set ret [expr $ret - 8]
    }
    set concat_block 0
    foreach sink_pin $sink_pins {
        set sink_periph [::hsi::get_cells -of_objects $sink_pin]
        if {[llength $sink_periph] == 0} {
            continue
        }
        set connected_ip [get_property IP_NAME [get_cells -hier $sink_periph]]
	# check for direct connection or concat block connected
        if { [string compare -nocase "$connected_ip" "xlconcat"] == 0 } {
            set sink_pin_temp $sink_pin
            set dout "dout"
	    set concat_block 1
	    set intr_pin [::hsi::get_pins -of_objects $sink_periph -filter "NAME==$dout"]
            set is_or_gate 0
            set sink_pins [::hsi::utils::get_sink_pins "$intr_pin"]
            for {set count 0} {$count < 10} {incr count} {
                foreach pin $sink_pins {
                    set sink_pin $pin
                    if { [string compare -nocase "$sink_pin" "IRQ0_F2P"] == 0 } {
                        set is_pl_ps_irq0 1
                    } elseif {[string compare -nocase "$sink_pin" "IRQ1_F2P"] == 0 } {
                        set is_pl_ps_irq1 1
                    } elseif {[string compare -nocase "$sink_pin" "op1"] == 0 } {
                        set is_or_gate 1
                    } elseif {[llength $is_versal] != 0} {
                        set port_intr_id [get_psv_interrupt_id $sink_pin]
                        if {[llength $port_intr_id] != 0} {
                            return $port_intr_id
                        }
                    }
                }
                if { $is_pl_ps_irq0 == 1 || $is_pl_ps_irq1 == 1 || $is_or_gate == 1 } {
                    break
                }
                set sink_periph [::hsi::get_cells -of_objects $sink_pin]
                set intr_pin [::hsi::get_pins -of_objects $sink_periph -filter "NAME==$dout"]
                set sink_pins [::hsi::utils::get_sink_pins "$intr_pin"]
            }
            if { $is_or_gate == 1 } {
                set number [regexp -all -inline -- {[0-9]+} $sink_pin_temp]
            } else {
                set number [get_concat_number $sink_periph $sink_pin_temp]
            }
        } else {
                 #case where interrupts are directly connected to the ps_pl_irq0/ps_pl_irq1 port
                 if { [string compare -nocase "$sink_pin" "IRQ0_F2P"] == 0 } {
                      set is_pl_ps_irq0 1
                 } elseif {[string compare -nocase "$sink_pin" "IRQ1_F2P"] == 0 } {
                       set is_pl_ps_irq1 1
                 } elseif {[llength $is_versal] != 0 } {
                        set port_intr_id [get_psv_interrupt_id $sink_pin]
                        if {[llength $port_intr_id] != 0} {
                            return $port_intr_id
                        }
                 }

        }

        # check for ORgate
	if { [string compare -nocase "$sink_pin" "Op1"] == 0 } {
	    set dout "Res"
	    set sink_periph [::hsi::get_cells -of_objects $sink_pin]
	    set intr_pin [::hsi::get_pins -of_objects $sink_periph -filter "NAME==$dout"]
	    set sink_pins [::hsi::utils::get_sink_pins "$intr_pin"]
	    foreach pin $sink_pins {
	        set sink_pin $pin

	        set sink_periph [::hsi::get_cells -of_objects $sink_pin]
	        set connected_ip [get_property IP_NAME [get_cells -hier $sink_periph]]
	        if { [string compare -nocase "$connected_ip" "xlconcat"] == 0 } {
	            set number [regexp -all -inline -- {[0-9]+} $sink_pin]
	            set dout "dout"
	            set concat_block 1
                    set intr_pin [::hsi::get_pins -of_objects $sink_periph -filter "NAME==$dout"]
	            set sink_pins [::hsi::utils::get_sink_pins "$intr_pin"]
	            foreach pin $sink_pins {
	                set sink_pin $pin
                        if { [string compare -nocase "$sink_pin" "IRQ0_F2P"] == 0 } {
		              set is_pl_ps_irq0 1
			} elseif {[string compare -nocase "$sink_pin" "IRQ1_F2P"] == 0 } {
			      set is_pl_ps_irq1 1
			}
	            }
	         }
	    }
	 }
        set result ""
	if {[llength [hsi::get_ports $port_name]] != 0 && [common::get_property LEFT [hsi::get_ports $port_name]] != ""} {
	    set vector_size [common::get_property LEFT [hsi::get_ports $port_name]]
	    set vector_size [expr {$vector_size + 1}]
	} else {
	    set vector_size 1
	}
	for {set vec 0} {$vec < $vector_size} {incr vec} {
	# generate irq id for IRQ1_F2P
            if {$is_pl_ps_irq1 == 1} {
                if {$found == 1} {
                    set irqval $pl_ps_irq1
                    set pl_ps_irq1 [expr $pl_ps_irq1 + 1]
                    if {$concat_block == "0"} {
                        return [lindex $intr_list_irq1 $irqval]
                    } else {
                        set ret [expr $irq1_base + [expr {$number + $vec}]]
                        lappend result $ret
                    }
	        }
            }
            if {$is_pl_ps_irq0 == 1} {
	        # generate irq id for IRQ0_F2P
                if {$found == 1} {
                    set irqval $pl_ps_irq0
                    set pl_ps_irq0 [expr $pl_ps_irq0 + 1]
		    if {$concat_block == "0"} {
                        return [lindex $intr_list_irq0 $irqval]
                    } else {
                        set ret [expr $irq0_base + [expr {$number + $vec}]]
                        lappend result $ret
                    }
                }
            }
	}
       if { [llength $result] != 0} {
           return $result
       }
    }
    set port_width [::hsi::utils::get_port_width $intr_pin]
    set id $ret
    for {set i 1 } { $i < $port_width } { incr i } {
       lappend ret [expr $id + 1]
       incr $id
    }
    return $ret
}

proc is_interrupt { IP_NAME } {
		if { [string match -nocase $IP_NAME "ps7_scugic"] } {
						return true
		} elseif { ([string match -nocase $IP_NAME "psu_acpu_gic"]) || ([string match -nocase $IP_NAME "psv_acpu_gic"])   } {
						return true
		} elseif { ([string match -nocase $IP_NAME "psu_rcpu_gic"]) || ([string match -nocase $IP_NAME "psv_rcpu_gic"])   } {
						return true
		}
		#puts "return $IP_NAME\n\r"
		return false;
}
