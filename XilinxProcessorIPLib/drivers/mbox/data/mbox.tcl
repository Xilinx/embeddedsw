###############################################################################
#
# Copyright (C) 2012 - 2014 Xilinx, Inc.  All rights reserved.
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
#
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 3.01a sdm  05/06/10 Updated to support AXI common::version of the core
# 3.02a bss  08/18/12 Updated the script to fix CR 655224 and CR 672073.
# 		      Added check for C_USE_EXTENDED_FSL_INSTR for AXI Stream.
# 3.02a bss  12/03/12 Updated the script to fix CR#687103 and CR#688715
# 4.1   sk   11/09/15 Removed delete filename statement CR# 784758.
# 4.2   ms   04/18/17 Modified tcl file to add suffix U for all macros
#                     definitions of mbox in xparameters.h
# 4.3   sd   07/26/17 Modified tcl file to prevent false unconnected flagging.
#
##############################################################################
#uses "xillib.tcl"


proc generate {drv_handle} {
    xdefine_mbox_config_files $drv_handle "xparameters.h" "xmbox_g.c" "XMbox"  
    xdefine_canonical_xpars $drv_handle "xparameters.h" "Mbox"
}

proc xdefine_mbox_config_if {periph hfile_handle cfile_handle bus_if if_num dev_id has_if0_dev_id} {

	upvar $dev_id device_id
	upvar $has_if0_dev_id has_if0_device_id

	set mbox_baseaddr	0
	set mbox_use_fsl	0
	set mbox_send_fsl	0
	set mbox_recv_fsl	0
	set if_isaxi		0
	set use_fsl 		0
	set uSuffix "U"

	# Copy over the right set of files as src based on processor type
	# sw_proc_handle contains driver handle for processor for which libgen is running. Name of the sw_proc_handle will be driver name for processor[Ex:cpu for microblaze]
	set sw_proc_handle [hsi::get_sw_processor]
	set hw_proc_handle [hsi::get_cells -hier $sw_proc_handle]
 	
	
	set periph_name [string toupper [common::get_property NAME $periph]]
	
	if {$bus_if == 2} {
		set mbox_baseaddr [common::get_property CONFIG.[format "C_S%d_AXI_BASEADDR" $if_num] $periph]
		
		set if_isaxi [check_if_connected $periph $if_num $bus_if] 	
	
	} else {
		## AXI Stream Interface
		# check if stream interface of mailbox is connected to current processor for which libgen is running
				
		set send_fsl 0
		set recv_fsl  0
		
		handle_stream $periph $bus_if $if_num use_fsl send_fsl recv_fsl	
				
		set if_isaxi 		0
		set mbox_use_fsl	$use_fsl
		set mbox_send_fsl	$send_fsl
		set mbox_recv_fsl	$recv_fsl
	}

	if { $if_isaxi == 1 || $use_fsl == 1 } {
	    puts $hfile_handle ""
	    puts $hfile_handle "/* Definitions for peripheral $periph_name IF ${if_num} */"

	    # The XPAR_INSTANCE_NAME_DEVICE_ID does not apply to Mailbox
	    # because the mailbox has two sides
	    # Unfortunately, this is used by TestApp
	    puts $hfile_handle [format "#define XPAR_%s_IF_%d_DEVICE_ID %s$uSuffix" $periph_name $if_num $device_id]
	    puts $hfile_handle [format "#define XPAR_%s_IF_%d_BASEADDR 0x%X$uSuffix" $periph_name $if_num $mbox_baseaddr]
	    puts $hfile_handle [format "#define XPAR_%s_IF_%d_USE_FSL %d$uSuffix" $periph_name $if_num $mbox_use_fsl]
	    puts $hfile_handle [format "#define XPAR_%s_IF_%d_SEND_FSL %d$uSuffix" $periph_name $if_num $mbox_send_fsl]
	    puts $hfile_handle [format "#define XPAR_%s_IF_%d_RECV_FSL %d$uSuffix" $periph_name $if_num $mbox_recv_fsl]

	    if {!$has_if0_device_id} {
		puts $hfile_handle ""
		puts $hfile_handle "/* Definition for TestApp ID */"		
		puts $hfile_handle [format "#define XPAR_%s_TESTAPP_ID %s$uSuffix" $periph_name $device_id]
	    }

	    puts $cfile_handle "\t\{"
	    puts $cfile_handle [format "\t\t XPAR_%s_IF_%d_DEVICE_ID, " $periph_name $if_num]
	    puts $cfile_handle [format "\t\t XPAR_%s_IF_%d_BASEADDR, " $periph_name $if_num]
	    puts $cfile_handle [format "\t\t XPAR_%s_IF_%d_USE_FSL," $periph_name $if_num]
	    puts $cfile_handle [format "\t\t XPAR_%s_IF_%d_SEND_FSL," $periph_name $if_num]
	    puts $cfile_handle [format "\t\t XPAR_%s_IF_%d_RECV_FSL" $periph_name $if_num]
	    puts -nonewline $cfile_handle "\t\}"
	    puts $cfile_handle ","
	    set has_if0_device_id 1
	    incr device_id
	}
}


#
# Create configuration C/H files as required by Xilinx drivers
#
proc xdefine_mbox_config_files {drv_handle hfile_name cfile_name drv_string} {

    # Open include file
    set hfile_handle [::hsi::utils::open_include_file $hfile_name]
    set cfile_name [file join "src" $cfile_name] 
    set cfile_handle [open $cfile_name w]
    set uSuffix "U"

    ::hsi::utils::write_c_header $cfile_handle "Driver configuration"
    puts $cfile_handle "#include \"xparameters.h\""
    puts $cfile_handle "#include \"[string tolower $drv_string].h\""
    puts $cfile_handle "\n/*"
    puts $cfile_handle " * The configuration table for devices"
    puts $cfile_handle " */\n"
    puts $cfile_handle [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
    puts $cfile_handle "\{"

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
    	set has_if0_device_id 0
	for {set if_num 0} {$if_num < 2} {incr if_num} {
		set bus_if [common::get_property CONFIG.[format "C_INTERCONNECT_PORT_%d" $if_num] $periph]
		xdefine_mbox_config_if $periph $hfile_handle $cfile_handle $bus_if $if_num device_id has_if0_device_id
	}
    }

    puts $cfile_handle "\};"
    close $cfile_handle

    puts $hfile_handle ""
    puts $hfile_handle "/* Definitions for driver MAILBOX */"
    puts $hfile_handle [format "#define XPAR_XMBOX_NUM_INSTANCES %d$uSuffix" $device_id]
    puts $hfile_handle "\n/******************************************************************/\n"
    close $hfile_handle
}

# Check whether the interface is connected or not
proc check_if_connected {periph if_num bus_if} {
	set sw_proc_handle [hsi::get_sw_processor]
	set hw_proc_handle [hsi::get_cells -hier $sw_proc_handle]
    	set if_isaxi 0
    	set if_axis_connected 0

     	if {$bus_if == 2} {
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
	} else { 		
  		set use_fsl 0
		set send_fsl 0
		set recv_fsl  0
  		handle_stream $periph $bus_if $if_num use_fsl send_fsl recv_fsl
  		set if_axis_connected $use_fsl
 	}	

    	if {$if_isaxi || $if_axis_connected} {
		return 1
    	} else {
		return 0
    	}
}

# Generate canonical definitions for device ID
proc gen_canonical_device_id {file_handle canonical_name periph_name if_num} {
    set lvalue [::hsi::utils::get_driver_param_name $canonical_name "DEVICE_ID"]
    set rvalue "XPAR_${periph_name}_IF_${if_num}_DEVICE_ID"
    puts $file_handle "#define $lvalue $rvalue"
}

# Generate canonical definitions for parameters
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

proc gen_canonical_fsl_param_def {file_handle canonical_name periph if_num} {
	set periph_name [string toupper [common::get_property NAME $periph]]

	puts $file_handle [format "#define [::hsi::utils::get_driver_param_name $canonical_name "USE_FSL"] XPAR_%s_IF_%d_USE_FSL" $periph_name $if_num]
	puts $file_handle [format "#define [::hsi::utils::get_driver_param_name $canonical_name "SEND_FSL"] XPAR_%s_IF_%d_SEND_FSL" $periph_name $if_num]
	puts $file_handle [format "#define [::hsi::utils::get_driver_param_name $canonical_name "RECV_FSL"] XPAR_%s_IF_%d_RECV_FSL" $periph_name $if_num]
}

# Generate canonical definitions for an interface
proc gen_canonical_if_def {file_handle periph if_num bus_if drv_string dev_id common_params} {
    upvar $dev_id device_id

    set periph_name [string toupper [common::get_property NAME $periph]]
    set canonical_name [format "%s_%s" $drv_string $device_id]

    # Make sure canonical name is not the same as hardware instance
    if { [string compare -nocase $canonical_name $periph_name] == 0 } {
	return
    }
  
    set if_connected [check_if_connected $periph $if_num $bus_if]

    if {$if_connected} {
	puts $file_handle ""
	puts $file_handle "/* Canonical definitions for peripheral $periph_name IF ${if_num} */"

	gen_canonical_device_id $file_handle $canonical_name $periph_name $if_num

	set addr_args [list "BASEADDR" "HIGHADDR"]
	
	gen_canonical_param_def $file_handle $canonical_name $periph "C_S${if_num}_AXI" $addr_args
	
	gen_canonical_param_def $file_handle $canonical_name $periph "" $common_params
	gen_canonical_fsl_param_def $file_handle $canonical_name $periph $if_num

	incr device_id
	puts $file_handle ""
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
	for {set if_num 0} {$if_num < 2} {incr if_num} {
		set bus_if [common::get_property CONFIG.[format "C_INTERCONNECT_PORT_%d" $if_num] $periph]
	    	gen_canonical_if_def $file_handle $periph $if_num $bus_if $drv_string device_id $args
	}
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

proc handle_stream {periph bus_if if_num usefsl sendfsl recfsl} {
	
	upvar $usefsl	use_fsl
	upvar $sendfsl	send_fsl
	upvar $recfsl 	recv_fsl
	set not_connected 0
	
	set sw_proc_handle [hsi::get_sw_processor]
	set hw_proc_handle [hsi::get_cells -hier $sw_proc_handle]
		
	set periph_name [string toupper [common::get_property NAME $periph]]
	
	set initiator_handle [::hsi::utils::get_connected_intf $periph S${if_num}_AXIS]
	if { [llength $initiator_handle] == 0 } {
		incr not_connected
	} else {
		set maxis_initiator_handle [hsi::get_cells -of_objects $initiator_handle]
		if { $maxis_initiator_handle == $hw_proc_handle } {
			if {[common::get_property CONFIG.C_USE_EXTENDED_FSL_INSTR $hw_proc_handle] != 1 } {
				error  "ERROR: The mailbox driver requires parameter C_USE_EXTENDED_FSL_INSTR on MicroBlaze to be enabled when an AXI Stream interface is used to connect the mailbox core." "" "mdt_error"				
	    		}
		set initiator_name [common::get_property NAME $initiator_handle]
		scan $initiator_name "M%d_AXIS" send_fsl
		set use_fsl 1
		} else {
			set use_fsl 0
		}
	}
			
	set target_handle [::hsi::utils::get_connected_intf $periph M${if_num}_AXIS]
	if { [llength $target_handle] == 0 } {
		incr not_connected
	} else {
		set saxis_target_handle [hsi::get_cells -of_objects $target_handle]
		if { $saxis_target_handle == $hw_proc_handle } {
			if {[common::get_property CONFIG.C_USE_EXTENDED_FSL_INSTR $hw_proc_handle] != 1 } {
				error "ERROR: The mailbox driver requires parameter C_USE_EXTENDED_FSL_INSTR on MicroBlaze to be enabled when an AXI Stream interface is used to connect the mailbox core." "" "mdt_error"				
			}
			set target_name [common::get_property NAME $target_handle]
			scan $target_name "S%d_AXIS" recv_fsl
			set use_fsl 1
		} else {
			set use_fsl 0
		}
	}
	
	if { $not_connected == 2 } {
		error "ERROR: Unable to figure out AXI stream connectivity for Interface ${if_num} on mailbox $periph_name."
	}

}
