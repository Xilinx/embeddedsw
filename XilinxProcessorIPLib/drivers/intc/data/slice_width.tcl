###############################################################################
# Copyright (C) 2019 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
##############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 3.10   adk  13/09/19 First release
# 3.11   adk  10/03/20 Fix race condition for designs where interrupt pin is
#                      connected to cascade slices.
#        adk  13/03/20 Add new proc is_slice_exists_in_path to know slice exist
#		       in between the source pin and the interrupt
#		       controller interrupt pin.
# 3.12   mus  15/07/20 Fixed designs where external interrupt port is connected
#              to more than one slice instances of variable output width.
#
##############################################################################

#uses "xillib.tcl"
#
# Check whether a slice exists b/w the interrupt source pin and interrupt controller
#

#
# These are global variables to track the traversed external ports and
# Corresponding slice instance traversed by it.
# These variables would be reset every time when intc_update_source_array
# proc is called.
#
array set traversed_source_port_name ""
set num_slice_traversed 0
array set traversed_slice_instance ""
set check_slice_duplication 0

proc is_slice_exists_in_path {periph_handle intr_src_pin} {
   set slice_exist 0
   lappend interrupt_pins
   set interrupt_pins [::hsi::get_pins -of_objects $periph_handle -filter {TYPE==INTERRUPT && DIRECTION==I}]
   foreach interrupt_pin $interrupt_pins {
       set slice_exist [get_intr_src_pins_for_slice $interrupt_pin $intr_src_pin]
   }

   if {$slice_exist == 0 || $slice_exist == ""} {
       return 0
   } else {
       return 1
   }
}

#
# Get handles for all ports driving the interrupt pin of a peripheral
#
proc get_intr_connected_slice_width {periph_handle intr_src_pin} {
   set slice_width 0
   lappend interrupt_pins
   set interrupt_pins [::hsi::get_pins -of_objects $periph_handle -filter {TYPE==INTERRUPT && DIRECTION==I}]
   foreach interrupt_pin $interrupt_pins {
       set slice_width [get_intr_src_pins_for_slice $interrupt_pin $intr_src_pin]
   }
   return $slice_width
}
#
# Get the interrupt source pins of a periph pin object
#
proc get_intr_src_pins_for_slice {interrupt_pin intr_src_pin} {
    lappend slice_width
    set source_pins [::hsi::utils::get_source_pins $interrupt_pin]
    foreach source_pin $source_pins {
        set source_cell [::hsi::get_cells -of_objects $source_pin]
        if { [llength $source_cell ] } {
            #For concat IP, we need to bring pin source for other end
            set ip_name [common::get_property IP_NAME $source_cell]
            if { [string match -nocase $ip_name "xlconcat" ] } {
                set slice_width [get_concat_interrupt_sources_for_slice $source_cell $intr_src_pin]
            } elseif { [string match -nocase $ip_name "xlslice"] } {
                set slice_width [get_slice_interrupt_sources_for_slice $source_cell $intr_src_pin]
            } elseif { [string match -nocase $ip_name "util_reduced_logic"] } {
                set slice_width [get_util_reduced_logic_interrupt_sources_for_slice $source_cell $intr_src_pin]
            }
        }
        if { [llength $slice_width] > 0 } {
            break
        }

    }
    return $slice_width
}


#It assume that XLCONCAT IP cell object is passed to this function
proc get_concat_interrupt_sources_for_slice { concat_ip_obj intr_src_pin {lsb -1} {msb -1} } {
    lappend slice_width
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
                    set slice_width [get_concat_interrupt_sources_for_slice $source_cell $intr_src_pin]
                } elseif { [string match -nocase $ip_name "xlslice"] } {
                    set slice_width [get_slice_interrupt_sources_for_slice $source_cell $intr_src_pin]
                } elseif { [string match -nocase $ip_name "util_reduced_logic"] } {
                    set slice_width [get_util_reduced_logic_interrupt_sources_for_slice $source_cell $intr_src_pin]
                }
               if { [llength $slice_width] > 0 } {
                   return $slice_width
               }
            }
        }
    }
    return $slice_width
}

#
# Parameter slice_ip_obj is slice instance name
# It returns 1 , if specific slice instance is already traversed for specific external
# interrupt port
#
proc is_slice_traversed { slice_ip_obj } {
    variable traversed_source_port_name
    variable traversed_slice_instance
    variable num_slice_traversed
    variable check_slice_duplication

    if {$check_slice_duplication == 1} {
        for {set count 0} {$count < $num_slice_traversed} {incr count} {
            if { ([string compare -nocase $traversed_source_port_name($count) $traversed_source_port_name($num_slice_traversed)] == 0) \
                && ([ string compare -nocase $traversed_slice_instance($count) $slice_ip_obj] == 0)} {
                return 1
             }
        }
    }
    return 0
}

#
# It adds slice instance "slice_ip_obj" to traversed_slice_instance array
#
proc update_traversed_slice { slice_ip_obj } {
    variable traversed_slice_instance
    variable num_slice_traversed

    set traversed_slice_instance($num_slice_traversed) $slice_ip_obj
}

proc get_slice_interrupt_sources_for_slice { slice_ip_obj intr_src_pin } {
    lappend slice_width
    set is_duplicate [is_slice_traversed $slice_ip_obj]

    if {$is_duplicate == 1} {
        return ""
    }
    set in_pin [::hsi::get_pins -of_objects $slice_ip_obj "Din"]
    set pins [::hsi::utils::get_source_pins $in_pin]
    foreach pin $pins {
        set source_cell [::hsi::get_cells -of_objects $pin]
        if { [llength $source_cell] } {
            set ip_name [common::get_property IP_NAME $source_cell]
            #Cascading case of xlslice IP
            if { [string match -nocase $ip_name "xlslice"] } {
                set slice_width [ common::get_property CONFIG.DOUT_WIDTH $slice_ip_obj ]
            } elseif { [string match -nocase $ip_name "xlconcat"] } {
                set from [::common::get_property CONFIG.DIN_FROM $slice_ip_obj]
                set to [::common::get_property CONFIG.DIN_TO $slice_ip_obj]
                set lsb [expr $from < $to ? $from : $to]
                set msb [expr $from > $to ? $from : $to]
                incr msb
                set slice_width [get_concat_interrupt_sources_for_slice $source_cell $intr_src_pin $lsb $msb ]
            } elseif { [string match -nocase $ip_name "util_reduced_logic"] } {
                set slice_width [get_util_reduced_logic_interrupt_sources_for_slice $intr_src_pin]
            } else {
                if { $pin eq $intr_src_pin } {
                  set slice_width [ common::get_property CONFIG.DOUT_WIDTH $slice_ip_obj ]
                }
            }

        } else {
              if { $pin eq $intr_src_pin } {
                  set slice_width [ common::get_property CONFIG.DOUT_WIDTH $slice_ip_obj ]
              }
        }

    }
    update_traversed_slice $slice_ip_obj
    return $slice_width
}

proc get_util_reduced_logic_interrupt_sources_for_slice { url_ip_obj intr_src_pin } {
    lappend slice_width
    set in_pin [::hsi::get_pins -of_objects $url_ip_obj "Op1"]
    set pins [::hsi::utils::get_source_pins $in_pin]
    foreach pin $pins {
        set source_cell [::hsi::get_cells -of_objects $pin]
        if { [llength $source_cell] } {
            set ip_name [common::get_property IP_NAME $source_cell]

            if { [string match -nocase $ip_name "xlslice"] } {
                set source_pins [get_slice_interrupt_sources_for_slice $source_cell $intr_src_pin]
            } elseif { [string match -nocase $ip_name "xlconcat"] } {
                set source_pins [get_concat_interrupt_sources_for_slice $source_cell $intr_src_pin]
            } elseif { [string match -nocase $ip_name "util_reduced_logic"] } {
        #Cascading case of util_reduced_logic IP
                set source_pins [get_util_reduced_logic_interrupt_sources_for_slice $source_cell $intr_src_pin]
            }

        }
    }
    return $slice_width
}
