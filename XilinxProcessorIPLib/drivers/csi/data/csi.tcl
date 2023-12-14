###############################################################################
# Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
# Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
# Ver Who Date     Changes
# --- --- -------- ----------------------------------------------------
# 1.0 vsa 07/21/15 Initial version
##############################################################################

#uses "xillib.tcl"

set periph_ninstances    0

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XCsi" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "CSI_LANES" "CSI_OFFLOAD_NONIMAGE" "CSI_EN_VC_SUPPORT" "CSI_FIXED_VC" "C_CSI_OPT3_FIXEDLANES"
  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Csi" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "CSI_LANES" "CSI_OFFLOAD_NONIMAGE" "CSI_EN_VC_SUPPORT" "CSI_FIXED_VC" "C_CSI_OPT3_FIXEDLANES"
  ::hsi::utils::define_config_file  $drv_handle "xcsi_g.c" "XCsi" "DEVICE_ID" "C_BASEADDR" "CSI_LANES" "CSI_OFFLOAD_NONIMAGE" "CSI_EN_VC_SUPPORT" "CSI_FIXED_VC" "C_CSI_OPT3_FIXEDLANES"
}
