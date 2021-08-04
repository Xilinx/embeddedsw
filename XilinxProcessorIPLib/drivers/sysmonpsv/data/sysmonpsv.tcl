##############################################################################
# Copyright (C) 2016 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   aad  01/30/19 First release
# 	aad  02/25/19 Fix XSysMonPsv_Supply list enum when no supplies
# 		      are configured
# 2.1   aad  03/29/21 Add supply names in string format.
# 2.3   aad  07/15/21 Add support for SSIT devices
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    generate_include_file $drv_handle "xparameters.h" "XSysMonPsv" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
    generate_sysmon_config $drv_handle "xsysmonpsv_g.c" "XSysMonPsv" "C_S_AXI_BASEADDR"
    generate_canonical_xpars $drv_handle "xparameters.h" "XSysMonPsv" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
}

proc generate_include_file {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string "NUM_INSTANCES"] [llength $periphs]"
    # Print all parameters for all peripherals
    foreach periph $periphs {
        puts $file_handle ""
        puts $file_handle "/* Definitions for peripheral [string toupper [::hsi::utils::get_param_value $periph NAME ]] */"
        foreach arg $args {
            set value [::hsi::utils::get_param_value $periph $arg]
            if {[llength $value] == 0} {
                set value 0
            }
            set value [::hsi::utils::format_addr_string $value $arg]
            puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value"
        }
    }
    for {set index 0} {$index < 160} {incr index} {
	    set meas "C_MEAS_${index}"
	    set id   "${meas}_ROOT_ID"
            set value [::hsi::utils::get_param_value $drv_handle $meas]
            if {[llength $value] == 0} {
		 set local_value [::hsi::utils::get_param_value $periph $meas]
		 set id_value [::hsi::utils::get_param_value $periph $id]
                if {[string compare -nocase $local_value ""] != 0} {
			set final_value "#define XPAR_PMC_SYSMON_0_${local_value}\t${id_value}"
			puts $file_handle $final_value
		}
	    }
    }
    puts $file_handle ""
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

proc generate_canonical_xpars {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
   # Open include file
   set file_handle [::hsi::utils::open_include_file $file_name]

   # Get all the peripherals connected to this driver
   set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

   # Get the names of all the peripherals connected to this driver
   foreach periph $periphs {
       set peripheral_name [string toupper [::hsi::utils::get_param_value $periph NAME]]
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
       set periph_name [string toupper [::hsi::utils::get_param_value $periph NAME]]

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

               puts $file_handle "#define $lvalue $rvalue"

           }
           puts $file_handle ""
           incr i
       }
   }
    for {set index 0} {$index < 160} {incr index} {
	    set meas "C_MEAS_${index}"
	    set id   "${meas}_ROOT_ID"
            set value [::hsi::utils::get_param_value $drv_handle $meas]
            if {[llength $value] == 0} {
		 set local_value [::hsi::utils::get_param_value $periph $meas]
		 set id_value [::hsi::utils::get_param_value $periph $id]
                if {[string compare -nocase $local_value ""] != 0} {
			set final_value "#define XPAR_XSYSMONPSV_0_${local_value}\t${id_value}"
			puts $file_handle $final_value
		} elseif {[string compare -nocase $local_value ""] == 0} {
			if {$index == 0} {
				puts $file_handle "#define XPAR_XSYSMONPSV_0_NO_MEAS\t161"
				break
			}
		}

	    }
    }


   puts $file_handle "\n/******************************************************************/\n"

   puts $file_handle "/* Xilinx Sysmon Device Name */"

   puts $file_handle "\n/******************************************************************/\n"

   close $file_handle
}

proc generate_sysmon_config {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
    set filename [file join "src" $file_name]
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
    foreach periph $periphs {
        puts $config_file [format "%s\t\{" $start_comma]
        set comma ""
		set len [llength $args]
        foreach arg $args {
                puts -nonewline $config_file [format "%s\t\t%s,\n" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
	}
	 puts $config_file [format "%s\t\t\t{" $start_comma]
	 for {set index 0} {$index < 160} {incr index} {
		    set measid "C_MEAS_${index}"
		    set value [::hsi::utils::get_param_value $drv_handle $measid ]
		  if {[llength $value] == 0} {
			 set local_value [::hsi::utils::get_param_value $periph $measid]
			  if {[string compare -nocase $local_value ""] != 0} {
				set final_value "XPAR_PMC_SYSMON_0_${local_value}"
				puts -nonewline $config_file [format "%s\t\t\t\t%s,\n" $comma $final_value]
			  }
		   }
	  }
    puts $config_file [format "%s\t\t\t}" $start_comma]
    puts $config_file "\n\t\}"
    set start_comma ",\n"
    }
    puts $config_file "\n\};"
    puts $config_file "\n";

    set proc_instance [hsi::get_sw_processor]
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]]
    if {$proc_type == "psv_cortexa72" || $proc_type == "psv_cortexr5" || $proc_type == "psu_cortexa72" ||
	    $proc_type == "psu_cortexr5"} {
	    puts $config_file "const char * [format "\ %s_Supply_Arr" $drv_string]\[\] = {"
	    for {set index 0} {$index < 160} {incr index} {
		    set measid "C_MEAS_${index}"
		    set value [::hsi::utils::get_param_value $drv_handle $measid]
		    if {[llength $value] == 0} {
				set local_value [::hsi::utils::get_param_value $periph $measid]
				if {[string compare -nocase $local_value ""] != 0} {
					puts $config_file "\t\"${local_value}\","
				}
		    }
	   }
	   puts $config_file "};"
    }
    close $config_file
    generate_sysmon_supplies $drv_handle  "xsysmonpsv_supplylist.h" "XSysMonPsv"
}

proc generate_sysmon_supplies {drv_handle file_name drv_string} {
    set filename [file join "src" $file_name]
    set config_file [open $filename w]
    set periph [::hsi::utils::get_common_driver_ips $drv_handle]
    set comma ""
    ::hsi::utils::write_c_header $config_file "Enabled Supply List"
    puts $config_file "#ifndef XSYSMONPSV_SUPPLYLIST"
    puts $config_file "#define XSYSMONPSV_SUPPLYLIST"
    puts $config_file "\n/*"
    puts $config_file "* The supply configuration table for sysmon"
    puts $config_file "*/\n"
    puts $config_file "typedef enum \{"
    for {set index 0} {$index < 160} {incr index} {
	    set measid "C_MEAS_${index}"
            set value [::hsi::utils::get_param_value $drv_handle $measid]
            if {[llength $value] == 0} {
		 set local_value [::hsi::utils::get_param_value $periph $measid]
                if {[string compare -nocase $local_value ""] != 0} {
		 puts -nonewline $config_file [format "%s\t%s,\n" $comma $local_value]
		} elseif {[string compare -nocase $local_value ""] == 0} {
			if {$index == 0} {
			        puts $config_file "\tEndList,"
				puts $config_file "\tNO_SUPPLIES_CONFIGURED = XPAR_XSYSMONPSV_0_NO_MEAS,"
				break
			}
			puts $config_file "\tEndList,"
			break
		}
	    }
    }
    puts $config_file [format "\} %s_Supply;\n" $drv_string]
    puts $config_file "#endif"
    close $config_file
}
