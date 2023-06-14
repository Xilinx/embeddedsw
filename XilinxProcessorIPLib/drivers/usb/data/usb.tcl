###############################################################################
# Copyright (C) 2007 - 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 5.0      adk    12/10/13 Updated as per the New Tcl API's
##############################################################################
## @BEGIN_CHANGELOG EDK_M
##  Removed the local ::hsi::utils::define_canonical_xpars API as there is
##  a common API in the tcl of the tools 
##
## @END_CHANGELOG

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XUsb" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_INCLUDE_DMA" "C_M_AXI_ADDR_WIDTH"
    ::hsi::utils::define_config_file $drv_handle "xusb_g.c" "XUsb"  "DEVICE_ID" "C_BASEADDR" "C_INCLUDE_DMA" "C_M_AXI_ADDR_WIDTH"
    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Usb" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_INCLUDE_DMA" "C_M_AXI_ADDR_WIDTH"
}
