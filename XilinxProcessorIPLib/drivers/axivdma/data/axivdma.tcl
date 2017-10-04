###############################################################################
#
# Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
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
#  @file axivdma_v2_1_0.tcl
#
#
# <pre>
# MODIFICATION HISTORY:
#
# Ver   Who  Date     Changes
# ----- ---- -------- -------------------------------------------------------
# 3.00a srt  08/26/11 Added new parameters for Flush on Frame Sync and Line
#		      Buffer Thresholds.
# 4.00a srt  11/21/11 Added new parameters for Genlock Source and Fsync
#		      Source Selection.
# 4.03a srt  01/18/13 Added TDATA_WIDTH parameters (CR: 691866)
# 4.04a srt  03/01/13 Added DEBUG_INFO parameters (CR: 703738)
# 4.05a srt  05/01/3  Merged v4.03a driver with v4.04a driver.
#		         Driver v4.03a - Supports VDMA IPv5.04a XPS release
#		         Driver v4.04a - Supports VDMA IPv6.00a IPI release
#	              The parameters C_ENABLE_DEBUG_* are only available in
#		      VDMA IPv6.00a. These parameters should be set to '1'
#		      for older versions of IP (XPS) and added this logic in
#		      this file.
#  5.0     adk    10/12/13 Updated as per the New Tcl API's
#  6.4     ms     04/17/17 Modified tcl file to add suffix U for all macros
#                          definitions of axivdma in xparameters.h
# </pre>
#
##############################################################################


#uses "xillib.tcl"

proc generate {drv_handle} {
	xdefine_vdma_include_file $drv_handle "xparameters.h" "XAxiVdma" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_NUM_FSTORES" "C_INCLUDE_MM2S" "C_INCLUDE_MM2S_DRE" "C_M_AXI_MM2S_DATA_WIDTH" "C_INCLUDE_S2MM" "C_INCLUDE_S2MM_DRE" "C_M_AXI_S2MM_DATA_WIDTH" "C_AXI_MM2S_ACLK_FREQ_HZ" "C_AXI_S2MM_ACLK_FREQ_HZ" "C_MM2S_GENLOCK_MODE" "C_MM2S_GENLOCK_NUM_MASTERS" "C_S2MM_GENLOCK_MODE" "C_S2MM_GENLOCK_NUM_MASTERS" "C_INCLUDE_SG" "C_ENABLE_VIDPRMTR_READS" "C_USE_FSYNC" "C_FLUSH_ON_FSYNC" "C_MM2S_LINEBUFFER_DEPTH" "C_S2MM_LINEBUFFER_DEPTH" "C_INCLUDE_INTERNAL_GENLOCK" "C_S2MM_SOF_ENABLE" "C_M_AXIS_MM2S_TDATA_WIDTH" "C_S_AXIS_S2MM_TDATA_WIDTH" "C_ENABLE_DEBUG_INFO_1" "C_ENABLE_DEBUG_INFO_5" "C_ENABLE_DEBUG_INFO_6" "C_ENABLE_DEBUG_INFO_7" "C_ENABLE_DEBUG_INFO_9" "C_ENABLE_DEBUG_INFO_13" "C_ENABLE_DEBUG_INFO_14" "C_ENABLE_DEBUG_INFO_15" "C_ENABLE_DEBUG_ALL" "c_addr_width"
	xdefine_vdma_canonical_xpars $drv_handle "xparameters.h" "AxiVdma" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_NUM_FSTORES" "C_INCLUDE_MM2S" "C_INCLUDE_MM2S_DRE" "C_M_AXI_MM2S_DATA_WIDTH" "C_INCLUDE_S2MM" "C_INCLUDE_S2MM_DRE" "C_M_AXI_S2MM_DATA_WIDTH" "C_AXI_MM2S_ACLK_FREQ_HZ" "C_AXI_S2MM_ACLK_FREQ_HZ" "C_MM2S_GENLOCK_MODE" "C_MM2S_GENLOCK_NUM_MASTERS" "C_S2MM_GENLOCK_MODE" "C_S2MM_GENLOCK_NUM_MASTERS" "C_INCLUDE_SG" "C_ENABLE_VIDPRMTR_READS" "C_USE_FSYNC" "C_FLUSH_ON_FSYNC" "C_MM2S_LINEBUFFER_DEPTH" "C_S2MM_LINEBUFFER_DEPTH" "C_INCLUDE_INTERNAL_GENLOCK" "C_S2MM_SOF_ENABLE" "C_M_AXIS_MM2S_TDATA_WIDTH" "C_S_AXIS_S2MM_TDATA_WIDTH" "C_ENABLE_DEBUG_INFO_1" "C_ENABLE_DEBUG_INFO_5" "C_ENABLE_DEBUG_INFO_6" "C_ENABLE_DEBUG_INFO_7" "C_ENABLE_DEBUG_INFO_9" "C_ENABLE_DEBUG_INFO_13" "C_ENABLE_DEBUG_INFO_14" "C_ENABLE_DEBUG_INFO_15" "C_ENABLE_DEBUG_ALL" "c_addr_width"
	::hsi::utils::define_config_file  $drv_handle "xaxivdma_g.c" "XAxiVdma" "DEVICE_ID" "C_BASEADDR" "C_NUM_FSTORES" "C_INCLUDE_MM2S" "C_INCLUDE_MM2S_DRE" "C_M_AXI_MM2S_DATA_WIDTH" "C_INCLUDE_S2MM" "C_INCLUDE_S2MM_DRE" "C_M_AXI_S2MM_DATA_WIDTH" "C_INCLUDE_SG" "C_ENABLE_VIDPRMTR_READS" "C_USE_FSYNC" "C_FLUSH_ON_FSYNC" "C_MM2S_LINEBUFFER_DEPTH" "C_S2MM_LINEBUFFER_DEPTH" "C_MM2S_GENLOCK_MODE" "C_S2MM_GENLOCK_MODE" "C_INCLUDE_INTERNAL_GENLOCK" "C_S2MM_SOF_ENABLE" "C_M_AXIS_MM2S_TDATA_WIDTH" "C_S_AXIS_S2MM_TDATA_WIDTH" "C_ENABLE_DEBUG_INFO_1" "C_ENABLE_DEBUG_INFO_5" "C_ENABLE_DEBUG_INFO_6" "C_ENABLE_DEBUG_INFO_7" "C_ENABLE_DEBUG_INFO_9" "C_ENABLE_DEBUG_INFO_13" "C_ENABLE_DEBUG_INFO_14" "C_ENABLE_DEBUG_INFO_15" "C_ENABLE_DEBUG_ALL" "c_addr_width"
}


proc xdefine_vdma_include_file {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [hsi::utils::get_common_driver_ips $drv_handle]

    set uSuffix "U"

    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [get_property NAME $drv_handle]] */"
        # Define NUM_INSTANCES
        puts $file_handle "#define [hsi::utils::get_driver_param_name $drv_string $arg] [llength $periphs]$uSuffix"
        set args [lreplace $args $posn $posn]
    }
    # Check if it is a driver parameter

    lappend newargs
    foreach arg $args {
        set value [get_property CONFIG.$arg $drv_handle]
        if {[llength $value] == 0} {
            lappend newargs $arg
        } else {
            puts $file_handle "#define [hsi::utils::get_driver_param_name $drv_string $arg] [get_property CONFIG.$arg $drv_handle]$uSuffix"
        }
    }
    set args $newargs

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
        puts $file_handle ""
        puts $file_handle "/* Definitions for peripheral [string toupper [get_property NAME $periph]] */"
        foreach arg $args {
            if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                set value $device_id
                incr device_id
            } else {
                set value [::hsi::utils::get_param_value $periph $arg]
            }
            if {[llength $value] == 0} {
                set value 0
            }
	    # Check for *_ENABLE_DEBUG_* parameters.  These parameters are applicable
	    # to VDMA IPv6.00a (IPI release). For all the previous versions these
            # parameters should be set.
            if {[string first "ENABLE_DEBUG" $arg] >= 0} {
		set foundparam [::hsi::utils::get_param_value $periph $arg]
		if {[llength $foundparam] == 0} {
		    set value 1
		}
	    }
            set value [hsi::utils::format_addr_string $value $arg]
            if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define [hsi::utils::get_ip_param_name $periph $arg] \"$value\""
            } else {
                puts $file_handle "#define [hsi::utils::get_ip_param_name $periph $arg] $value$uSuffix"
            }
        }
        puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

proc xdefine_vdma_canonical_xpars {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [hsi::utils::open_include_file $file_name]

    # Get all the peripherals connected to this driver
    set periphs [hsi::utils::get_common_driver_ips $drv_handle]

    # Get the names of all the peripherals connected to this driver
    foreach periph $periphs {
        set peripheral_name [string toupper [get_property NAME $periph]]
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
        set periph_name [string toupper [get_property NAME $periph]]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {
            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            foreach arg $args {
                set lvalue [hsi::utils::get_driver_param_name $canonical_name $arg]

                # The commented out rvalue is the name of the instance-specific constant
                # set rvalue [hsi::utils::get_driver_param_name $periph $arg]
                # The rvalue set below is the actual value of the parameter
                set rvalue [::hsi::utils::get_param_value $periph $arg]
                if {[llength $rvalue] == 0} {
                    set rvalue 0
                }
		# Check for *_ENABLE_DEBUG_* parameters.  These parameters are applicable
		# to VDMA IPv6.00a (IPI release).  For all the previous versions these
		# parameters should be set.
                if {[string first "ENABLE_DEBUG" $arg] >= 0} {
		    set foundparam [::hsi::utils::get_param_value $periph $arg]
		    if {[llength $foundparam] == 0} {
		        set rvalue 1
		    }
	        }
                set rvalue [hsi::utils::format_addr_string $rvalue $arg]

		set uSuffix [xdefine_getSuffix $lvalue $rvalue]
                puts $file_handle "#define $lvalue $rvalue$uSuffix"

            }
            puts $file_handle ""
            incr i
        }
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

proc xdefine_getSuffix {arg_name value} {
	set uSuffix ""
	if { [string match "*DEVICE_ID" $value] == 0 } {
		set uSuffix "U"
	}
	return $uSuffix
}
