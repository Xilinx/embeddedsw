###############################################################################
#
# Copyright (C) 2005 - 2018 Xilinx, Inc.  All rights reserved.
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
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 3.0      adk    12/10/13 Updated as per the New Tcl API's
# 3.5      sk     11/09/15 Removed delete filename statement CR# 784758.
# 3.7      ms     04/18/17 Modified tcl file to add suffix U for macro
#                          definitions of intc in xparameters.h
# 3.9      adk    19/12/18 If design contains unconnected interrupt pins
#			   return proper error message CR#1018878
##############################################################################

## @BEGIN_CHANGELOG
##
##     09/17/07 ecm  Removed the PCI specific connections
##              Moved to the Linux MLD.
##     10/24/07 rpm  Fixed KIND_OF_INTR bug in canonicals and
##              also generalized match on external interrupts
##     06/25/09 sdm  Updated so that canonical definitions are
##              not generated when instance name matches
##              canonical name
##     04/27/10 sdm Updated the tcl so that the defintions are generated in
##		the xparameters.h to know whether the optional registers
##		SIE, CIE and IVR are enabled in the HW - Refer CR 555392
##     05/24/11 hvm updated tcl to generate vector ids for external interrupts
##		CR565336
##     06/15/11 hvm Updated tcl with bypassing the external interrupt definition
##		in xredefine_intc function. CR613925.
##     01/19/12 Updated the intc_define_use_dcr function so that it doesnot
##              error out if there is more than one bus interface to the
##              intc controller. The new common::version of the AXI_INTC can have two
##              bus interfaces to support the fast interrupt for MicroBlaze.
##              Updated for the generation of the C_HAS_FAST xparameters
##     08/16/12 bss added generation of C_IVAR_RESET_VALUE xparameters
##
##     01/29/13 bss Added check_cascade and get_intctype to support
##		    Cascade mode. Modified intc_define_vector_table procedure
##		    to generate interrupt IDs as 32..63 and 64..95 and so on
##		    for Slave controllers in Cascade mode
##     01/22/14 bss Modified check_cascade to fix CR#764865
##     17/02/14 adk Fixed the CR:771287 in intc_define_vector_table
##		    if number of interrupt ports not equal to total numer of
##		    interrupts returning immediatly.And in the xredefine_intc
##		    if there is not interrupt source returining immediately.
##     4/8/14   bss Modified xredefine_intc to handle external interrupt pins
##		    correctly (CR#799609).
##     11/3/14 adk  added generation of C_HAS_ILR parameter to xparameters.h
##		    (CR#828046).
##     01/07/17 mus Updated xredefine_intc to return immediately, if number of
##                  connected interrupt sources are 0 (CR#966295)
##     01/25/17 mus Updated xredefine_intc and intc_define_vector_table functions
##                  to generate separate canonical definitions and constants
##                  definitions for interrupt IDs/Masks, if interrupt pin of same IP
##                  is connected to two axi intc pins
##     06/28/18 mus Updated check_cascade proc, to add check
##                  for irq_in pin, while detecting cascaded
##                  interrupt controllers.It fixes CR#1005371.
##
##
##
## @END_CHANGELOG

############################################################
# Global interrupt handlers array, default handler routine
############################################################
array set interrupt_handlers ""
set default_interrupt_handler "XNullHandler"
set cascade 0
set intrid 0

array set source_port_name         ""
array set source_name              ""
array set source_port_type         ""
array set source_driver            ""
array set source_interrupt_handler ""
array set source_interrupt_id      ""
set total_source_intrs             0

############################################################
# DRC procedure
############################################################
proc intc_drc {drv_handle} {

}


############################################################
# "generate" procedure
############################################################
proc generate {drv_handle} {

	# Generate the following definitions in xparameters.h
	# 2. BASEADDR, HIGHADDR, C_NUM_INTR_INPUTS, XPAR_INTC_MAX_NUM_INTR_INPUTS

	set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
    set count [llength $periphs]
    variable cascade

	if {$count > 1} {
		set cascade [check_cascade $drv_handle]
	}

	if {$cascade == 0} {
		::hsi::utils::define_max $drv_handle "xparameters.h" "XPAR_INTC_MAX_NUM_INTR_INPUTS" "C_NUM_INTR_INPUTS"
	} else {
		set maxintrs 0
		foreach periph $periphs {
			set intrs [common::get_property CONFIG.C_NUM_INTR_INPUTS $periph]
			set maxintrs [expr "$maxintrs + $intrs"]
		}
		set file_handle [::hsi::utils::open_include_file "xparameters.h"]
		puts $file_handle "#define XPAR_INTC_MAX_NUM_INTR_INPUTS $maxintrs"
        close $file_handle
	}

	foreach periph $periphs {
		set fast [::hsi::utils::get_param_value $periph "C_HAS_FAST"]
		set nested [::hsi::utils::get_param_value $periph "C_HAS_ILR"]
	        if {$fast == 1 && $nested == 1} {
				 puts "ERROR: Internal error: Interrupt Controller Driver has no support for nesting fast interrupts"
	        }
	        if {$cascade == 1 && $nested == 1} {
			puts "ERROR: Internal error: Interrupt Controller Driver has no support for nesting interrupts in Cascaded mode"
	        }
	}

	::hsi::utils::define_if_all $drv_handle "xparameters.h" "XIntc" "C_HAS_IPR" "C_HAS_SIE" "C_HAS_CIE" "C_HAS_IVR" "C_HAS_ILR"
	::hsi::utils::define_include_file $drv_handle "xparameters.h" "XIntc" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_KIND_OF_INTR" "C_HAS_FAST" "C_IVAR_RESET_VALUE" "C_NUM_INTR_INPUTS" "C_ADDR_WIDTH"


	# Define XPAR_SINGLE_DEVICE_ID

	if {$count == 1} {
	    ::hsi::utils::define_with_names $drv_handle [::hsi::utils::get_common_driver_ips $drv_handle] "xparameters.h" "XPAR_INTC_SINGLE_BASEADDR" "C_BASEADDR" "XPAR_INTC_SINGLE_HIGHADDR" "C_HIGHADDR" "XPAR_INTC_SINGLE_DEVICE_ID" "DEVICE_ID"
	}


	set config_inc [::hsi::utils::open_include_file "xparameters.h"]

	# Generate config table, vector tables
	intc_define_config_file $drv_handle $periphs $config_inc

	close $config_inc

	# Generate canonical xparameters
	xdefine_canonical_xpars $drv_handle "xparameters.h" "Intc" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_KIND_OF_INTR" "C_HAS_FAST" "C_IVAR_RESET_VALUE" "C_NUM_INTR_INPUTS" "C_ADDR_WIDTH"
}


##########################################################################
# Generate Configuration C file xintc_g.c
# This file has the Config Table and vector tables for each intc instance
# as required by Xilinx intc driver
##########################################################################

proc intc_define_config_file {drv_handle periphs config_inc} {

	variable interrupt_handlers
	variable default_interrupt_handler
	variable cascade

	# set isr_options to be XIN_SVC_SGL_ISR_OPTION as defined in xintc.h
	set isr_options XIN_SVC_SGL_ISR_OPTION
	set file_name "xintc_g.c"
	set drv_string "XIntc"
	set args [list "DEVICE_ID" "C_BASEADDR" "C_KIND_OF_INTR" "C_HAS_FAST" "C_IVAR_RESET_VALUE" "C_NUM_INTR_INPUTS" "C_ADDR_WIDTH"]
	set filename [file join "src" $file_name]
	set config_file [open $filename w]
	::hsi::utils::write_c_header $config_file "Driver configuration"
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
        set periph_name [string toupper [get_property NAME $periph ]]
	set xpar_periph_name [::hsi::utils::format_xparam_name $periph_name]

	set uSuffix "U"
	if {$cascade == 1} {
            puts $config_inc [format "#define XPAR_%s_%s %d$uSuffix" [string toupper [common::get_property NAME $periph ]] "TYPE" [get_intctype $periph]]
        } else {
            puts $config_inc [format "#define XPAR_%s_%s %d$uSuffix" [string toupper [common::get_property NAME $periph ]] "TYPE" $cascade]
        }
        puts $tmp_config_file [format "%s\t\{" $start_comma]
            set comma ""
            foreach arg $args {
                # Check if this is a driver parameter or a peripheral parameter
                set value [common::get_property CONFIG.$arg $drv_handle ]
            if {[llength $value] == 0} {
                puts -nonewline $tmp_config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
            } else {
                puts -nonewline $tmp_config_file [format "%s\t\t%s" $comma [::hsi::utils::get_driver_param_name $drv_string $arg]]
            }
            set comma ",\n"
        }
        # add the OPTIONS as an arg to the config table - default OPTIONS value is XIN_SVC_SGL_ISR_OPTION
		puts -nonewline $tmp_config_file [format "%s\t\t%s" $comma $isr_options]
		puts -nonewline $tmp_config_file ",\n\t\t"
		puts -nonewline $tmp_config_file [format "XPAR_%s_%s" [string toupper [common::get_property NAME $periph]] "TYPE"]

		# generate the vector table for this intc instance
		intc_define_vector_table $periph $config_inc $tmp_config_file

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

proc intc_define_vector_table {periph config_inc config_file} {

	set uSuffix "U"
	variable interrupt_handlers
	variable default_interrupt_handler
	variable cascade
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
    set interrupt_pin [hsi::get_pins -of_objects $periph intr]

    # Get pins/ports that are driving the interrupt
	lappend source_pins
	set source_pins [::hsi::utils::get_source_pins $interrupt_pin]

    set num_intr_inputs [common::get_property CONFIG.C_NUM_INTR_INPUTS $periph]
    #calculate the total interrupt sources

    set total_intr_ports [::hsi::utils::get_connected_pin_count $interrupt_pin]

    if {$num_intr_inputs != $total_intr_ports} {
        if {$num_intr_inputs != 1} {
            puts "ERROR: Internal error: Num intr inputs $num_intr_inputs not the same as length of ::hsi::utils::get_interrupt_sources [llength $source_pins] hsi_error"
        }
       return
    }

    #Check if default_interrupt_handler has to have an extern definition
    if {[array size interrupt_handlers] < $total_source_intrs } {
        intc_add_handler $default_interrupt_handler
    }

    puts -nonewline $config_file ",\n\t\t\{"
    set comma "\n"
    set instance_list {}

    for {set i 0} {$i < $total_source_intrs} {incr i} {
        set source_ip $source_name($i)
        if { [llength $source_ip] != 0 && [llength [hsi::get_cells -hier $source_ip]] != 0} {
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
            set source_name_port_name $source_name($i)$source_port_name($i)
            set sname [string toupper $source_name($i)]
	    set source_xparam_name [::hsi::utils::format_xparam_name $sname]
	    set pname [string toupper $periph_name]
	    set periph_xparam_name [::hsi::utils::format_xparam_name $pname]
	    if {[lcount $instance_list $source_name_port_name] != 0} {
	        puts $config_inc [format "#define XPAR_%s_%s_LOW_PRIORITY_MASK %#08X$uSuffix" $source_xparam_name [string toupper $source_port_name($i)] [expr 1 << $i]]
	    } else {
	        puts $config_inc [format "#define XPAR_%s_%s_MASK %#08X$uSuffix" $source_xparam_name [string toupper $source_port_name($i)] [expr 1 << $i]]
	    }
	    if {$cascade ==1} {
	       if {[lcount $instance_list $source_name_port_name] == 0} {
                puts $config_inc [format "#define XPAR_%s_%s_%s_INTR %d$uSuffix" [string toupper $periph_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] $source_interrupt_id($i)]
	       } else {
	           puts $config_inc [format "#define XPAR_%s_%s_%s_LOW_PRIORITY_INTR %d$uSuffix" [string toupper $periph_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] $source_interrupt_id($i)]
	       }
            } else {
                 if {[lcount $instance_list $source_name_port_name] == 0} {
                     puts $config_inc [format "#define XPAR_%s_%s_%s_INTR %d$uSuffix" [string toupper $periph_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] $i]
                 } else {
                     puts $config_inc [format "#define XPAR_%s_%s_%s_LOW_PRIORITY_INTR %d$uSuffix" [string toupper $periph_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] $i]
                 }

            }
	    lappend instance_list $source_name_port_name
            if {[string compare -nocase "global" $source_port_type($i) ] != 0 && \
                [string compare $source_interrupt_handler($i) $default_interrupt_handler ] != 0} {
                puts $config_file [format "\t\t\t\t(void *) %s" $source_handler_arg($i)]
            } else {
                puts $config_file "\t\t\t\t(void *) XNULL"
            }
        }
        puts -nonewline $config_file "\t\t\t\}"
        set comma ",\n"
    }
    puts $config_file "\n\t\t\}"
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


#
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {

   variable cascade

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

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {
            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            foreach arg $args {
                set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]

    # The commented out rvalue is the name of the instance-specific constant
    #           set rvalue [::hsi::utils::get_ip_param_name $periph $arg]

                # The rvalue set below is the actual value of the parameter
                set rvalue [::hsi::utils::get_param_value $periph $arg]
                if {[llength $rvalue] == 0} {
                     set rvalue 0
                }
                set rvalue [::hsi::utils::format_addr_string $rvalue $arg]

		set uSuffix [xdefine_getSuffix $lvalue $rvalue]
                puts $file_handle "#define $lvalue $rvalue$uSuffix"

            }
            if {$cascade == 1} {
		puts $file_handle "#define [::hsi::utils::get_driver_param_name $canonical_name "INTC_TYPE"] [get_intctype $periph]$uSuffix"
            } else {
		puts $file_handle "#define [::hsi::utils::get_driver_param_name $canonical_name "INTC_TYPE"] $cascade$uSuffix"
            }
            puts $file_handle ""
            incr i
        }
    }

    #
    # Now redefine the Interrupt ID constants
    #
    xredefine_intc $drv_handle $file_handle

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

################################################
#
# intc redefines
#
################################################
proc xredefine_intc {drvhandle config_inc} {
    variable source_port_name
    variable source_name
    variable source_port_type
    variable source_driver
    variable source_interrupt_handler
    variable source_interrupt_id
    variable total_source_intrs


    # Next define interrupt IDs for each connected peripheral

    set periphs [::hsi::utils::get_common_driver_ips $drvhandle]
    set device_id 0
    set periph_name [string toupper "intc"]

    if {$total_source_intrs == 0} {
        return
     }

    foreach periph $periphs {
        #update global array of Interrupt sources for this periph
        intc_update_source_array $periph

        lappend source_list
        for {set j 0 } { $j < $total_source_intrs   } { incr j} {
            lappend source_list $source_name($j)
        }


        # Get the edk based name of peripheral for printing redefines
        set periph_ip_name [common::get_property NAME $periph]

        set num_intr_inputs [common::get_property CONFIG.C_NUM_INTR_INPUTS $periph]
	if {$num_intr_inputs != $total_source_intrs} {
	    error "ERROR: unconnected interrupt pins in the design.\n" "" "MDT_ERROR"
	    return
	}

        set instance_list {}
        for {set i 0} {$i < $num_intr_inputs} {incr i} {

            if {[string compare -nocase $source_name($i) "system"] == 0} {
                continue
            }
            set drv [hsi::get_drivers -filter "HW_INSTANCE==$source_name($i)"]

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
                # that is connected to this intc.
                #
                set drvname [string toupper $drvname]

                #
                # Treat sources with multiple interrupt ports slightly different
                # by including the interrupt port name in the canonical constant
                # name
                #
                if { [lcount $source_list $source_name($i)] > 1} {
                    set port_name [string toupper $source_port_name($i)]
                    set source_name_port_name $source_name($i)$source_port_name($i)
		    #
		    # If IP is interrupting through two axi intc pins
		    # then add "LOW_PRIORITY" string to canonical definition and constant
		    # definition of interrupt IDs connected to higher pin number ( i.e. low priority pins )
		    # These canonical definitions can be used by applications to dynamically change
		    # the interrupt priority of IP. Provided that IP is interrupting
		    # through 2 axi intc pins (i.e. low priority and high priory ) in HW design
		    #

                    #
                    # If there are multiple interrupt ports for axi_ethernet, do not include
                    # the port name in the canonical name, for the port "INTERRUPT". Other ports
                    # will have the port name in the canonical name. This is to make sure that
                    # the canonical name for the port INTERRUPT will remain same irrespective of
                    # whether the design has a single interrupt port or multiple interrupt ports
                    #
                    if {[string compare -nocase $drvname "axiethernet"] == 0} {
                        if {[string compare -nocase $port_name "INTERRUPT"] == 0} {

                            if {[lcount $instance_list $source_name_port_name] == 0 } {
                                set first_part [format "#define XPAR_%s_%s_%s_%s_VEC_ID" $periph_name $device_id $drvname $instance]
                                set second_part [format "XPAR_%s_%s_%s_INTR" [string toupper $periph_ip_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] ]
                             } else {
                                set first_part [format "#define XPAR_%s_%s_%s_%s_LOW_PRIORITY_VEC_ID" $periph_name $device_id $drvname $instance]
                                set second_part [format "XPAR_%s_%s_%s_LOW_PRIORITY_INTR" [string toupper $periph_ip_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] ]
                             }
                        } else {
                            if {[lcount $instance_list $source_name_port_name] == 0 } {
                                set first_part [format "#define XPAR_%s_%s_%s_%s_%s_VEC_ID" $periph_name $device_id $drvname $instance $port_name]
                                set second_part [format "XPAR_%s_%s_%s_INTR" [string toupper $periph_ip_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] ]
                            } else {
                                set first_part [format "#define XPAR_%s_%s_%s_%s_%s_LOW_PRIORITY_VEC_ID" $periph_name $device_id $drvname $instance $port_name]
                                set second_part [format "XPAR_%s_%s_%s_LOW_PRIORITY_INTR" [string toupper $periph_ip_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] ]
                            }
                        }

                    } elseif {[lcount $instance_list $source_name_port_name] != 0}  {
                            set first_part [format "#define XPAR_%s_%s_%s_%s_%s_LOW_PRIORITY_VEC_ID" $periph_name $device_id $drvname $instance $port_name]
                            set second_part [format "XPAR_%s_%s_%s_LOW_PRIORITY_INTR" [string toupper $periph_ip_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] ]
                    } else {
		            set first_part [format "#define XPAR_%s_%s_%s_%s_%s_VEC_ID" $periph_name $device_id $drvname $instance $port_name]
			    set second_part [format "XPAR_%s_%s_%s_INTR" [string toupper $periph_ip_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] ]
		    }
		    lappend instance_list $source_name_port_name
                } else {
                    set first_part [format "#define XPAR_%s_%s_%s_%s_VEC_ID" $periph_name $device_id $drvname $instance]
                    set second_part [format "XPAR_%s_%s_%s_INTR" [string toupper $periph_ip_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] ]
                }


                if {[string compare -nocase $drvname "generic"] != 0} {
                    set first_part_xparam_name [::hsi::utils::format_xparam_name $first_part]
                    set second_part_xparam_name [::hsi::utils::format_xparam_name $second_part]
                    puts $config_inc "$first_part_xparam_name $second_part_xparam_name"
                }
            }
        }
        incr device_id
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
        set name [common::get_property NAME $inst]
        if {[string compare -nocase $instname $name] == 0} {
            return $i
        }
        incr i
    }
    set i 0
    return $i
}

###################################################################################
#
# Checks whether system has Cascade interrupt controllers
#
###################################################################################
proc check_cascade {drv_handle} {
	set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
    foreach periph $periphs {
		set i 0
		set source_pins [::hsi::utils::get_interrupt_sources $periph]
        set irq_input [::hsi::get_pins -of_objects $periph irq_in]
        set irq [::hsi::utils::get_source_pins $irq_input]
        if { [llength $irq] > 0 } {
            lappend source_pins $irq
        }
        foreach source_pin $source_pins {
            set source_pin_name($i) [common::get_property NAME $source_pin]
            if { [::hsi::utils::is_external_pin $source_pin] } {
                continue
            }
            set source_periph [hsi::get_cells -of_objects $source_pin ]
            set source_type [common::get_property IP_TYPE $source_periph]
            if {[string compare -nocase $source_type "INTERRUPT_CNTLR"] == 0} {
		return 1
            }
        }
    }
	return 0
}

##################################################################
#
# Returns Interrupt controller type
# 1 - primary instance
# 2 - secondary instance
# 3 - last instance
#
##################################################################

proc get_intctype {periph} {
		set iscascade [::hsi::utils::get_param_value $periph "C_EN_CASCADE_MODE"]
		set ismaster [::hsi::utils::get_param_value $periph "C_CASCADE_MASTER"]
		if {$iscascade == 1 && $ismaster == 1} {
			set retval 1
		} elseif {$iscascade == 1 && $ismaster == 0} {
			 set retval 2
		} elseif {$iscascade == 0 && $ismaster == 0} {
			 set retval 3
		} else {
			error "ERROR: The C_CASCADE_MASTER is allowed to set only when the C_EN_CASCADE_MODE is set to 1"
		}

		return $retval
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
		if { [string match "*DEVICE_ID" $value] == 0 } {
			set uSuffix "U"
		}
		return $uSuffix
}
