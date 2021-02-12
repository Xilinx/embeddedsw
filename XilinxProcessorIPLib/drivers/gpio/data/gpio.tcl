###############################################################################
# Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XGpio" "NUM_INSTANCES" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID" "C_INTERRUPT_PRESENT" "C_IS_DUAL"
  ::hsi::utils::define_config_file $drv_handle "xgpio_g.c" "XGpio"  "DEVICE_ID" "C_BASEADDR" "C_INTERRUPT_PRESENT" "C_IS_DUAL"
  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Gpio" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID" "C_INTERRUPT_PRESENT" "C_IS_DUAL"
}
