###############################################################################
# Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 2.0      adk    12/10/13 Updated as per the New Tcl API's
# 2.0      bss    05/02/14 Modified to generate PITx_EXPIRED_MASK parameter 
#			      to fix CR#794167.
# 2.2	   nsk	  21/07/15 Updated iomodule_define_vector_table by removing
#		  	   absoluted hsi commands like xget_handle.CR#865544.
#			   Modified generate proc to get canonical definitions
#			   in xparameters.h.
# 2.2	   nsk	  07/08/15 Updated iomodule_define_vector_table to handle
#		 	   External vector interrupts CR#871572.
# 2.2	   nsk    19/08/15 Modified iomodule_defince_vector_table
#			   to handle, if iomodule doesn't have interrupts
#			   enabled CR#876507.
# 2.2	   nsk    02/09/15 Modified iomodule_define_vector_table
#			   when no external interrupts are used.CR#878782.
# 2.3      nsk    05/11/15 Updated xdefine_canonical_xpars such that
#                          Generate canonical definitions, whose canonical
#                          name is not the same as hardware instance name.
#                          CR #876604.
#          sk     11/09/15 Removed delete filename statement CR# 784758.
# 2.5      ms     04/18/17 Modified tcl file to add suffix U for all macros
#                          definitions of iomodule in xparameters.h
# 2.6      mus    09/25/18 Updated tcl to replace "hsi::get_cells -of_object"
#                          with the "hsi::get_cells -of_objects". CR#1011395.
# 2.10     nsk    12/14/20 Modified the tcl to not to use the instance names.
# 2.12     mus    07/06/21 Updated is_psmicroblaze_iomodule proc to support
#                          SSIT devices. Also added check for IS_PL flag
#                          to avoid incorrect settings, in case PL iomodule
#                          instance base address matches with PSM/PMC iomodule.
# 2.12     mus    08/24/21 Updated xredefine_iomodule with additional checks,
#                          to avoid tcl errors for HW design where interrupts
#                          are not connected to iomodule. It fixes CR#1108543
##############################################################################


############################################################
# Global interrupt handlers array, default handler routine
############################################################
array set interrupt_handlers ""
set default_interrupt_handler "XNullHandler"


############################################################
# DRC procedure
############################################################
proc iomodule_drc {drv_handle} {

}


############################################################
# "generate" procedure
############################################################
proc generate {drv_handle} {
    # Generate the following definitions in xparameters.h
    # 1. Common
    set common_params [list "NUM_INSTANCES" "DEVICE_ID" \
                            "C_BASEADDR" "C_HIGHADDR" "C_MASK" "C_FREQ"]

    # 2. UART
    set uart_params [list "C_USE_UART_RX" "C_USE_UART_TX" "C_UART_BAUDRATE" \
                          "C_UART_PROG_BAUDRATE" "C_UART_DATA_BITS" "C_UART_USE_PARITY" \
                          "C_UART_ODD_PARITY" "C_UART_RX_INTERRUPT" \
                          "C_UART_TX_INTERRUPT" "C_UART_ERROR_INTERRUPT"]

    # 3. FIT
    foreach i {1 2 3 4} {
      set fit${i}_params [list "C_USE_FIT${i}" "C_FIT${i}_No_CLOCKS" "C_FIT${i}_INTERRUPT"]
    }

    # 4. PIT
    foreach i {1 2 3 4} {
      set pit${i}_params [list "C_USE_PIT${i}" "C_PIT${i}_SIZE" "C_PIT${i}_EXPIRED_MASK" \
      			"C_PIT${i}_READABLE" "C_PIT${i}_PRESCALER" "C_PIT${i}_INTERRUPT"]
    }

    # 5. GPO
    foreach i {1 2 3 4} {
      set gpo${i}_params [list "C_USE_GPO${i}" "C_GPO${i}_SIZE"]
    }

    # 6. GPI
    foreach i {1 2 3 4} {
      set gpi${i}_params [list "C_USE_GPI${i}" "C_GPI${i}_SIZE" "C_GPI${i}_INTERRUPT"]
    }

    # 7. INTC
    set intc_params [list "C_INTC_USE_EXT_INTR" "C_INTC_INTR_SIZE" "C_INTC_HAS_FAST" "C_INTC_BASE_VECTORS" "C_INTC_ADDR_WIDTH "]

    # 8. IO BUS
    set io_params [list "C_USE_IO_BUS" "C_IO_BASEADDR" "C_IO_HIGHADDR" "C_IO_MASK"]

    set all_params [concat $common_params $uart_params \
                           $fit1_params $fit2_params $fit3_params $fit4_params \
                           $pit1_params $pit2_params $pit3_params $pit4_params \
                           $gpo1_params $gpo2_params $gpo3_params $gpo4_params \
                           $gpi1_params $gpi2_params $gpi3_params $gpi4_params \
                           $intc_params $io_params]

    eval [xdefine_include_file $drv_handle "xparameters.h" "XIOModule" $all_params]
    eval [::hsi::utils::define_config_file  $drv_handle "xiomodule_g.c" "XIOModule" $all_params]
    
    # Generate the following definitions as hexadecimal values in xparameters.h
    # 5. GPO
    foreach i {1 2 3 4} {
      set gpo${i}_params [list "C_GPO${i}_INIT"]
    }

    # 7. INTC
    set intc_params [list "C_INTC_LEVEL_EDGE" "C_INTC_POSITIVE"]

    set params [concat  $gpo1_params $gpo2_params $gpo3_params $gpo4_params $intc_params]

    eval [xdefine_include_file_hex $drv_handle "xparameters.h" "XIOModule" $params]

    # 7. INTC: Set XPAR_IOMODULE_INTC_MAX_INTR_SIZE
    set max_intr_size 0
    set periphs [hsi::utils::get_common_driver_ips $drv_handle]
    foreach periph $periphs {
      set periph_num_intr_internal [get_num_intr_internal $periph]
      set periph_num_intr_inputs [get_num_intr_inputs $periph]
      set periph_intr_size [expr $periph_num_intr_internal + $periph_num_intr_inputs]
      if {$max_intr_size < $periph_intr_size} {
        set max_intr_size $periph_intr_size
      }
    }

    # 7. INTC: Define XPAR_SINGLE_BASEADDR, XPAR_SINGLE_HIGHADDR, and XPAR_SINGLE_DEVICE_ID
    set uSuffix "U"
    set periphs [hsi::utils::get_common_driver_ips $drv_handle]
    set count [llength $periphs]
    if {$count == 1} {
      hsi::utils::define_with_names $drv_handle [hsi::utils::get_common_driver_ips $drv_handle] "xparameters.h" \
         "XPAR_IOMODULE_SINGLE_BASEADDR" "C_BASEADDR" \
         "XPAR_IOMODULE_SINGLE_HIGHADDR" "C_HIGHADDR" \
         "XPAR_IOMODULE_INTC_SINGLE_DEVICE_ID" "DEVICE_ID"
	 }

    set config_inc [hsi::utils::open_include_file "xparameters.h"]
    puts $config_inc "#define XPAR_IOMODULE_INTC_MAX_INTR_SIZE $max_intr_size$uSuffix"
    # 7. INTC: Generate config table, vector tables
    iomodule_define_config_file $drv_handle $periphs $config_inc
    close $config_inc

    set all_params [concat $all_params $params]
    xdefine_canonical_xpars $drv_handle "xparameters.h" "IOModule" $all_params

}

##########################################################################
# Generate interrupt definitions in Configuration C file xiomodule_g.c
# This file has the Config Table and vector tables for each iomodule 
# instance as required by Xilinx iomodule driver
##########################################################################
proc iomodule_define_config_file {drv_handle periphs config_inc} {

    variable interrupt_handlers
    variable default_interrupt_handler

    # set isr_options to be XIN_SVC_SGL_ISR_OPTION as defined in xiomodule.h
    set isr_options XIN_SVC_SGL_ISR_OPTION
    set file_name "xiomodule_g.c"
    set drv_string "XIOModule"
    set args [list "DEVICE_ID" "C_BASEADDR" "C_IO_BASEADDR" "C_INTC_HAS_FAST" "C_INTC_BASE_VECTORS" "C_INTC_ADDR_WIDTH "]
    set filename [file join "src" $file_name]
    set config_file [open $filename w]
    hsi::utils::write_c_header $config_file "Driver configuration"
    puts $config_file "#include \"xparameters.h\""
    puts $config_file "#include \"[string tolower $drv_string].h\""
    puts $config_file "\n"

    set tmp_filename [file join "src" "tmpconfig.c"]
    set tmp_config_file [open $tmp_filename w]

    puts $tmp_config_file "\n/*"
    puts $tmp_config_file "* The configuration table for devices"
    puts $tmp_config_file "*/\n"
    puts $tmp_config_file [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
    puts $tmp_config_file "\{"

    set start_comma ""
    foreach periph $periphs {
        puts $tmp_config_file [format "%s\t\{" $start_comma]
        set comma ""
        foreach arg $args {
            # Check if this is a driver parameter or a peripheral parameter
            set value [common::get_property CONFIG.$arg $drv_handle]
            if {[llength $value] == 0} {
                puts -nonewline $tmp_config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
            } else {
                puts -nonewline $tmp_config_file [format "%s\t\t%s" $comma [hsi::utils::get_driver_param_name $drv_string $arg]]
            }
            set comma ",\n"
        }

        # add AckBeforeService as an arg to the config table - Ack Before for edge interrupts
        puts -nonewline $tmp_config_file [format "%s\t\t%s" $comma "(([::hsi::utils::get_ip_param_name $periph C_INTC_LEVEL_EDGE] << 16) | 0x7FF)"]

        # add the OPTIONS as an arg to the config table - default OPTIONS value is XIN_SVC_SGL_ISR_OPTION
        puts -nonewline $tmp_config_file [format "%s\t\t%s" $comma $isr_options]

        # add the FREQ as an arg to the config table
        puts -nonewline $tmp_config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph "C_FREQ"] ]

        # add the BAUDRATE as an arg to the config table
        puts -nonewline $tmp_config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph "C_UART_BAUDRATE"] ]

        # add the PIT use as an arg to the config table
        puts $tmp_config_file [format "%s\t\t\{" $comma ]
        for {set i 1} {$i <= 4} {incr i} {
            puts $tmp_config_file [format "\t\t\t%s," [::hsi::utils::get_ip_param_name $periph "C_USE_PIT${i}"] ]
        }
        puts -nonewline $tmp_config_file "\t\t\}"

        # add the PIT sizes as an arg to the config table
        puts $tmp_config_file [format "%s\t\t\{" $comma ]
        for {set i 1} {$i <= 4} {incr i} {
            puts $tmp_config_file [format "\t\t\t%s," [::hsi::utils::get_ip_param_name $periph "C_PIT${i}_SIZE"] ]
        }
        puts -nonewline $tmp_config_file "\t\t\}"

 	# add the PIT sizes as an arg to the config table
        puts $tmp_config_file [format "%s\t\t\{" $comma ]
        for {set i 1} {$i <= 4} {incr i} {
            puts $tmp_config_file [format "\t\t\t%s," [::hsi::utils::get_ip_param_name $periph "C_PIT${i}_EXPIRED_MASK"] ]
        }
        puts -nonewline $tmp_config_file "\t\t\}"

        # add the PIT prescalers as an arg to the config table
        puts $tmp_config_file [format "%s\t\t\{" $comma ]
        for {set i 1} {$i <= 4} {incr i} {
            puts $tmp_config_file [format "\t\t\t%s," [::hsi::utils::get_ip_param_name $periph "C_PIT${i}_PRESCALER"] ]
        }
        puts -nonewline $tmp_config_file "\t\t\}"

        # add if PIT has readable counter as an arg to the config table
        puts $tmp_config_file [format "%s\t\t\{" $comma ]
        for {set i 1} {$i <= 4} {incr i} {
            puts $tmp_config_file [format "\t\t\t%s," [::hsi::utils::get_ip_param_name $periph "C_PIT${i}_READABLE"] ]
        }
        puts -nonewline $tmp_config_file "\t\t\}"

        # add the GPO initialization values as an arg to the config table
        puts $tmp_config_file [format "%s\t\t\{" $comma ]
        for {set i 1} {$i <= 4} {incr i} {
            puts $tmp_config_file [format "\t\t\t%s," [::hsi::utils::get_ip_param_name $periph "C_GPO${i}_INIT"] ]
        }
        puts -nonewline $tmp_config_file "\t\t\}"

        # generate the vector table for this iomodule instance
        iomodule_define_vector_table $periph $config_inc $tmp_config_file

        puts $config_inc "\n/******************************************************************/\n"

        puts -nonewline $tmp_config_file "\n\t\}"
        set start_comma ",\n"
    }
    puts $tmp_config_file "\n\};"
    close $tmp_config_file

    # Write out the extern definitions of handlers...
    foreach elem [array names interrupt_handlers] {
        puts $config_file [format "extern void %s (void *);" $elem ]
    }

    # copy over the tmp_config_file contents to config_file
    set tmp_config_file [open $tmp_filename r]
    while {![eof $tmp_config_file]} {
        gets $tmp_config_file line
    puts $config_file $line
    }
    close $tmp_config_file
    file delete -force $tmp_filename

    close $config_file
}


##########################################################################
# Define the vector table
##########################################################################
proc iomodule_define_vector_table {periph config_inc config_file} {

    set uSuffix "U"
    variable interrupt_handlers
    variable default_interrupt_handler
    variable source_port_name
    variable source_name
    variable source_port_type
    variable source_driver
    variable source_interrupt_handler
    variable source_interrupt_id
    variable total_source_intrs

    #update global array of Interrupt sources for this periph
    intc_update_source_array $periph
    set periph_name [common::get_property NAME $periph]
    set interrupt_pin [hsi::get_pins -of_objects [get_cells -hier $periph] -filter {TYPE==INTERRUPT&&DIRECTION==I}]

    # Get pins/ports that are driving the interrupt
    set c_intc_use_ext_intr [common::get_property CONFIG.C_INTC_USE_EXT_INTR $periph]
    if {$c_intc_use_ext_intr == 0} {
	set num_intr_inputs 0
    } else {
	set num_intr_inputs [common::get_property CONFIG.C_INTC_INTR_SIZE $periph]
    }

    #calculate the total interrupt sources
    set total_intr_ports [get_num_intr_internal $periph]
    if {$num_intr_inputs != $total_intr_ports} {
       set source_pins [::hsi::utils::get_interrupt_sources $periph]
       puts "WARNING: Num intr inputs $num_intr_inputs not the \
	     same as length of ::hsi::utils::get_interrupt_sources [llength \
		 $source_pins] hsi_warning"
    }

    #Check if default_interrupt_handler has to have an extern definition
    if {[array size interrupt_handlers] < $total_source_intrs } {
        intc_add_handler $default_interrupt_handler
    }

    puts -nonewline $config_file ",\n\t\t\{"
    set comma "\n"

    for {set i 0} {$i < $total_source_intrs} {incr i} {
        set source_ip $source_name($i)
        if { [llength $source_ip] != 0 && [llength [hsi::get_cells -hier $source_ip]] \
	   != 0} {
           set ip_name [common::get_property IP_NAME [hsi::get_cells -hier $source_ip]]
           if { [string compare -nocase $ip_name "xlconstant"] == 0 } {
              #do no generate interrupt handler entries for xlconstant
              continue
           }
        }
        puts $config_file [format "%s\t\t\t\{" $comma ]
        puts $config_file [format "\t\t\t\t%s," $source_interrupt_handler($i) ]
        if {[llength $source_name($i)] == 0} {
            puts $config_file "\t\t\t\t(void *)XNULL"
        } else {
            set sname [string toupper $source_name($i)]
            set source_xparam_name [::hsi::utils::format_xparam_name $sname]
            set pname [string toupper $periph_name]
            set periph_xparam_name [::hsi::utils::format_xparam_name $pname]
            puts $config_inc [format "#define XPAR_%s_%s_%s_MASK %#08X$uSuffix" \
		[string toupper $periph_name] $source_xparam_name [string \
		toupper $source_port_name($i)] [expr 1 << $i]]
            puts $config_inc [format "#define XPAR_%s_%s_%s_INTR %d$uSuffix" [string \
		toupper $periph_name] [string toupper $source_name($i)] [string \
		 toupper $source_port_name($i)] $i]

            if {[string compare -nocase "global" $source_port_type($i) ] != 0 && \
                [string compare $source_interrupt_handler($i) $default_interrupt_handler ] != 0} {
                puts $config_file [format "\t\t\t\t(void *) %s" \
			$source_handler_arg($i)]
            } else {
                puts $config_file "\t\t\t\t(void *) XNULL"
            }
        }
        puts -nonewline $config_file "\t\t\t\}"
        set comma ",\n"
    }
    puts $config_file "\n\t\t\}"

}

##########################################################################
# This procedure creates a unique list of
# handlers that needs to have an extern defn.
# in xparameters.h
##########################################################################
proc iomodule_add_handler {handler} {

    variable interrupt_handlers

    set interrupt_handlers($handler) 1
}


##########################################################################
# Given a list of arguments, define each as a canonical constant name,
# using the driver name, in an include file.
##########################################################################
proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
    set args [hsi::utils::get_exact_arg_list $args]
    # Open include file
    set file_handle [hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [hsi::utils::get_common_driver_ips $drv_handle]

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

    # Print canonical parameters for each peripheral
    set device_id 0
    foreach periph $periphs {
        puts $file_handle ""
        set periph_name [string toupper [common::get_property NAME $periph]]
	if { [lsearch $canonicals $periph_name] < 0 } {
		set canonical_name [format "%s_%s" $drv_string [lindex $indices $device_id]]
		puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
		foreach arg $args {
			if {[string first "_EXPIRED_MASK" $arg] > 0} {
				set charindex [string first "_EXPIRED_MASK" $arg]
				set size [string index $arg [expr $charindex - 1]]
				set sizearg [format "C_PIT%d_SIZE" $size]
				set lvalue [format "C_PIT%d_EXPIRED_MASK" $size]
				set lvalue [hsi::utils::get_driver_param_name $canonical_name $lvalue]
				set rvalue [common::get_property CONFIG.$sizearg $periph]
				set rvalue [expr pow(2, $rvalue) - 1]
				set rvalue [format "%.0f" $rvalue]
				set rvalue [format "0x%08X" $rvalue]
			} else {
				set lvalue [hsi::utils::get_driver_param_name $canonical_name [string toupper $arg]]
				# The commented out rvalue is the name of the instance-specific constant
				# set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
				# The rvalue set below is the actual value of the parameter
				set rvalue [common::get_property CONFIG.$arg  $periph]
				}
			if {[llength $rvalue] == 0} {
				set rvalue 0
			}
			set rvalue [hsi::utils::format_addr_string $rvalue $arg]
			set uSuffix [xdefine_getSuffix $lvalue $rvalue]
			puts $file_handle "#define $lvalue $rvalue$uSuffix"
		}
	incr device_id
	puts $file_handle ""
	}
     }

    #
    # Now redefine the Interrupt ID constants
    #
    set redef_value [is_psmicroblaze_iomodule $drv_handle]
    if {$redef_value == 0} {
	     xredefine_iomodule $drv_handle $file_handle
    }


    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

##########################################################################
# iomodule redefines
##########################################################################
proc xredefine_iomodule {drvhandle config_inc} {

    variable default_interrupt_handler

    # Next define interrupt IDs for each connected peripheral

    set periphs [hsi::utils::get_common_driver_ips $drvhandle]
    set device_id 0
    set periph_name [string toupper "iomodule"]

    foreach periph $periphs {

        # Get the edk based name of peripheral for printing redefines
        set edk_periph_name [common::get_property NAME $periph]

        # Get ports that are driving the interrupt
        set source_ports [hsi::utils::get_interrupt_sources $periph]
	set i 0
	lappend source_list
        foreach source_pin $source_ports {
        set source_periph($i) [hsi::get_cells -of_objects $source_pin ]
        if { [llength $source_periph($i) ] == 0} {
            #external interrupt port case
            set width [hsi::utils::get_port_width $source_pin]
            for { set j 0 } { $j < $width } { incr j } {
                set source_port_name($i) "[common::get_property NAME $source_pin]_$j"
                set source_name($i) "system"
                set port_type($i) "global"
                set source_driver ""
                set source_interrupt_handler($i) $default_interrupt_handler
		set source_periph($i) ""
		lappend source_list $source_name($i)
                incr i
            }
        } else {
            #peripheral interrupt case
            set port_type($i) "local"
            set source_name($i) [common::get_property NAME $source_periph($i)]
            set source_port_name($i) [common::get_property NAME $source_pin]
            set source_driver [hsi::get_drivers -filter "HW_INSTANCE==$source_periph($i)"]
    	    set source_interrupt_handler($i) $default_interrupt_handler
	    lappend source_list $source_name($i)
            incr i
        }
     }

        set num_intr_inputs [get_num_intr_inputs $periph]
        for {set i 0} {$i < $num_intr_inputs} {incr i} {

            # Skip global (external) ports
            if {[info exist source_periph($i)] == 0 || $source_periph($i) == ""} {
                continue
            }
            set port_type($i) [common::get_property TYPE $source_periph($i)]
            if {[string compare -nocase $port_type($i) "global"] == 0} {
                continue
            }

            set drv [hsi::get_drivers -filter "HW_INSTANCE==$source_name($i)"]
            set iptype [common::get_property IPTYPE $source_periph($i)]

#           if {[llength $source_name($i)] != 0 && [llength $drv] != 0 && 
#               [string compare -nocase $iptype "PERIPHERAL"] == 0}
            if {[llength $source_name($i)] != 0 && [llength $drv] != 0} {

                set instance [xfind_instance $drv $source_name($i)]
                set drvname [common::get_property NAME $drv]

                #
                # Handle reference cores, which have non-reference driver names
                #
                if {[string compare -nocase $drvname "touchscreen_ref"] == 0} {
                    set drvname "touchscreen"
                } elseif {[string compare -nocase $drvname "ps2_ref"] == 0} {
                    set drvname "ps2"
                    set instance [expr $instance*2]
                    if {[string match -nocase  "*SYS_INTR2" $source_port_name($i)] == 1} {
                        incr instance
                    }
                }

                #
                # Define the interrupt vector IDs in xparameters.h for each core
                # that is connected to this iomodule.
                #
                set drvname [string toupper $drvname]

                #
                # Treat sources with multiple interrupt ports slightly different
                # by including the interrupt port name in the canonical constant
                # name
                #
                if { [lcount $source_list $source_name($i)] > 1} {
                    set first_part [format "#define XPAR_%s_%s_%s_%s_%s_VEC_ID" \
                      $periph_name $device_id $drvname $instance \
                      [string toupper $source_port_name($i)]]
                } else {
                    set first_part [format "#define XPAR_%s_%s_%s_%s_VEC_ID" \
                      $periph_name $device_id $drvname $instance]
                }

                set second_part [format "XPAR_%s_%s_%s_INTR" \
                  [string toupper $edk_periph_name] \
                  [string toupper $source_name($i)] \
                  [string toupper $source_port_name($i)] ]

                if {[string compare -nocase $drvname "generic"] != 0} {
                    puts $config_inc "$first_part $second_part"
                }
            }
        }
        incr device_id
    }
}

##########################################################################
# Get the number of elements in the given list that match the given
# entry.  Assume elements are strings.
##########################################################################
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


##########################################################################
# Get the HW instance number for a particular device. This will be used to
# enumerate the vector ID defines if more than one interrupt from the core
# is connected to the interrupt controller.
##########################################################################
proc xfind_instance {drvhandle instname} {

    set instlist [hsi::utils::get_common_driver_ips $drvhandle]
    set i 0
    foreach inst $instlist {
        set name [common::get_property NAME $inst]
        if {[string compare -nocase $instname $name] == 0} {
            return $i
        }
        incr i
    }
    set i 0
    return $i
}


##########################################################################
# Get the type of port, whether it is "local" (from an IP), or "global"
# (from external source).
##########################################################################
proc xget_port_type {periph} {
    set mhs [hsi::get_cells -of_objects $periph]
    if {[llength $mhs] == 0} {
        return "global"
    } else {
        return "local"
    }
}


##########################################################################
# Get number of used external interrupts
##########################################################################
proc get_num_intr_inputs {periph} {
    set intc_use_ext_intr [common::get_property CONFIG.C_INTC_USE_EXT_INTR $periph]
    if {$intc_use_ext_intr} {
        set num_intr_inputs [common::get_property CONFIG.C_INTC_INTR_SIZE $periph]
    } else {
        set num_intr_inputs 0
    }
    return $num_intr_inputs
}


##########################################################################
# Get number of used internal interrupts
##########################################################################
proc get_num_intr_internal {periph} {
    set c_use_uart_rx          [common::get_property CONFIG.C_USE_UART_RX $periph]
    set c_uart_error_interrupt [common::get_property CONFIG.C_UART_ERROR_INTERRUPT $periph]
    set c_uart_rx_interrupt    [common::get_property CONFIG.C_UART_RX_INTERRUPT $periph]
    set c_use_uart_tx          [common::get_property CONFIG.C_USE_UART_TX $periph]
    set c_uart_tx_interrupt    [common::get_property CONFIG.C_UART_TX_INTERRUPT $periph]
    set c_intc_use_ext_intr    [common::get_property CONFIG.C_INTC_USE_EXT_INTR $periph]
    set c_intc_intr_size       [common::get_property CONFIG.C_INTC_INTR_SIZE $periph]

    set num_intr_internal 0
    if {$c_use_uart_tx * $c_use_uart_rx * $c_uart_error_interrupt} { set num_intr_internal 1 }
    if {$c_use_uart_tx * $c_uart_tx_interrupt}                     { set num_intr_internal 2 }
    if {$c_use_uart_rx * $c_uart_rx_interrupt}                     { set num_intr_internal 3 }
    foreach kind {PIT FIT GPI} suffix {"SIZE" "No_CLOCKS" "INTERRUPT"} intbit {3 7 11} {
      foreach it {1 2 3 4} {
        set c_use_it       [expr [common::get_property CONFIG.C_${kind}${it}_${suffix} $periph] > 0]
        set c_it_interrupt [common::get_property CONFIG.C_${kind}${it}_INTERRUPT $periph]
        if {$c_use_it * $c_it_interrupt} { set num_intr_internal [expr $intbit + $it] }
      }
    }
    # If any external interrupts are used - return 16 since in that case all internal interrupts
    # must be accounted for because external interrupts start at bit position 16
    if {$c_intc_use_ext_intr} {
        return 16
    }
    return $num_intr_internal
}


##########################################################################
# Define parameters as hexadecimal values in include file. Derived from
# xdefine_include_file in "$XILINX_EDK/data/datastructure/xillib_sw.tcl".
##########################################################################
proc xdefine_include_file_hex {drv_handle file_name drv_string args} {

	set uSuffix "U"
	set args [hsi::utils::get_exact_arg_list $args]
    # Open include file
    set file_handle [hsi::utils::open_include_file $file_name]
  
    # Get all peripherals connected to this driver
    set periphs [hsi::utils::get_common_driver_ips $drv_handle]

    # Print all parameters for all peripherals as hexadecimal strings
    set device_id 0
    foreach periph $periphs {
        puts $file_handle ""
        puts $file_handle "/* Additional definitions for peripheral [string toupper [common::get_property NAME  $periph]] */"
        foreach arg $args {
            set value [common::get_property CONFIG.$arg $periph]
            if {[llength $value] == 0} {
                set value 0
            }

            # Convert binary string starting with "0b" to hexadecimal
            if {[string first "0b" $value] == 0} {
                set value_int 0
                for {set i 2} {$i < [string length $value]} {incr i} {
                    set value_int [expr $value_int * 2 + [string index $value $i]]
                }
                set value [format "0x%08x" $value_int]
            }

            puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value$uSuffix"
        }
        puts $file_handle ""
    }           
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

##############################################################################
# Define parameters in xparameters.h and generate PIT_EXPIRED_MASK parameter
##############################################################################
proc xdefine_include_file {drv_handle file_name drv_string args} {
    set args [hsi::utils::get_exact_arg_list $args]
    # Open include file
    set file_handle [hsi::utils::open_include_file $file_name]
    set flag 0

    set uSuffix "U"

    # Get all peripherals connected to this driver
    set periphs [hsi::utils::get_common_driver_ips $drv_handle]

    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [common::get_property name $drv_handle]] */"
        # Define NUM_INSTANCES
        puts $file_handle "#define [hsi::utils::get_driver_param_name $drv_string $arg] [llength $periphs]$uSuffix"
        set args [lreplace $args $posn $posn]
    }

    # Check if it is a driver parameter
    lappend newargs 
    foreach arg $args {
        set value [common::get_property CONFIG.$arg $drv_handle]
        if {[llength $value] == 0} {
            lappend newargs $arg
        } else {
            puts $file_handle "#define [hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property $arg $drv_handle]$uSuffix"
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
            	if {[string first "_EXPIRED_MASK" $arg] > 0} {
	      		set charindex [string first "_EXPIRED_MASK" $arg]
	        	set size [string index $arg [expr $charindex - 1]]
	        	set sizearg [format "C_PIT%d_SIZE" $size]
	        	set lvalue [format "PIT%d_EXPIRED_MASK" $size]
			set lvalue [format "XPAR_%s_%s" [string toupper [common::get_property NAME $periph]] $lvalue]
	                set rvalue [common::get_property CONFIG.$sizearg $periph]
	                set rvalue [expr pow(2, $rvalue) - 1]
	                set rvalue [format "%.0f" $rvalue]
	                set rvalue [format "0x%08X" $rvalue]
	                set flag 1
		} else {
			set value [common::get_property CONFIG.$arg $periph]
            	}
            }
            if {[llength $value] == 0} {
                set value 0
            }
            set value [hsi::utils::format_addr_string $value $arg]
            if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] \"$value\""
            } else {
                if {$flag == 0} {
			puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value$uSuffix"
                } else {
			puts $file_handle "#define $lvalue $rvalue$uSuffix"
                	set flag 0
                }
                
            }
        }
        puts $file_handle ""
    }		
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

################################################
# This procedure creates a unique list of
# handlers that needs to have an extern defn.
# in xparameters.h
################################################
proc intc_add_handler {handler} {

    variable interrupt_handlers

    set interrupt_handlers($handler) 1
}

###############################################################################
# this proc traverse all the interrupt source of peripheral and create a array
# will the the details
###############################################################################
proc intc_update_source_array {periph} {

    variable default_interrupt_handler
    variable source_port_name
    variable source_name
    variable source_port_type
    variable source_driver
    variable source_interrupt_handler
    variable source_interrupt_id
    variable total_source_intrs

    array unset source_port_name
    array unset source_name
    array unset source_port_type
    array unset source_driver
    array unset source_interrupt_handler
    array unset source_interrupt_id

    lappend source_pins
    set source_pins [::hsi::utils::get_interrupt_sources $periph]
    set intr_cnt 0
    foreach source_pin $source_pins {

        #default value as per external processor
        set t_source_port_name          [common::get_property NAME $source_pin]
        set t_source_name               "system"
        set t_ip_name                   ""
        set t_port_type                 "global"
        set t_source_driver             ""
        set t_source_interrupt_handler  $default_interrupt_handler
        set t_source_intrrupt_id        "-1"

        #if interrupt is coming from IP, update it.
        if { [::hsi::utils::is_external_pin $source_pin] == 0} {
            set source_periph   [hsi::get_cells -of_objects $source_pin]
            set t_source_name   [common::get_property NAME $source_periph]
            set t_ip_name       $t_source_name
            set t_port_type     "local"
           set t_source_driver [hsi::get_drivers -filter "HW_INSTANCE==$t_source_name"]
        }
        set port_intr_id [::hsi::utils::get_interrupt_id $t_ip_name $source_pin]
        if { [llength $port_intr_id ] > 1 } {

            #this is the case of vector interrupt port
            set j 0
            foreach pin_id $port_intr_id {
                set source_port_name($intr_cnt)         "${t_source_port_name}_$j"
                set source_name($intr_cnt)              $t_source_name
                set source_port_type($intr_cnt)         $t_port_type
                set source_driver($intr_cnt)            $t_source_driver
                set source_interrupt_handler($intr_cnt) $t_source_interrupt_handler
                set source_interrupt_id($intr_cnt)      $pin_id
                incr intr_cnt
                incr j
            }
        } else {
            set source_port_name($intr_cnt)         "${t_source_port_name}"
            set source_name($intr_cnt)              $t_source_name
            set source_port_type($intr_cnt)          $t_port_type
            set source_driver($intr_cnt)            $t_source_driver
            set source_interrupt_handler($intr_cnt) $t_source_interrupt_handler
            set source_interrupt_id($intr_cnt)      $port_intr_id
            incr intr_cnt
        }
    }
    set total_source_intrs $intr_cnt
}

proc xdefine_getSuffix {arg_name value} {
		set uSuffix ""
		if { [string match "*DEVICE_ID" $value] == 0} {
			set uSuffix "U"
		}
		return $uSuffix
}

###############################################################################
# this proc checks returns 1 if the given drv_handle is pmc iomodule or psm
# iomodule, other wise it returns 0
###############################################################################
proc is_psmicroblaze_iomodule {drv_handle} {
        set list [get_mem_ranges -of_objects [hsi::get_cells -hier [get_sw_processor]]]
        set index [lsearch $list $drv_handle]
        set val 0
        if {$index >= 0} {
                set base_val [common::get_property BASE_VALUE [lindex [get_mem_ranges -of_objects [hsi::get_cells -hier [get_sw_processor]]] $index]]
		set base_val [string trimleft $base_val "0x"]
		set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
		foreach periph $periphs {
			set baseaddr [::hsi::utils::get_param_value $periph C_BASEADDR]
			set baseaddr [string trimleft $base_val "0x"]
			if {[string compare -nocase $base_val $baseaddr] == 0} {
				set is_pl [common::get_property IS_PL $periph]
				if {$is_pl == 1} {
					return $val
				}
			}
		}
		set addr_list "F0280000 FFC80000 100280000 108280000 110280000 118280000"
		if {[lsearch -nocase $addr_list $base_val] >= 0} {
			set val 1
		}
	}

	return $val
}
