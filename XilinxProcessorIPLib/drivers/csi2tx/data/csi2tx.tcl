###############################################################################
# Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
# Ver Who Date     Changes
# --- --- -------- -----------------------------------------------------------
# 1.0 sss 07/21/16 Initial version
# 1.1 vsa 02/28/18 Added Frame Generation feature
##############################################################################

#uses "xillib.tcl"

set periph_ninstances    0

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XCsi2Tx" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_CSI_LANES" "C_CSI_EN_ACTIVELANES" "C_EN_REG_BASED_FE_GEN"
  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Csi2Tx" "DEVICE_ID" "C_BASEADDR" "C_CSI_LANES" "C_CSI_EN_ACTIVELANES" "C_EN_REG_BASED_FE_GEN"
  ::hsi::utils::define_config_file  $drv_handle "xcsi2tx_g.c" "XCsi2Tx" "DEVICE_ID" "C_BASEADDR" "C_CSI_LANES" "C_CSI_EN_ACTIVELANES" "C_EN_REG_BASED_FE_GEN"
}
