###############################################################################
# Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
##############################################################################

proc generate {drv_handle} {
	::hsi::utils::define_include_file $drv_handle "xparameters.h" "XAxiCdma" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_INCLUDE_DRE" "C_USE_DATAMOVER_LITE" "C_M_AXI_DATA_WIDTH" "C_INCLUDE_SG" "C_M_AXI_MAX_BURST_LEN" "C_ADDR_WIDTH"

	::hsi::utils::define_config_file  $drv_handle "xaxicdma_g.c" "XAxiCdma" "DEVICE_ID" "C_BASEADDR" "C_INCLUDE_DRE" "C_USE_DATAMOVER_LITE" "C_M_AXI_DATA_WIDTH" "C_M_AXI_MAX_BURST_LEN" "C_ADDR_WIDTH"

	::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "AxiCdma" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_INCLUDE_DRE" "C_USE_DATAMOVER_LITE" "C_M_AXI_DATA_WIDTH" "C_INCLUDE_SG" "C_M_AXI_MAX_BURST_LEN" "C_ADDR_WIDTH"
	set periph [get_cells -hier $drv_handle]
	set vlnv_string [string tolower [common::get_property VLNV $periph]]
	set vlnv_string [split $vlnv_string :]
	set ip_version [lindex $vlnv_string 3]
	set file_handle [::hsi::utils::open_include_file "xparameters.h"]
	# In CDMA IP (4.1 onwards) BTT is increased to 26 bits
	if { $ip_version >= 4.1 } {
		puts $file_handle "#define XAXICDMA_MAX_TRANSFER_LEN    0x3FFFFFF"
	} else {
		puts $file_handle "#define XAXICDMA_MAX_TRANSFER_LEN    0x7FFFFF"
	}
	close $file_handle
}
