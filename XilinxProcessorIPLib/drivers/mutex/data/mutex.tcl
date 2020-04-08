###############################################################################
# Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 3.01a sdm  05/06/10 Updated to support AXI common::version of the core
# 3.01a sdm  05/06/11 Updated to handle mutex connected through axi2axiconnector
# 3.02a bss  01/31/13 Updated script to fix CR #679127
# 4.0   bss  12/10/13 Updated as per the New Tcl API's
# 4.1   sk   11/09/15 Removed delete filename statement CR# 784758.
# 4.3   ms   04/18/17 Modified tcl file to add suffix U for all macros
#                     definitions of mutex in xparameters.h
# 4.4   adk  19/09/19 Updated tcl to generate proper canonical definitions when
#		      mutex is configured for more then one axi interface.
###############################################################################
#uses "xillib.tcl"

proc generate {drv_handle} {
    xdefine_mutex_config_files $drv_handle "xparameters.h" "xmutex_g.c" "XMutex"  
    xdefine_canonical_xpars $drv_handle "xparameters.h" "Mutex" "C_NUM_MUTEX" "C_ENABLE_USER"
}

#
# Create configuration data of an interface, for c/h files as required by Xilinx drivers
#
proc xdefine_mutex_config_if {periph hfile_handle cfile_handle num_ifs dev_id has_if0_dev_id} {
	upvar $dev_id device_id
	upvar $has_if0_dev_id if0_connected

	set mutex_baseaddr    0
	set mutex_enableuser  0
	set mutex_num     0
	set if_connected  0
	set uSuffix "U"

	set periph_name [string toupper [common::get_property NAME $periph]]


	for {set x 0} {$x < $num_ifs} {incr x} {
	    set if_connected [check_if_connected $periph $x]

	    if {$if_connected} {		
		
		set mutex_baseaddr [common::get_property CONFIG.[format "C_S%d_AXI_BASEADDR" $x] $periph]
		set mutex_enableuser [common::get_property CONFIG.C_ENABLE_USER $periph]
		set mutex_num [common::get_property CONFIG.C_NUM_MUTEX $periph]

		puts $hfile_handle ""
		puts $hfile_handle "/* Definitions for peripheral $periph_name IF ${x} */"
		puts $hfile_handle [format "#define XPAR_%s_IF_%d_DEVICE_ID %s$uSuffix" $periph_name $x $device_id]

		if {!$if0_connected} {
		    puts $hfile_handle [format "#define XPAR_%s_TESTAPP_ID %s$uSuffix" $periph_name $device_id]
		}

		puts $hfile_handle [format "#define XPAR_%s_IF_%d_BASEADDR 0x%X$uSuffix" $periph_name $x $mutex_baseaddr]
		puts $hfile_handle [format "#define XPAR_%s_IF_%d_NUM_MUTEX %s$uSuffix" $periph_name $x $mutex_num]
		puts $hfile_handle [format "#define XPAR_%s_IF_%d_ENABLE_USER %s$uSuffix" $periph_name $x $mutex_enableuser]

		puts $cfile_handle "\t\{"
		puts $cfile_handle [format "\t\t XPAR_%s_IF_%d_DEVICE_ID, " $periph_name $x]
		puts $cfile_handle [format "\t\t XPAR_%s_IF_%d_BASEADDR, " $periph_name $x]
		puts $cfile_handle [format "\t\t XPAR_%s_IF_%d_NUM_MUTEX, " $periph_name $x]
		puts $cfile_handle [format "\t\t XPAR_%s_IF_%d_ENABLE_USER, " $periph_name $x]
		puts -nonewline $cfile_handle "\t\}"
		puts $cfile_handle ","
		set if0_connected 1
		incr device_id
	    }
	}
}

#
# Create configuration c/h files as required by Xilinx drivers
#
proc xdefine_mutex_config_files {drv_handle hfile_name cfile_name drv_string} {

    # Open include file
    set hfile_handle [::hsi::utils::open_include_file $hfile_name]
    set cfile_name [file join "src" $cfile_name] 
    set cfile_handle [open $cfile_name w]
    set uSuffix "U"

    ::hsi::utils::write_c_header $cfile_handle "Driver configuration"
    puts $cfile_handle "#include \"xparameters.h\""
    puts $cfile_handle "#include \"[string tolower $drv_string].h\""
    puts $cfile_handle "\n/*"
    puts $cfile_handle "* The configuration table for devices"
    puts $cfile_handle "*/\n"
    puts $cfile_handle [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
    puts $cfile_handle "\{"

    # Get all peripherals connected to this driver
     set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
	set has_if0_device_id 0
	set num_ifs [common::get_property CONFIG.C_NUM_AXI $periph]
	xdefine_mutex_config_if $periph $hfile_handle $cfile_handle $num_ifs device_id has_if0_device_id
    }

    puts $cfile_handle "\};"
    close $cfile_handle

    puts $hfile_handle ""
    puts $hfile_handle "/* Definitions for driver MUTEX */"
    puts $hfile_handle [format "#define XPAR_XMUTEX_NUM_INSTANCES %d$uSuffix" $device_id]
    puts $hfile_handle "\n/******************************************************************/\n"
    close $hfile_handle
}

proc check_if_connected {periph if_num} {
	set sw_proc_handle [hsi::get_sw_processor]
	set hw_proc_handle [hsi::get_cells -hier $sw_proc_handle]
    	set if_isaxi 0

	set baseaddr [common::get_property CONFIG.[format "C_S%d_AXI_BASEADDR" $if_num] $periph]
	set mem [hsi::get_mem_ranges -of_objects $hw_proc_handle -filter "INSTANCE==$periph"]
	if {[llength $mem] != 0} {
		set addrs [common::get_property BASE_VALUE $mem]
		foreach addr $addrs {
			if {$addr == $baseaddr} {
				set if_isaxi 1
			}	
		}
	}	
	
	if {$if_isaxi == 1} {
        	return 1
    	} else {
        	return 0
    	}
}

#
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc gen_canonical_device_id {file_handle canonical_name periph_name if_num} {
    set lvalue [::hsi::utils::get_driver_param_name $canonical_name "DEVICE_ID"]
    set rvalue "XPAR_${periph_name}_IF_${if_num}_DEVICE_ID"
    puts $file_handle "#define $lvalue $rvalue"
}

proc gen_canonical_param_def {file_handle canonical_name periph param_prefix params} {
    set uSuffix "U"
    foreach arg $params {
	if {$param_prefix == ""} {
	    set actual_arg "${arg}"
	} else {
	    set actual_arg "${param_prefix}_${arg}"
	}
	set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]
	
	
	# The rvalue set below is the actual value of the parameter
	set rvalue [::hsi::utils::get_param_value $periph $actual_arg]
	if {[llength $rvalue] == 0} {
	    set rvalue 0
	}
	set rvalue [::hsi::utils::format_addr_string $rvalue $actual_arg]
	
	puts $file_handle "#define $lvalue $rvalue$uSuffix"
    }
}

proc gen_canonical_if_def {file_handle periph num_ifs drv_string dev_id common_params} {
    upvar $dev_id device_id

    set periph_name [string toupper [common::get_property NAME $periph]]
    
    for {set x 0} {$x < $num_ifs} {incr x} {
	set if_connected [check_if_connected $periph $x]
	
	if {$if_connected} {
	    puts $file_handle ""
	    puts $file_handle "/* Canonical definitions for peripheral $periph_name IF ${x} */"

	    set canonical_name [format "%s_%s" $drv_string $x]
	    gen_canonical_device_id $file_handle $canonical_name $periph_name $x

	    set addr_args [list "BASEADDR" "HIGHADDR"]
	 
	    gen_canonical_param_def $file_handle $canonical_name $periph "C_S${x}_AXI" $addr_args
	    gen_canonical_param_def $file_handle $canonical_name $periph "" $common_params

	    incr device_id
	    puts $file_handle ""
	}
    }
}

#
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Print canonical parameters for each peripheral
    set device_id 0
    foreach periph $periphs {
	set num_ifs [::hsi::utils::get_param_value $periph "C_NUM_AXI"]
	gen_canonical_if_def $file_handle $periph $num_ifs $drv_string device_id $args
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}
