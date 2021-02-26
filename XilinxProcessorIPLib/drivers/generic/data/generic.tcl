###############################################################################
# Copyright (C) 2009 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 2.0     adk    12/10/13 Updated as per the New Tcl API's
# 3.0     nsk    12/14/20 Modified the tcl to genrate canonical
#                         and peripheral defines for CIPS3
#                         designs
#         nsk    01/06/21 Modified the generate proc to check
#                         the hier prop if cips is present
# 3.0     nsk    02/26/21 Modified the generate proc to
#                         generate both canonical and
#                         peripheral defines
###############################################################

#----------------------------------------------------
# Defines all possible address params in the filename
# for all periphs that use this driver
#----------------------------------------------------
global set ipmap [dict create]

proc generate {drv_handle} {
    global paramlist
    set paramlist ""
    define_addr_params $drv_handle "xparameters.h" "peripheral"
    define_addr_params $drv_handle "xparameters.h" "canonical"
}

proc find_addr_params {periph} {

   set addr_params [list]

   #get the mem_ranges which belongs to this peripheral
   if { [llength $periph] != 0 } {
   set sw_proc_handle [::hsi::get_sw_processor]
   set hw_proc_handle [::hsi::get_cells -hier [common::get_property hw_instance $sw_proc_handle]]
   set mem_ranges [::hsi::get_mem_ranges -of_objects $hw_proc_handle -filter "INSTANCE==$periph"]
   foreach mem_range $mem_ranges {
       set bparam_name [common::get_property BASE_NAME $mem_range]
       set bparam_value [common::get_property BASE_VALUE $mem_range]
       set hparam_name [common::get_property HIGH_NAME $mem_range]
       set hparam_value [common::get_property HIGH_VALUE $mem_range]

       # Check if already added
       set bposn [lsearch -exact $addr_params $bparam_name]
       set hposn [lsearch -exact $addr_params $hparam_name]
       if {$bposn > -1  || $hposn > -1 } {
           continue
       }
       lappend addr_params $bparam_name
       lappend addr_params $hparam_name
   }
   }
   return $addr_params
}

proc get_ip_param_name {periph_handle param type {device_id ""}} {

   global paramlist
   set dev_id $device_id
   if {[string match -nocase $type "canonical"]} {
       set name [common::get_property IP_NAME $periph_handle ]
       # For the IPS psv_fpd_smmutcu_0 and psv_fpd_maincci_0 the IP_NAME is same
       # so duplicate macros will be defined in xparameters.h, so use NAME
       # instead of IP_NAME. will remove this check, once it is fixed
       if {[string match -nocase $name "psv_fpd_smmutcu"]} {
           set prop [common::get_property CONFIG.C_S_AXI_BASEADDR $periph_handle]
           if {$prop == 0xFD000000} {
               set name [common::get_property NAME $periph_handle ]
           }
       }
   } else {
       set name [common::get_property NAME $periph_handle ]
       set dev_id ""
   }
   set name [string toupper $name]
   set name [format "XPAR_%s_" $name]
   set devname $name
   set param [string toupper $param]
   if {$dev_id == ""} {
       if {[string match C_* $param]} {
           set name [format "%s%s" $name [string range $param 2 end]]
       } else {
           set name [format "%s%s" $name $param]
       }

   } else {
       if {[string match C_* $param]} {
           set name [format "%s%s%s" $name "${device_id}_" [string range $param 2 end]]
       } else {
           set name [format "%s%s%s" $name "${device_id}_" $param]
       }
   }

   # Update the global parameter list, so that we will check for duplicate entries
   # when we write to xparameters.h file.
   if {[lsearch -nocase $paramlist $name] == -1} {
        set paramlist [lappend paramlist $name]
   } else {
	return ""
   }

   # For peripheral names, do not use device ids
   if {$dev_id == ""} {
      if {[string match C_* $param]} {
          set name [format "%s%s" $devname [string range $param 2 end]]
      } else {
          set name [format "%s%s" $devname $param]
      }
   }

   return $name
}
proc define_addr_params {drv_handle file_name type} {

   global ipmap
   set ipmap [dict create]
   set addr_params [list]
   global paramlist
   # Open include file
   set file_handle [::hsi::utils::open_include_file $file_name]
   # Get all peripherals connected to this driver
   set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
   # Print all parameters for all peripherals

   foreach periph $periphs {
	set name [common::get_property IP_NAME $periph]
	set inst_name [common::get_property NAME $periph]
	set is_nr [regexp -all {[_][0-9]$} $name]
	if {$is_nr == 0} {
		set device_id [get_count $name]
	} else {
		set device_id ""
	}
   puts $file_handle ""

   set addr_params ""
   set addr_params [find_addr_params $periph]
   set periph_change 0
   foreach arg $addr_params {
       set value [::hsi::utils::get_param_value $periph $arg]
       if {$value != ""} {
           set value [::hsi::utils::format_addr_string $value $arg]
	   if {$device_id == ""} {
               set addrparam [get_ip_param_name $periph $arg $type]
               if {$addrparam != ""} {
			if {$periph_change == 0} {
			set periph_change 1
			if {[string match -nocase $type "canonical"]} {
				puts $file_handle "/* Canonical Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
			   } else {
			       puts $file_handle "/* Peripheral Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
			   }
			}
	           puts $file_handle "#define $addrparam $value"
              }
	   } else {
               set addrparam [get_ip_param_name $periph $arg $type $device_id]
               if {$addrparam != ""} {
		    if {$periph_change == 0} {
			set periph_change 1
			if {[string match -nocase $type "canonical"]} {
				puts $file_handle "/* Canonical Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
		       } else {
				puts $file_handle "/* Peripheral Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
			}
		    }
	            puts $file_handle "#define $addrparam $value"
		}
	   }

       }
   }
   puts $file_handle ""
   }
   puts $file_handle "\n/******************************************************************/\n"

   close $file_handle
}

proc get_count args {
	set param [lindex $args 0]
	global ipmap
	if {[catch {set rt [dict get $ipmap $param]} msg]} {
		dict append ipmap $param 0
		set value 0
	} else {
		set value [expr $rt + 1]
		dict unset ipmap $param
		dict append ipmap $param $value
	}

	return $value
}
