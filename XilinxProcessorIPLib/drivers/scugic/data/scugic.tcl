###############################################################################
# Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
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
# 4.3   mus  07/10/20 Updated get_psu_interrupt_id and xdefine_gic_params proc
#                     to support broader range of HW designs. These changes
#                     have been done specifically to support cascaded concat
#                     blocks and slice IP. Also,  get_psu_interrupt_id logic
#                     has been modified to make interrupt calculation logic
#                     generic, to support HW designs where concat, slice and OR
#                     gate IPs are connected in different manner.
# 4.3   mus  08/26/20 Updated procs to make use of IP_NAME, instead of directly
#                     using instance name. Instance names for GIC might vary
#                     based on the HW design, so its better to avoid using
#                     instance names directly. It fixes CR#1073003.
# 4.5   mus  12/14/20 Updated get_psu_interrupt_id proc with additional checks
#                     to fix BSP generation for specific designs. It fixes
#                     CR#1084286.
# 4.6   mus  06/25/21 Used get_param_value instead of get_property to read
#                     IP parameters. This has been done to support SSIT
#                     devices.
# 4.6   dp   08/04/21 Defined get_interrupt_sources and dependent procs to handle
#                     Utility vector logic. Also defined get_interrupt_parent and
#                     get_connected_intr_cntrl to handle utility vector logic and
#                     return proper interrupt controller.
# 5.0   mus  22/02/22 Added support for VERSAL NET.
# 5.2   mus  08/08/22 Support PL to PS interrupts for VERSAL NET.
##############################################################################

#uses "xillib.tcl"

############################################################
# "generate" procedure
############################################################
    array set traversed_port_name ""
    array set traversed_ip_name ""
    set traversed_ports_count 0

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

    set intr_wrap [common::get_property CONFIG.xil_interrupt [hsi::get_os]]
    if { [string match -nocase $intr_wrap "true"] > 0} {
        generate_ipdefine $drv_handle "xparameters.h"
    }
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
    foreach periph_inst $periphs {
	    set periph [common::get_property IP_NAME $periph_inst]
	if {([string compare -nocase $proctype "ps7_cortexa9"] == 0)|| ([string compare -nocase $proctype "psu_cortexa72"] == 0) ||
	     (([string compare -nocase $proctype "psv_cortexa72"] == 0) && ([string compare -nocase $periph "psv_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psv_cortexr5"] == 0) && ([string compare -nocase $periph "psv_rcpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psxl_cortexa78"] == 0) && ([string compare -nocase $periph "psxl_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psxl_cortexr52"] == 0) && ([string compare -nocase $periph "psxl_rcpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psx_cortexa78"] == 0) && ([string compare -nocase $periph "psx_acpu_gic"] == 0)) ||
             (([string compare -nocase $proctype "psx_cortexr52"] == 0) && ([string compare -nocase $periph "psx_rcpu_gic"] == 0)) ||
	    (([string compare -nocase $proctype "psu_cortexa53"] == 0)&&([string compare -nocase $periph "psu_acpu_gic"] == 0))||
	    (([string compare -nocase $proctype "psu_cortexr5"] == 0)&&([string compare -nocase $periph "psu_rcpu_gic"] == 0))} {
		lappend newperiphs $periph_inst
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
	set value [::hsi::utils::get_param_value $drv_handle $arg]
	if {[llength $value] == 0 || [string compare -nocase "DEVICE_ID" $arg] == 0} {
	    lappend newargs $arg
	} else {
	    puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [::hsi::utils::get_param_value $drv_handle $arg]$uSuffix"
	}
    }
    set args $newargs
    # Print all parameters for all peripherals
    set device_id 0
    set hypervisor_guest [common::get_property CONFIG.hypervisor_guest [hsi::get_os] ]
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
			} elseif {([string compare -nocase $proctype "psxl_cortexr52"] == 0) || ([string compare -nocase $proctype "psxl_cortexa78"] == 0) || ([string compare -nocase $proctype "psx_cortexr52"] == 0) || ([string compare -nocase $proctype "psx_cortexa78"] == 0)} {
				set value 0xE2001000
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set value [::hsi::utils::get_param_value $periph $arg]
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
			} elseif {([string compare -nocase $proctype "psxl_cortexr52"] == 0) || ([string compare -nocase $proctype "psxl_cortexa78"] == 0) || ([string compare -nocase $proctype "psx_cortexr52"] == 0) || ([string compare -nocase $proctype "psx_cortexa78"] == 0)} {
				set value 0xE2001FFF
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set value [::hsi::utils::get_param_value $periph $arg]
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
			} elseif {([string compare -nocase $proctype "psxl_cortexr52"] == 0) || ([string compare -nocase $proctype "psxl_cortexa78"] == 0) || ([string compare -nocase $proctype "psx_cortexr52"] == 0) || ([string compare -nocase $proctype "psx_cortexa78"] == 0)} {
				set value 0xE2000000
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
			set value [::hsi::utils::get_param_value $periph $arg]
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
    foreach periph_inst $periphs {
	set periph [common::get_property IP_NAME $periph_inst]
	if {([string compare -nocase $proctype "ps7_cortexa9"] == 0) || ([string compare -nocase $proctype "psu_cortexa72"] == 0) ||
	     (([string compare -nocase $proctype "psv_cortexa72"] == 0) && ([string compare -nocase $periph "psv_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psv_cortexr5"] == 0) && ([string compare -nocase $periph "psv_rcpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psxl_cortexa78"] == 0) && ([string compare -nocase $periph "psxl_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psxl_cortexr52"] == 0) && ([string compare -nocase $periph "psxl_rcpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psx_cortexa78"] == 0) && ([string compare -nocase $periph "psx_acpu_gic"] == 0)) ||
             (([string compare -nocase $proctype "psx_cortexr52"] == 0) && ([string compare -nocase $periph "psx_rcpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psu_cortexa53"] == 0)&&([string compare -nocase $periph "psu_acpu_gic"] == 0))||
	    (([string compare -nocase $proctype "psu_cortexr5"] == 0)&&([string compare -nocase $periph "psu_rcpu_gic"] == 0))} {
		lappend newperiphs $periph_inst
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
    set hypervisor_guest [common::get_property CONFIG.hypervisor_guest [hsi::get_os] ]
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
			} elseif {([string compare -nocase $proctype "psxl_cortexr52"] == 0) || ([string compare -nocase $proctype "psxl_cortexa78"] == 0) || ([string compare -nocase $proctype "psx_cortexr52"] == 0) || ([string compare -nocase $proctype "psx_cortexa78"] == 0)} {
				set rvalue 0xE2001000
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set rvalue [::hsi::utils::get_param_value $periph $arg]
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
			} elseif {([string compare -nocase $proctype "psxl_cortexr52"] == 0) || ([string compare -nocase $proctype "psxl_cortexa78"] == 0) || ([string compare -nocase $proctype "psx_cortexr52"] == 0) || ([string compare -nocase $proctype "psx_cortexa78"] == 0)} {
				set rvalue 0xE2001FFF
			} elseif {[string compare -nocase $proctype "ps7_cortexa9"] == 0} {
				set rvalue [::hsi::utils::get_param_value $periph $arg]
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
			} elseif {([string compare -nocase $proctype "psxl_cortexr52"] == 0) || ([string compare -nocase $proctype "psxl_cortexa78"] == 0) || ([string compare -nocase $proctype "psx_cortexr52"] == 0) || ([string compare -nocase $proctype "psx_cortexa78"] == 0)} {
				set rvalue 0xE2000000
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
			set rvalue [::hsi::utils::get_param_value $periph $arg]
		}
		if {[llength $rvalue] == 0 || [string compare -nocase "DEVICE_ID" $arg] == 0} {
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
    foreach periph_inst $periphs {
	set periph [common::get_property IP_NAME $periph_inst]
	if {([string compare -nocase $proctype "ps7_cortexa9"] == 0)|| ([string compare -nocase $proctype "psu_cortexa72"] == 0)||
	     (([string compare -nocase $proctype "psv_cortexa72"] == 0) && ([string compare -nocase $periph "psv_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psv_cortexr5"] == 0) && ([string compare -nocase $periph "psv_rcpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psxl_cortexa78"] == 0) && ([string compare -nocase $periph "psxl_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psxl_cortexr52"] == 0) && ([string compare -nocase $periph "psxl_rcpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psx_cortexa78"] == 0) && ([string compare -nocase $periph "psx_acpu_gic"] == 0)) ||
             (([string compare -nocase $proctype "psx_cortexr52"] == 0) && ([string compare -nocase $periph "psx_rcpu_gic"] == 0)) ||
	    (([string compare -nocase $proctype "psu_cortexa53"] == 0)&&([string compare -nocase $periph "psu_acpu_gic"] == 0))||
	    (([string compare -nocase $proctype "psu_cortexr5"] == 0)&&([string compare -nocase $periph "psu_rcpu_gic"] == 0))} {
		lappend newperiphs $periph_inst
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
           set value [::hsi::utils::get_param_value $drv_handle $arg]
           if {[llength $value] == 0 || [string compare -nocase "DEVICE_ID" $arg] == 0} {
            set local_value [::hsi::utils::get_param_value $periph $arg]
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
    foreach periph_inst  $periphs {
	set periph [common::get_property IP_NAME $periph_inst]
	if {([string compare -nocase $proctype "ps7_cortexa9"] == 0)|| ([string compare -nocase $proctype "psu_cortexa72"] == 0)||
	     (([string compare -nocase $proctype "psv_cortexa72"] == 0) && ([string compare -nocase $periph "psv_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psv_cortexr5"] == 0) && ([string compare -nocase $periph "psv_rcpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psxl_cortexa78"] == 0) && ([string compare -nocase $periph "psxl_acpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psxl_cortexr52"] == 0) && ([string compare -nocase $periph "psxl_rcpu_gic"] == 0)) ||
	     (([string compare -nocase $proctype "psx_cortexa78"] == 0) && ([string compare -nocase $periph "psx_acpu_gic"] == 0)) ||
             (([string compare -nocase $proctype "psx_cortexr52"] == 0) && ([string compare -nocase $periph "psx_rcpu_gic"] == 0)) ||
	   (([string compare -nocase $proctype "psu_cortexa53"] == 0)&&([string compare -nocase $periph "psu_acpu_gic"] == 0))||
	   (([string compare -nocase $proctype "psu_cortexr5"] == 0)&&([string compare -nocase $periph "psu_rcpu_gic"] == 0))} {
		lappend newperiphs $periph_inst
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
        set source_ports [get_interrupt_sources $periph]
        set i 0
        lappend source_list
        foreach source_port $source_ports {

            if {[llength $source_port] ==0 } {
                continue
            }
            set portType [common::get_property TYPE $source_port]
            if { ([string compare -nocase $portType "INTERRUPT"]) && ([string compare -nocase $source_port "gpio_io_o"] != 0) } {
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
		if { ($is_ip_port_detected == 1) && ([string compare -nocase $source_port_name_temp "gpio_io_o"] != 0) } {
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
            if {([string compare -nocase $proctype "psu_cortexa53"] == 0) || ([string compare -nocase $proctype "psu_cortexr5"] == 0) || ([string compare -nocase $proctype "psv_cortexa72"] == 0) || ([string compare -nocase $proctype "psv_cortexr5"] == 0) || ([string compare -nocase $proctype "psxl_cortexa78"] == 0) || ([string compare -nocase $proctype "psxl_cortexr52"] == 0) || ([string compare -nocase $proctype "psx_cortexa78"] == 0) || ([string compare -nocase $proctype "psx_cortexr52"] == 0)} {
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
                if {([string compare -nocase $proctype "psu_cortexa53"] == 0) || ([string compare -nocase $proctype "psu_cortexr5"] == 0) || ([string compare -nocase $proctype "psv_cortexa72"] == 0) || ([string compare -nocase $proctype "psv_cortexr5"] == 0) || ([string compare -nocase $proctype "psxl_cortexa78"] == 0)  || ([string compare -nocase $proctype "psxl_cortexr52"] == 0) || ([string compare -nocase $proctype "psx_cortexa78"] == 0)  || ([string compare -nocase $proctype "psx_cortexr52"] == 0)} {
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
				#
				# In case of vectored interrupts connected through slice port_intr_id would
				# contain details as given below
				# Index 0 - unique code to identify slice, 32 (If interrupt is coming through slice)
				# Index 1 - First pin number of vectored interrupt source
				#

				if { [lindex $port_intr_id 0] == 32 } {
					set index 2
					set input_pin_num [expr {[lindex $port_intr_id 1] - 32}]
				} else {
                    set index 0
					set input_pin_num 0
				}
                for { } {$index < [llength $port_intr_id]} { } {
					set intr_id [lindex $port_intr_id $index]
                    if { [string compare -nocase $ip_name ""] } {
                            puts $config_inc [format "#define XPAR_FABRIC_%s_%s_%d_INTR %d$uSuffix" \
                            [string toupper $ip_name] [string toupper $port_name] $input_pin_num $intr_id ]
                    } else {
                            puts $config_inc [format "#define XPAR_FABRIC_%s_%d_INTR %d$uSuffix" \
                            [string toupper $port_name] $input_pin_num $intr_id  ]
                    }
                    incr input_pin_num
                    incr index
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
						if { [string compare -nocase $source_port_name($i) "gpio_io_o"] == 0 } {
							continue
						}
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
# Get interrupt ID for VERSAL NET pl-ps interrupts
#
###################################################################
proc get_psx_interrupt_id { sink_pin } {
    if {[string compare -nocase "$sink_pin" "pl_psx_irq0"] == 0 } {
        return 104
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq1"] == 0 } {
        return 105
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq2"] == 0 } {
        return 106
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq3"] == 0 } {
        return 107
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq4"] == 0 } {
        return 108
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq5"] == 0 } {
        return 109
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq6"] == 0 } {
        return 110
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq7"] == 0 } {
        return 111
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq8"] == 0 } {
        return 112
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq9"] == 0 } {
        return 113
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq10"] == 0 } {
        return 114
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq11"] == 0 } {
        return 115
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq12"] == 0 } {
        return 116
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq13"] == 0 } {
        return 117
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq14"] == 0 } {
        return 118
    } elseif {[string compare -nocase "$sink_pin" "pl_psx_irq15"] == 0 } {
        return 119
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
    set is_external_port 0
    set check_duplication 0
    set is_pl_ps_irq1 0
    set is_pl_ps_irq0 0
    global pl_ps_irq1
    global pl_ps_irq0
    global or_id
    global or_cnt
    variable traversed_port_name
    variable traversed_ip_name
    variable traversed_ports_count
    set is_versal [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa72" || IP_NAME=="psv_cortexa72"}]
    set is_versal_net [hsi::get_cells -hier -filter {IP_NAME=="psx_cortexa78"}]


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
        set is_external_port 1
    }
    if { ($is_external_port == 1) || ([string compare -nocase "$port_name" "gpio_io_o"] == 0)} {
        set check_duplication 1
    }
    set intc_periph [get_interrupt_parent $ip_name $port_name]

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
    set intc_src_ports [get_interrupt_sources $intc_periph]

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
    if {[llength $is_versal] != 0} {
	set irq0_base 84
        set irq1_base 92
        set intr_list_irq0 [list 84 85 86 87 88 89 90 91]
        set intr_list_irq1 [list 92 93 94 95 96 97 98 99]
    }  elseif {[llength $is_versal_net] != 0} {
        set irq0_base 104
        set irq1_base 112
        set intr_list_irq0 [list 104 105 106 107 108 109 110 111]
        set intr_list_irq1 [list 112 113 114 115 116 117 118 119]
    } else {
	set irq0_base 89
        set irq1_base 104
        set intr_list_irq0 [list 89 90 91 92 93 94 95 96]
        set intr_list_irq1 [list 104 105 106 107 108 109 110 111]
    }
    set sink_pins [::hsi::utils::get_sink_pins $intr_pin]
    if { [llength $sink_pins] == 0 } {
        return
     }

    if { $i > 9} {
        set ret [expr $ret - 8]
    }
    set concat_block 0
	set is_slice 0
	#
	# Parameter names for width of concat block input port
	#
	set concat_in_list [list CONFIG.IN0_WIDTH CONFIG.IN1_WIDTH CONFIG.IN2_WIDTH CONFIG.IN3_WIDTH CONFIG.IN4_WIDTH \
		CONFIG.IN5_WIDTH CONFIG.IN6_WIDTH CONFIG.IN7_WIDTH CONFIG.IN8_WIDTH CONFIG.IN9_WIDTH CONFIG.IN10_WIDTH \
		CONFIG.IN11_WIDTH CONFIG.IN12_WIDTH CONFIG.IN13_WIDTH CONFIG.IN14_WIDTH CONFIG.IN15_WIDTH  CONFIG.IN16_WIDTH \
		CONFIG.IN17_WIDTH CONFIG.IN18_WIDTH CONFIG.IN19_WIDTH CONFIG.IN120_WIDTH CONFIG.IN21_WIDTH CONFIG.IN22_WIDTH \
		CONFIG.IN23_WIDTH CONFIG.IN24_WIDTH CONFIG.IN25_WIDTH CONFIG.IN26_WIDTH CONFIG.IN27_WIDTH CONFIG.IN28_WIDTH \
		CONFIG.IN29_WIDTH CONFIG.IN30_WIDTH CONFIG.IN1_WIDTH]

	foreach sink_pin $sink_pins {
		set sink_periph [::hsi::get_cells -of_objects $sink_pin]
		if {[llength $sink_periph] == 0} {
			continue
		}
		set level_offset 0
		set traveresing_details 0
		set duplicate 0

		if { $check_duplication == 1 } {
			for {set count 0} {$count < $traversed_ports_count} {incr count} {
				if { ([string compare -nocase $port_name $traversed_port_name($count)] == 0) && ([string compare -nocase $sink_pin $traversed_ip_name($count)] == 0)} {
					set duplicate 1
				}
			}
		}

		if { $duplicate == 1 } {
			continue
		}

		#
		# Scenario where interrupt source is directly connected to PL-PS
		# interrupt port
		#
		if { [string compare -nocase "$sink_pin" "IRQ0_F2P"] == 0 } {
			set is_pl_ps_irq0 1
			if { $check_duplication == 1 } {
				set traversed_ip_name($traversed_ports_count) "IRQ0_F2P"
				set traversed_port_name($traversed_ports_count) $port_name
				set traveresing_details 1
				incr traversed_ports_count
			}
		} elseif {[string compare -nocase "$sink_pin" "IRQ1_F2P"] == 0 } {
			set is_pl_ps_irq1 1
			if { $check_duplication == 1 } {
				set traversed_ip_name($traversed_ports_count) "IRQ1_F2P"
				set traversed_port_name($traversed_ports_count) $port_name
				set traveresing_details 1
				incr traversed_ports_count
			}
		} elseif {[llength $is_versal] != 0 || [llength $is_versal_net] != 0} {
			if {[llength $is_versal] != 0 } {
				set port_intr_id [get_psv_interrupt_id $sink_pin]
			} else {
				set port_intr_id [get_psx_interrupt_id $sink_pin]
			}

			if {[llength $port_intr_id] != 0} {
				if { $check_duplication == 1 } {
					set traversed_ip_name($traversed_ports_count) $sink_pin
					set traversed_port_name($traversed_ports_count) $port_name
					incr traversed_ports_count
					set traveresing_details 1
				}
				return $port_intr_id
			}
		}

	#
	# Interrupt source traversed through concat block/utility reduced logic/slice
	#
	set connected_ip_prev ""
	if { $traveresing_details == 0 } {
		for {set itr 0} {[llength $sink_pins] != 0 && $itr < [llength $sink_pins]} { } {
			set sink_pin [lindex $sink_pins $itr]
			if {[llength $sink_pin] == 0} {
				incr itr
				continue
			}
			set sink_periph [::hsi::get_cells -of_objects $sink_pin]
			set duplicate 0
			if {[llength $sink_periph] == 0} {
				incr itr
				continue
			}
			set connected_ip [get_property IP_NAME [get_cells -hier $sink_periph]]
			if { ($traveresing_details == 0) && ($check_duplication == 1) } {
				for {set count 0} {$count < $traversed_ports_count} {incr count} {
					if { ([string compare -nocase $port_name $traversed_port_name($count)] == 0) && ([string compare -nocase $sink_periph $traversed_ip_name($count)] == 0)} {
						set duplicate 1
					}
				}
			}

			if { $duplicate == 1 } {
				incr itr
				continue
			}

			if { ($traveresing_details == 0) && ($check_duplication == 1)} {
				if { ([string compare -nocase $connected_ip "xlconcat"] == 0) || ([string compare -nocase $connected_ip "util_reduced_logic"] == 0) || ([string compare -nocase $connected_ip "xlslice"] == 0) || ([string compare -nocase $connected_ip "util_vector_logic"] == 0) } {
					set traversed_port_name($traversed_ports_count) $port_name
					set traversed_ip_name($traversed_ports_count) $sink_periph
					set traveresing_details 1
				}
			}

			if { [string compare -nocase "$connected_ip" "xlconcat"] == 0} {
				set sink_pin_temp $sink_pin
				set dout "dout"
				set concat_block 1
				set intr_pin [::hsi::get_pins -of_objects $sink_periph -filter "NAME==$dout"]
				set is_or_gate 0
				set input_pin_num [string trimleft $sink_pin In]
				for {set count 0} {$count < $input_pin_num} {incr count} {
					set temp [get_property [lindex $concat_in_list $count] $sink_periph]
					set level_offset [expr {$level_offset + $temp}]
				}
				set sink_pins [::hsi::utils::get_sink_pins "$intr_pin"]
				set connected_ip_prev $connected_ip
				set itr 0
				continue
			} elseif { [string compare -nocase "$connected_ip" "util_reduced_logic"] == 0} {
				set level_offset 0
				set sink_pin_temp $sink_pin
				set dout "Res"
				set concat_block 0
				set intr_pin [::hsi::get_pins -of_objects $sink_periph -filter "NAME==$dout"]
				set is_or_gate 1

				set sink_pins [::hsi::utils::get_sink_pins "$intr_pin"]
				set connected_ip_prev $connected_ip
				set itr 0
				continue
			} elseif { [string compare -nocase "$connected_ip" "util_vector_logic"] == 0} {
				set level_offset 0
				set sink_pin_temp $sink_pin
				set dout "Res"
				set concat_block 0
				set intr_pin [::hsi::get_pins -of_objects $sink_periph -filter "NAME==$dout"]
				set is_or_gate 1

				set sink_pins [::hsi::utils::get_sink_pins "$intr_pin"]
				set connected_ip_prev $connected_ip
				set itr 0
				continue

			} elseif { [string compare -nocase "$connected_ip" "xlslice"] == 0} {
				set sink_pin_temp $sink_pin
				set dout "Dout"
				set concat_block 0
				set intr_pin [::hsi::get_pins -of_objects $sink_periph -filter "NAME==$dout"]
				set is_or_gate 0
				set is_slice 1

				set dout_width [get_property CONFIG.DOUT_WIDTH $sink_periph]
				set dout_first [get_property CONFIG.DIN_TO $sink_periph]
				set input_present_in_dout 0

				# If slice IP is having concat block on left side
				if { ([string compare -nocase $connected_ip_prev "xlconcat"] == 0)|| ([string compare -nocase $connected_ip_prev "xlslice"] == 0) } {
					set temp $dout_first
					for {set slice_out 0} {$slice_out < $dout_width} {incr slice_out} {
						if { $temp == $level_offset } {
							set level_offset $slice_out
							set input_present_in_dout 1
							set is_slice 0
							break
						}
						incr temp
					}
				} else {
					set input_present_in_dout 1
					set level_offset 0
				}

				if { $input_present_in_dout == 0 } {
					incr itr
					continue
				}
				set sink_pins [::hsi::utils::get_sink_pins "$intr_pin"]
				set connected_ip_prev $connected_ip
				set itr 0
				continue
			} elseif { [string compare -nocase "$sink_pin" "IRQ0_F2P"] == 0 } {
				set is_pl_ps_irq0 1
			} elseif {[string compare -nocase "$sink_pin" "IRQ1_F2P"] == 0 } {
				set is_pl_ps_irq1 1
			} elseif {[llength $is_versal] != 0 || [llength $is_versal_net] != 0} {
				set port_intr_id ""
				if {[llength $is_versal] != 0 } {
					set port_intr_id_temp [get_psv_interrupt_id $sink_pin]
				} else {
					set port_intr_id_temp [get_psx_interrupt_id $sink_pin]
				}
				if {[llength $port_intr_id_temp] != 0} {
					if { $traveresing_details == 1 } {
						incr traversed_ports_count
					}

					if { $is_slice == 1 } {
						lappend port_intr_id 0
						lappend port_intr_id $dout_first
					}
					lappend port_intr_id $port_intr_id_temp
					return $port_intr_id
				}
			}

			if { $is_pl_ps_irq0 == 1 || $is_pl_ps_irq1 == 1 } {
				incr itr
				for { } {$itr < [llength $sink_pins]} {incr itr} {
					set sink_pin2 [lindex $sink_pins $itr]
					if { $is_pl_ps_irq0 == 1 } {
						if {[string compare -nocase "$sink_pin2" "IRQ1_F2P"] == 0 } {
							set is_pl_ps_irq1 1
						}
					} else {
						if {[string compare -nocase "$sink_pin2" "IRQ0_F2P"] == 0 } {
							set is_pl_ps_irq0 1
						}
					}
				}
				if { $traveresing_details == 1 } {
					incr traversed_ports_count
				}
				break
			}
			incr itr
		}
	}

	set number level_offset

        set result ""
	if { $is_slice == 1} {
		set vector_size $dout_width
		#
		# If vectored interrupt source is connected through slice,
		# interrupt names exported to xparameters.h should have
		# input pin numbers. To achieve that, xdefine_gic_params
		# needs to identify the vectored interrupt source with slice
		# connectivity. Following details are added to index 0 and
		# index 1 of result, so that xdefine_gic_params can identify
		# vectored interrupts connected through slice.
		# Index 0 : unique value 0
		# Index 1: Pin number from where output of slice would start (DIN parameter)
		#
		lappend result 0
		lappend result $dout_first
	} elseif {[llength [hsi::get_ports $port_name]] != 0 && [common::get_property LEFT [hsi::get_ports $port_name]] != ""} {
	    set vector_size [common::get_property LEFT [hsi::get_ports $port_name]]
	    set vector_size [expr {$vector_size + 1}]
	} else {
	    set vector_size 1
	}

	for { set vec 0 } {$vec < $vector_size} {incr vec} {
	# generate irq id for IRQ1_F2P
            if {$is_pl_ps_irq1 == 1} {
                if {$found == 1} {
                    set irqval $pl_ps_irq1
                    if {$concat_block == "0"} {
                        set ret [ expr {[lindex $intr_list_irq1 $irqval] + $vec}]
						lappend result $ret
                    } else {
                        set ret [expr {$irq1_base + $level_offset + $vec}]
                        lappend result $ret
                    }
	        }
            }
            if {$is_pl_ps_irq0 == 1} {
	        # generate irq id for IRQ0_F2P
                if {$found == 1} {
                    set irqval $pl_ps_irq0
		    if {$concat_block == "0"} {
                        set ret [ expr {[lindex $intr_list_irq0 $irqval] + $vec}]
						lappend result $ret
                    } else {
                        set ret [expr {$irq0_base + $level_offset + $vec}]
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
		} elseif { ([string match -nocase $IP_NAME "psu_acpu_gic"]) || ([string match -nocase $IP_NAME "psv_acpu_gic"]) || ([string match -nocase $IP_NAME "psxl_acpu_gic"]) || ([string match -nocase $IP_NAME "psx_acpu_gic"])} {
						return true
`		} elseif { ([string match -nocase $IP_NAME "psu_rcpu_gic"]) || ([string match -nocase $IP_NAME "psv_rcpu_gic"]) || ([string match -nocase $IP_NAME "psxl_rcpu_gic"]) || ([string match -nocase $IP_NAME "psx_rcpu_gic"])} {
						return true
		}
		#puts "return $IP_NAME\n\r"
		return false;
}


#
# Get handles for all ports driving the interrupt pin of a peripheral
#
proc get_interrupt_sources {periph_handle } {
   lappend interrupt_sources
   lappend interrupt_pins
   set interrupt_pins [::hsi::get_pins -of_objects $periph_handle -filter {TYPE==INTERRUPT && DIRECTION==I}]
   foreach interrupt_pin $interrupt_pins {
       set source_pins [get_intr_src_pins $interrupt_pin]
       foreach source_pin $source_pins {
           lappend interrupt_sources $source_pin
       }
   }
   return $interrupt_sources
}

#
# Get the interrupt source pins of a periph pin object
#
proc get_intr_src_pins {interrupt_pin} {
    lappend interrupt_sources
    set source_pins [::hsi::utils::get_source_pins $interrupt_pin]
    foreach source_pin $source_pins {
        set source_cell [::hsi::get_cells -of_objects $source_pin]
        if { [llength $source_cell ] } {
            #For concat IP, we need to bring pin source for other end
            set ip_name [common::get_property IP_NAME $source_cell]
            if { [string match -nocase $ip_name "xlconcat" ] } {
                set interrupt_sources [list {*}$interrupt_sources {*}[get_concat_interrupt_sources $source_cell]]
            } elseif { [string match -nocase $ip_name "xlslice"] } {
                set interrupt_sources [list {*}$interrupt_sources {*}[::hsi::__internal::get_slice_interrupt_sources $source_cell]]
            } elseif { [string match -nocase $ip_name "util_reduced_logic"] } {
                set interrupt_sources [list {*}$interrupt_sources {*}[::hsi::__internal::get_util_reduced_logic_interrupt_sources $source_cell]]
            } elseif { [string match -nocase $ip_name "util_vector_logic"] } {
                set interrupt_sources [list {*}$interrupt_sources {*}[get_util_vector_logic_interrupt_sources $source_cell]]
            } else {
                lappend interrupt_sources $source_pin
            }
        } else {
            lappend interrupt_sources $source_pin
        }
    }
    return $interrupt_sources
}

#It assume that XLCONCAT IP cell object is passed to this function
proc get_concat_interrupt_sources { concat_ip_obj {lsb -1} {msb -1} } {
    lappend source_pins
    if {$lsb == -1 } {
        set i 0
        set num_ports [common::get_property CONFIG.NUM_PORTS $concat_ip_obj]
    } else {
        set i $lsb
        set num_ports $msb
    }
    for { $i } { $i < $num_ports } { incr i } {
        set in_pin [::hsi::get_pins -of_objects $concat_ip_obj "In$i"]
        set pins [::hsi::utils::get_source_pins $in_pin]
        foreach pin $pins {
            set source_cell [::hsi::get_cells -of_objects $pin]
            if { [llength $source_cell] } {
                set ip_name [common::get_property IP_NAME $source_cell]
                #Cascading case of concat IP
                if { [string match -nocase $ip_name "xlconcat"] } {
                    set source_pins [list {*}$source_pins {*}[get_concat_interrupt_sources $source_cell]]
                } elseif { [string match -nocase $ip_name "xlslice"] } {
                    set source_pins [list {*}$source_pins {*}[::hsi::__internal::get_slice_interrupt_sources $source_cell]]
                } elseif { [string match -nocase $ip_name "util_reduced_logic"] } {
                    set source_pins [list {*}$source_pins {*}[::hsi::__internal::get_util_reduced_logic_interrupt_sources $source_cell]]
                } elseif { [string match -nocase $ip_name "util_vector_logic"] } {
                    set source_pins [list {*}$source_pins {*}[get_util_vector_logic_interrupt_sources $source_cell]]
                } else {
                    lappend source_pins $pin
                }

            } else {
                lappend source_pins $pin
            }
        }
    }
    return $source_pins
}

proc get_util_vector_logic_interrupt_sources { url_ip_obj } {
    lappend source_pins
    set in_pin [::hsi::get_pins -of_objects $url_ip_obj "Op1"]
    set pins [::hsi::utils::get_source_pins $in_pin]
    foreach pin $pins {
        set source_cell [::hsi::get_cells -of_objects $pin]
        if { [llength $source_cell] } {
            set ip_name [common::get_property IP_NAME $source_cell]
            if { [string match -nocase $ip_name "xlslice"] } {
                set source_pins [list {*}$source_pins {*}[::hsi::__internal::get_slice_interrupt_sources $source_cell]]
            } elseif { [string match -nocase $ip_name "xlconcat"] } {
                set source_pins [list {*}$source_pins {*}[::hsi::__internal::get_concat_interrupt_sources $source_cell]]
            } elseif { [string match -nocase $ip_name "util_reduced_logic"] } {
		    #Cascading case of util_reduced_logic IP
                    set source_pins [list {*}$source_pins {*}[::hsi::__internal::get_util_reduced_logic_interrupt_sources $source_cell]]
            } elseif { [string match -nocase $ip_name "util_vector_logic"] } {
		    #Cascading case of util_reduced_logic IP
                    set source_pins [list {*}$source_pins {*}[get_util_vector_logic_interrupt_sources $source_cell]]
            } else {
                lappend source_pins $pin
            }

        } else {
            lappend source_pins $pin
        }
    }
    return $source_pins
}

#It gets connected interrupt controller
proc get_interrupt_parent {  ip_name port_name } {
    set intc [get_connected_intr_cntrl $ip_name $port_name]
    return $intc
}

#
# It needs IP name and interrupt port name and it will return the connected
# interrupt controller
# for External interrupt port, IP name should be empty
#
proc get_connected_intr_cntrl { periph_name intr_pin_name } {
    lappend intr_cntrl
    if { [llength $intr_pin_name] == 0 } {
        return $intr_cntrl
    }
    if { [llength $periph_name] != 0 } {
        #This is the case where IP pin is interrupting
        set periph [::hsi::get_cells -hier -filter "NAME==$periph_name"]
        if { [llength $periph] == 0 } {
            return $intr_cntrl
        }
        set intr_pin [::hsi::get_pins -of_objects $periph -filter "NAME==$intr_pin_name"]
        if { [llength $intr_pin] == 0 } {
            return $intr_cntrl
        }
        set pin_dir [common::get_property DIRECTION $intr_pin]
        if { [string match -nocase $pin_dir "I"] } {
          return $intr_cntrl
        }
    } else {
        #This is the case where External interrupt port is interrupting
        set intr_pin [::hsi::get_ports $intr_pin_name]
        if { [llength $intr_pin] == 0 } {
            return $intr_cntrl
        }
        set pin_dir [common::get_property DIRECTION $intr_pin]
        if { [string match -nocase $pin_dir "O"] } {
          return $intr_cntrl
        }
    }
    set intr_sink_pins [::hsi::utils::get_sink_pins $intr_pin]
    foreach intr_sink $intr_sink_pins {
        #changes made to fix CR 933826
        set sink_periph [lindex [::hsi::get_cells -of_objects $intr_sink] 0]
        if { [llength $sink_periph ] && [::hsi::utils::is_intr_cntrl $sink_periph] == 1 } {
            lappend intr_cntrl $sink_periph
        } elseif { [llength $sink_periph] && [string match -nocase [common::get_property IP_NAME $sink_periph] "xlconcat"] } {
            #this the case where interrupt port is connected to XLConcat IP.
            #changes made to fix CR 933826
            set intr_cntrl [list {*}$intr_cntrl {*}[::hsi::utils::get_connected_intr_cntrl $sink_periph "dout"]]
        } elseif { [llength $sink_periph] && [string match -nocase [common::get_property IP_NAME $sink_periph] "xlslice"] } {
            set intr_cntrl [list {*}$intr_cntrl {*}[::hsi::utils::get_connected_intr_cntrl $sink_periph "Dout"]]
        } elseif { [llength $sink_periph] && [string match -nocase [common::get_property IP_NAME $sink_periph] "util_reduced_logic"] } {
            set intr_cntrl [list {*}$intr_cntrl {*}[::hsi::utils::get_connected_intr_cntrl $sink_periph "Res"]]
        } elseif { [llength $sink_periph] && [string match -nocase [common::get_property IP_NAME $sink_periph] "util_vector_logic"] } {
            set intr_cntrl [list {*}$intr_cntrl {*}[::hsi::utils::get_connected_intr_cntrl $sink_periph "Res"]]
        }
    }
    return $intr_cntrl
}

proc generate_ipdefine {drv_handle file_name} {
    set file_handle [::hsi::utils::open_include_file $file_name]
    puts $file_handle "\#define XPAR_SCUGIC"
    close $file_handle
}
