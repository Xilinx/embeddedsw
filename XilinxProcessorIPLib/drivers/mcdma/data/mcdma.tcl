###############################################################################
#
# Copyright (C) 2017 - 2019 Xilinx, Inc.  All rights reserved.
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
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who   Date     Changes
# ----- ----  -------- -----------------------------------------------
# 1.0   adk   18/07/17  First release
# 1.1   adk   09/02/18  Updated tcl logic to export proper values for
#			CACHE_COHERENT properties when h/w is configured for
#			single axi4 data interface.
#			Added failure checks in the tcl to avoid bsp compilation
#			errors incase stream interface is unconnected.
# 1.2   rsp   09/07/18  Pass "hier" argument to get_cells API to support hierarchical designs.
# 1.3   rsp   02/05/19  Enable CCI only at EL1 non-secure state.
#       rsp   02/12/19  Export use RxLength field.
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XMcdma" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_INCLUDE_MM2S" "C_INCLUDE_S2MM" "C_NUM_MM2S_CHANNELS" "C_NUM_S2MM_CHANNELS" "C_INCLUDE_MM2S_DRE" "C_M_AXI_SG_ADDR_WIDTH" "C_INCLUDE_S2MM_DRE" "C_ENABLE_SINGLE_INTR" "C_M_AXI_MM2S_DATA_WIDTH" "C_M_AXI_S2MM_DATA_WIDTH" "C_SG_LENGTH_WIDTH" "C_SG_INCLUDE_STSCNTRL_STRM" "C_SG_USE_STSAPP_LENGTH"

    generate_cci_params $drv_handle "xparameters.h"
    ::hsi::utils::define_config_file $drv_handle "xmcdma_g.c" "XMcdma" "DEVICE_ID" "C_BASEADDR" "C_M_AXI_SG_ADDR_WIDTH" "C_ENABLE_SINGLE_INTR" "C_INCLUDE_MM2S" "C_INCLUDE_MM2S_DRE" "C_NUM_MM2S_CHANNELS" "C_INCLUDE_S2MM" "C_INCLUDE_S2MM_DRE" "C_NUM_S2MM_CHANNELS" "C_M_AXI_MM2S_DATA_WIDTH" "C_M_AXI_S2MM_DATA_WIDTH" "C_SG_LENGTH_WIDTH" "C_SG_INCLUDE_STSCNTRL_STRM" "C_SG_USE_STSAPP_LENGTH" "IS_MM2S_CACHE_COHERENT" "IS_S2MM_CACHE_COHERENT"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Mcdma" "DEVICE_ID" "C_BASEADDR" "C_M_AXI_SG_ADDR_WIDTH" "C_ENABLE_SINGLE_INTR" "C_INCLUDE_MM2S" "C_INCLUDE_MM2S_DRE" "C_NUM_MM2S_CHANNELS" "C_INCLUDE_S2MM" "C_INCLUDE_S2MM_DRE" "C_NUM_S2MM_CHANNELS" "C_M_AXI_MM2S_DATA_WIDTH" "C_M_AXI_S2MM_DATA_WIDTH" "C_SG_LENGTH_WIDTH" "C_SG_INCLUDE_STSCNTRL_STRM" "C_SG_USE_STSAPP_LENGTH"

}

proc generate_cci_params {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	# Get all peripherals connected to this driver
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]

	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

	#set is_hpcdesign 0
	foreach ip $ips {
		set iptype [common::get_property IP_NAME [get_cells -hier $ip]]
		if {$processor_type == "psu_cortexa53"} {
			set is_hpcdesign 0
			set has_signleintf [common::get_property CONFIG.c_single_interface [get_cells -hier $ip]]
			set has_mm2s [common::get_property CONFIG.c_include_mm2s [get_cells -hier $ip]]
			set hypervisor [common::get_property CONFIG.hypervisor_guest [hsi::get_os]]
			if {$has_mm2s == 1} {
				set is_hpcdesign [get_connected_if $ip "M_AXI_MM2S"]
			}
			if {$has_signleintf == 1} {
				set is_hpcdesign [get_connected_if $ip "M_AXI"]
			}
			if { $is_hpcdesign && $hypervisor} {
				puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_MM2S_CACHE_COHERENT"] 1"
			} else {
				puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_MM2S_CACHE_COHERENT"] 0"
			}
			set is_hpcdesign 0
			set has_s2mm [common::get_property CONFIG.c_include_s2mm [get_cells -hier $ip]]
			if {$has_s2mm == 1} {
				set is_hpcdesign [get_connected_if $ip "M_AXI_S2MM"]
			}
			if {$has_signleintf == 1} {
				set is_hpcdesign [get_connected_if $ip "M_AXI"]
			}
			if { $is_hpcdesign && $hypervisor} {
				puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_S2MM_CACHE_COHERENT"] 1"
			} else {
				puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_S2MM_CACHE_COHERENT"] 0"
			}
		} else {
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_MM2S_CACHE_COHERENT"] 0"
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_S2MM_CACHE_COHERENT"] 0"
		}
	}
	close $file_handle
}

proc get_connected_if {drv_handle dma_pin} {
	set iphandle [::hsi::utils::get_connected_stream_ip $drv_handle $dma_pin]
        if {[llength $iphandle] == 0} {
		return 0
	}
	set ipname [get_property IP_NAME $iphandle]
	if {$ipname == "axi_interconnect" || $ipname == "smartconnect"} {
		set get_intf [::hsi::utils::get_connected_intf $iphandle M00_AXI]
		set name "HPC"
		set intf_names [split $get_intf "_"]
		foreach intf $intf_names {
			set name [string trimright $intf]
			if {[string compare -length 3 $name "HPC"] == 0} {
				return 1
			}
		}
	}
	return 0
}
