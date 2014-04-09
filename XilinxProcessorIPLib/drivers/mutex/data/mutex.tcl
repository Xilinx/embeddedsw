##############################################################################
#
# (c) Copyright 2007-2014 Xilinx, Inc. All rights reserved.
#
# This file contains confidential and proprietary information of Xilinx, Inc.
# and is protected under U.S. and international copyright and other
# intellectual property laws.
#
# DISCLAIMER
# This disclaimer is not a license and does not grant any rights to the
# materials distributed herewith. Except as otherwise provided in a valid
# license issued to you by Xilinx, and to the maximum extent permitted by
# applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
# FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
# MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
# and (2) Xilinx shall not be liable (whether in contract or tort, including
# negligence, or under any other theory of liability) for any loss or damage
# of any kind or nature related to, arising under or in connection with these
# materials, including for any direct, or any indirect, special, incidental,
# or consequential loss or damage (including loss of data, profits, goodwill,
# or any type of loss or damage suffered as a result of any action brought by
# a third party) even if such damage or loss was reasonably foreseeable or
# Xilinx had been advised of the possibility of the same.
#
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-safe, or for use in
# any application requiring fail-safe performance, such as life-support or
# safety devices or systems, Class III medical devices, nuclear facilities,
# applications related to the deployment of airbags, or any other applications
# that could lead to death, personal injury, or severe property or
# environmental damage (individually and collectively, "Critical
# Applications"). Customer assumes the sole risk and liability of any use of
# Xilinx products in Critical Applications, subject only to applicable laws
# and regulations governing limitations on product liability.
#
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
# AT ALL TIMES.
#
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 3.01a sdm  05/06/10 Updated to support AXI version of the core
# 3.01a sdm  05/06/11 Updated to handle mutex connected through axi2axiconnector
# 3.02a bss  01/31/13 Updated script to fix CR #679127
# 4.0   bss  12/10/13 Updated as per the New Tcl API's
#
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

	set periph_name [string toupper [get_property NAME $periph]]


	for {set x 0} {$x < $num_ifs} {incr x} {
	    set if_connected [check_if_connected $periph $x]

	    if {$if_connected} {		
		
		set mutex_baseaddr [get_property CONFIG.[format "C_S%d_AXI_BASEADDR" $x] $periph]		
		set mutex_enableuser [get_property CONFIG.C_ENABLE_USER $periph]
		set mutex_num [get_property CONFIG.C_NUM_MUTEX $periph]

		puts $hfile_handle ""
		puts $hfile_handle "/* Definitions for peripheral $periph_name IF ${x} */"
		puts $hfile_handle [format "#define XPAR_%s_IF_%d_DEVICE_ID %s" $periph_name $x $device_id]

		if {!$if0_connected} {
		    puts $hfile_handle [format "#define XPAR_%s_TESTAPP_ID %s" $periph_name $device_id]
		}

		puts $hfile_handle [format "#define XPAR_%s_IF_%d_BASEADDR 0x%X" $periph_name $x $mutex_baseaddr]
		puts $hfile_handle [format "#define XPAR_%s_IF_%d_NUM_MUTEX %s" $periph_name $x $mutex_num]
		puts $hfile_handle [format "#define XPAR_%s_IF_%d_ENABLE_USER %s" $periph_name $x $mutex_enableuser]

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
    set hfile_handle [xopen_include_file $hfile_name]
    set cfile_name [file join "src" $cfile_name] 
    file delete $cfile_name
    set cfile_handle [open $cfile_name w]

    xprint_generated_header $cfile_handle "Driver configuration"    
    puts $cfile_handle "#include \"xparameters.h\""
    puts $cfile_handle "#include \"[string tolower $drv_string].h\""
    puts $cfile_handle "\n/*"
    puts $cfile_handle "* The configuration table for devices"
    puts $cfile_handle "*/\n"
    puts $cfile_handle [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
    puts $cfile_handle "\{"

    # Get all peripherals connected to this driver
     set periphs [xget_sw_iplist_for_driver $drv_handle]

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
	set has_if0_device_id 0
	set num_ifs [get_property CONFIG.C_NUM_AXI $periph]
	xdefine_mutex_config_if $periph $hfile_handle $cfile_handle $num_ifs device_id has_if0_device_id
    }

    puts $cfile_handle "\};"
    close $cfile_handle

    puts $hfile_handle ""
    puts $hfile_handle "/* Definitions for driver MUTEX */"
    puts $hfile_handle [format "#define XPAR_XMUTEX_NUM_INSTANCES %d" $device_id]
    puts $hfile_handle "\n/******************************************************************/\n"
    close $hfile_handle
}

proc check_if_connected {periph if_num} {
	set sw_proc_handle [get_sw_processor]
	set hw_proc_handle [get_cells $sw_proc_handle]
    	set if_isaxi 0

	set baseaddr [get_property CONFIG.[format "C_S%d_AXI_BASEADDR" $if_num] $periph]
	set mem [get_mem_ranges -of_objects $hw_proc_handle -filter "INSTANCE==$periph"]
	if {[llength $mem] != 0} {
		set addrs [get_property BASE_VALUE $mem]
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
    set lvalue [xget_dname $canonical_name "DEVICE_ID"]
    set rvalue "XPAR_${periph_name}_IF_${if_num}_DEVICE_ID"
    puts $file_handle "#define $lvalue $rvalue"
}

proc gen_canonical_param_def {file_handle canonical_name periph param_prefix params} {
    foreach arg $params {
	if {$param_prefix == ""} {
	    set actual_arg "${arg}"
	} else {
	    set actual_arg "${param_prefix}_${arg}"
	}
	set lvalue [xget_dname $canonical_name $arg]
	
	
	# The rvalue set below is the actual value of the parameter
	set rvalue [xget_param_value $periph $actual_arg]
	if {[llength $rvalue] == 0} {
	    set rvalue 0
	}
	set rvalue [xformat_addr_string $rvalue $actual_arg]
	
	puts $file_handle "#define $lvalue $rvalue"
    }
}

proc gen_canonical_if_def {file_handle periph num_ifs drv_string dev_id common_params} {
    upvar $dev_id device_id

    set periph_name [string toupper [get_property NAME $periph]]
    set canonical_name [format "%s_%s" $drv_string $device_id]
    
    # Make sure canonical name is not the same as hardware instance
    if { [string compare -nocase $canonical_name $periph_name] == 0 } {
	return
    }
   
    for {set x 0} {$x < $num_ifs} {incr x} {
	set if_connected [check_if_connected $periph $x]
	puts "correct"
	puts $if_connected
	
	if {$if_connected} {
	    puts $file_handle ""
	    puts $file_handle "/* Canonical definitions for peripheral $periph_name IF ${x} */"

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
    set file_handle [xopen_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [xget_sw_iplist_for_driver $drv_handle]

    # Print canonical parameters for each peripheral
    set device_id 0
    foreach periph $periphs {
	set num_ifs [xget_param_value $periph "C_NUM_AXI"]
	gen_canonical_if_def $file_handle $periph $num_ifs $drv_string device_id $args
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}
